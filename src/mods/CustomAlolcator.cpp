#include "CustomAlolcator.hpp" // NOLINT
#include <queue>
#include <vadefs.h>
#include <winuser.h>
#include <list>
#include <d3dx9.h>

#include <DbgHelp.h> // ImageNtHeader
#pragma comment(lib, "dbghelp.lib")
#include "StyleSwitchFX.hpp"
// to check if allocation succeded in other mods (LDK so far)
CustomAlolcator* g_custom_alolcator{nullptr};
bool g_mem_patch_applied{false};

struct DataSectionInfo {
    uintptr_t g_data_section_start {0};
    size_t    g_data_section_size {0};
};

DataSectionInfo g_data_section_info;

// size literals
constexpr std::size_t operator""_KiB(unsigned long long int x) {
    return 1024ULL * x;
}

constexpr std::size_t operator""_MiB(unsigned long long int x) {
    return 1024_KiB * x;
}

constexpr std::size_t operator""_GiB(unsigned long long int x) {
    return 1024_MiB * x;
}

#pragma region MEMORY_ARENA
namespace memory {
// code stolen from gingerBills article on memory allocators
// https://www.gingerbill.org/article/2019/02/08/memory-allocation-strategies-002/

#include <stddef.h>
#include <stdint.h>

#if !defined(__cplusplus)
#if (defined(_MSC_VER) && _MSC_VER < 1800) || \
    (!defined(_MSC_VER) && !defined(__STDC_VERSION__))
#ifndef true
#define true (0 == 0)
#endif
#ifndef false
#define false (0 != 0)
#endif
typedef unsigned char bool;
#else
#include <stdbool.h>
#endif
#endif

#include <assert.h>
#include <stdio.h>
#include <string.h>

static bool is_power_of_two(uintptr_t x) { return (x & (x - 1)) == 0; }

static uintptr_t align_forward(uintptr_t ptr, size_t align) {
    uintptr_t p, a, modulo;

    assert(is_power_of_two(align));

    p = ptr;
    a = (uintptr_t)align;
    // Same as (p % a) but faster as 'a' is a power of two
    modulo = p & (a - 1);
    // modulo = p & (a - 1);

    if (modulo != 0) {
        // If 'p' address is not aligned, push the address to the
        // next value which is aligned
        p += a - modulo;
    }
    return p;
}

#ifndef DEFAULT_ALIGNMENT
#define DEFAULT_ALIGNMENT (2 * sizeof(void*))
#endif

typedef struct Arena Arena;
struct Arena {
    unsigned char* buf;
    size_t buf_len;
    size_t prev_offset; // This will be useful for later on
    size_t curr_offset;
};

static void arena_init(Arena* a, void* backing_buffer, size_t backing_buffer_length) {
    a->buf         = (unsigned char*)backing_buffer;
    a->buf_len     = backing_buffer_length;
    a->curr_offset = 0;
    a->prev_offset = 0;
#if 0 // was checking for float math fucking up 0x7ff8dead is NaN when
      // interpreted as float, the more you know
	uint32_t* iter = (uint32_t*)a->buf;
	for (size_t i = 0; i < (a->buf_len / sizeof(int)); i++) {
		iter[i] = 0x7ff8dead;
	}
#endif
}

static void* arena_alloc_align(Arena* a, size_t size, size_t align) {
    // Align 'curr_offset' forward to the specified alignment
    uintptr_t curr_ptr = (uintptr_t)a->buf + (uintptr_t)a->curr_offset;
    uintptr_t offset   = align_forward(curr_ptr, align);
    offset -= (uintptr_t)a->buf; // Change to relative offset

    // Check to see if the backing memory has space left
    if (offset + size <= a->buf_len) {
        void* ptr      = &a->buf[offset];
        a->prev_offset = offset;
        a->curr_offset = offset + size;

        // Zero new memory by default
#ifndef _NDEBUG
        // memset(ptr, 0, size);
#else
#endif
        return ptr;
    }
    // Return NULL if the arena is out of memory (or handle differently)
    return NULL;
}

// Because C doesn't have default parameters
static void* arena_alloc(Arena* a, size_t size) {
    return arena_alloc_align(a, size, DEFAULT_ALIGNMENT);
}

static void arena_free(Arena* a, void* ptr) {
    // Do nothing
}

static void* arena_resize_align(Arena* a, void* old_memory, size_t old_size,
                         size_t new_size, size_t align) {
    unsigned char* old_mem = (unsigned char*)old_memory;

    assert(is_power_of_two(align));

    if (old_mem == NULL || old_size == 0) {
        return arena_alloc_align(a, new_size, align);
    }
    if (a->buf <= old_mem && old_mem < a->buf + a->buf_len) {
        if (a->buf + a->prev_offset == old_mem) {
            a->curr_offset = a->prev_offset + new_size;
            if (new_size > old_size) {
                // Zero the new memory by default
                memset(&a->buf[a->curr_offset], 0, new_size - old_size);
            }
            return old_memory;
        }
        void* new_memory = arena_alloc_align(a, new_size, align);
        size_t copy_size = old_size < new_size ? old_size : new_size;
        // Copy across old memory to the new memory
        memmove(new_memory, old_memory, copy_size);
        return new_memory;
    }
    assert(0 && "Memory is out of bounds of the buffer in this arena");
    return NULL;
}

// Because C doesn't have default parameters
static void* arena_resize(Arena* a, void* old_memory, size_t old_size,
                   size_t new_size) {
    return arena_resize_align(a, old_memory, old_size, new_size,
                              DEFAULT_ALIGNMENT);
}

static void arena_free_all(Arena* a) {
    a->curr_offset = 0;
    a->prev_offset = 0;
}

Arena g_bigass_arena = {0};

} // namespace memory
#pragma endregion

#pragma region FILE_ROUTINES

static inline std::uint32_t
SafeTruncateUInt64(std::uint64_t Value)
{
    IM_ASSERT(Value <= 0xFFFFFFFF);
    std::uint32_t Result = (std::uint32_t)Value;
    return(Result);
}

struct File {
    LPVOID Contents;
    size_t ContentsSize;
};

static void
DEBUGPlatformFreeFileMemory(void* Memory)
{
    if (Memory)
    {
        VirtualFree(Memory, 0, MEM_RELEASE);
    }
}

static File
DEBUGPlatformReadEntireFile(char* Filename)
{
    File Result{};
    HANDLE FileHandle = CreateFileA(Filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    if (FileHandle != INVALID_HANDLE_VALUE)
    {
        LARGE_INTEGER FileSize;
        if (GetFileSizeEx(FileHandle, &FileSize))
        {
            auto FileSize32 = SafeTruncateUInt64(FileSize.QuadPart);
            Result.Contents = VirtualAlloc(0, FileSize32, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
            if (Result.Contents)
            {
                DWORD BytesRead;
                if (ReadFile(FileHandle, Result.Contents, FileSize32, &BytesRead, 0) &&
                    (FileSize32 == BytesRead))
                {
                    // NOTE(casey): File read successfully
                    Result.ContentsSize = FileSize32;
                }
                else
                {
                    // TODO(casey): Logging
                    DEBUGPlatformFreeFileMemory(Result.Contents);
                    Result.Contents = 0;
                }
            }
            else
            {
                // TODO(casey): Logging
            }
        }
        else
        {
            // TODO(casey): Logging
        }

        CloseHandle(FileHandle);
    }
    else
    {
        // TODO(casey): Logging
    }

    return(Result);
}

static bool
DEBUGPlatformWriteEntireFile(char* Filename, uint32_t MemorySize, void* Memory)
{
    bool Result = false;

    HANDLE FileHandle = CreateFileA(Filename, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
    if (FileHandle != INVALID_HANDLE_VALUE)
    {
        DWORD BytesWritten;
        if (WriteFile(FileHandle, Memory, MemorySize, &BytesWritten, 0))
        {
            // NOTE(casey): File read successfully
            Result = (BytesWritten == MemorySize);
        }
        else
        {
            // TODO(casey): Logging
        }

        CloseHandle(FileHandle);
    }
    else
    {
        // TODO(casey): Logging
    }

    return(Result);
}
#pragma endregion


#pragma region POSSIBLE_TEXTURE_FIX
#if 0
static uint32_t g_numtextures {0};
static void release_textures() {

    TextureTableEntry* tex_entry = (TextureTableEntry*)0x252F750;
    TextureTableEntry* tex_entry_iter = tex_entry;
    // release everytexture before loading
    for(;;) {
        if (tex_entry_iter->ptrD3Texture == NULL || tex_entry_iter->ptrTextureData == NULL) {
            break;
        }
        auto tp = tex_entry_iter->ptrD3Texture->texturePointer;
        if (!tp) {
            tex_entry_iter++;
            continue;
        }
        HRESULT hr = tp->Release();
        if (FAILED(hr)) {
            printf("failed to release texture at index: %X", (uintptr_t)tex_entry_iter);
        }
        tex_entry_iter++;

    }
}
static void load_textures() {
    TextureTableEntry* tex_entry = (TextureTableEntry*)0x252F750;
    // create and load textures
    auto device = g_framework->get_d3d9_device();
    for (uint32_t i = 0; i < g_numtextures; i++) {
        Devil3Texture* t = tex_entry[i].ptrD3Texture;
        if (!t) {
            continue;
        }
        char buffer[MAX_PATH] = {0};
        sprintf(buffer, "texdump\\texture_%X.dds", (uintptr_t)t);
        //HRESULT hr = device->CreateTexture(t->width, t->height, 1u, NULL, t->format, D3DPOOL_SYSTEMMEM, &t->texturePointer, NULL);
        //HRESULT hr = D3DXCreateTextureFromFileExA(device, buffer, t->width, t->height, 1u, NULL, t->format, D3DPOOL_SYSTEMMEM, )
        HRESULT hr = D3DXCreateTextureFromFileA(device, buffer, &t->texturePointer);
        //t->dirty = false;
        if (FAILED(hr)) {
            printf("failed to create texture from file at: %X\n", (uintptr_t)t);
        }
    }
}
static void save_textures() {
    TextureTableEntry* tex_entry = (TextureTableEntry*)0x252F750;
    g_numtextures = 0;
    for (;;) {
        if (tex_entry->ptrD3Texture == NULL || tex_entry->ptrTextureData == NULL) {
            break;
        }
        char buffer[MAX_PATH] = {0};
        sprintf(buffer, "texdump\\texture_%X.dds", (uintptr_t)tex_entry->ptrD3Texture);
        HRESULT hr = D3DXSaveTextureToFileA(buffer, D3DXIFF_DDS, tex_entry->ptrD3Texture->texturePointer, NULL);
        if(FAILED(hr)) {
            printf("error saving texture:%X\n", (uintptr_t)tex_entry->ptrD3Texture);
        }
        g_numtextures += 1;
        tex_entry++;
    }
}

std::vector<Devil3Texture*> g_texture_pointers = {};
static uintptr_t detour_d3d9_load_texture_jump_back {0};

static void add_texture_pointer(Devil3Texture* ecx) {
    if(std::any_of(g_texture_pointers.begin(), g_texture_pointers.end(), [ecx](Devil3Texture* a){ return ecx == a;})) {
        return;
    }
    g_texture_pointers.push_back(ecx);
}

static void clear_texture_pointers() {
    g_texture_pointers.clear();
}

static __declspec(naked) void detour_d3d9_load_texture_sub_006E0B10(void) {
    __asm {
        pushad
        push ecx
        call add_texture_pointer
        pop ecx
        popad
    originalCode:
        sub esp, 0Ch
        push esi
        mov esi, ecx
        jmp DWORD PTR [detour_d3d9_load_texture_jump_back]
    }
}

#endif
#pragma endregion


#pragma region SAVE_STATE
#if 0
enum class SaveStateCommand {
    SS_CMD_NOOP,
    SS_CMD_SAVE,
    SS_CMD_LOAD_QUEUE,
    SS_CMD_LOAD_QUEUED,
    SS_CMD_LOAD,
};


SaveStateCommand g_save_state_command {SaveStateCommand::SS_CMD_NOOP};
static uintptr_t detour_game_main_jmp_back {0};
static BOOL*  g_game_update_flag = (BOOL*)0x00833220;
static bool g_skip_textures = false;

static void save_state_callback() {
    BOOL backup_update_flag = *g_game_update_flag;
    LPCRITICAL_SECTION cs   = (LPCRITICAL_SECTION)0x0252F358;
    BYTE* someshit          = (BYTE*)0x0081FC90;
    IDirect3DDevice9 **g_D3D9_device_dword_252F374 = (IDirect3DDevice9 **)0x0252F374;
    IDirect3DDevice9* backup_d3d_device9 = *g_D3D9_device_dword_252F374;
    switch (g_save_state_command)
    {
    case SaveStateCommand::SS_CMD_SAVE:
        EnterCriticalSection(cs);
        // pause before saving
        *g_game_update_flag = 1;
        // arena
        DEBUGPlatformWriteEntireFile("g_bigass_arena.ass", memory::g_bigass_arena.buf_len, memory::g_bigass_arena.buf);
        // data section
        DEBUGPlatformWriteEntireFile("g_data_section.ass", g_data_section_info.g_data_section_size, (void*)g_data_section_info.g_data_section_start);
        {
            CSceneGameMain* scn = devil3_sdk::get_main_scene();
            if (scn) {
                DEBUGPlatformWriteEntireFile("g_current_area", sizeof(uint16_t), &scn->currentLevel);
            }
        }
        save_textures();
        g_save_state_command = SaveStateCommand::SS_CMD_NOOP;
        // restore update flag after saving
        *g_game_update_flag = backup_update_flag;
        LeaveCriticalSection(cs);
        return;
    case SaveStateCommand::SS_CMD_LOAD:
        // pause before loading
        *g_game_update_flag = 1;

        
        EnterCriticalSection(cs);

        release_textures();

        // load arena memory
        {
            File memdump        = DEBUGPlatformReadEntireFile("g_bigass_arena.ass");
            unsigned long* w_iter = (unsigned long*)memory::g_bigass_arena.buf;
            unsigned long* r_iter = (unsigned long*)memdump.Contents;

            for (unsigned long long i = 0; i < (memory::g_bigass_arena.buf_len / sizeof(unsigned long)); i++) {
                while (InterlockedCompareExchange(&w_iter[i], r_iter[i], w_iter[i]) != w_iter[i]) {
                }
            }
            DEBUGPlatformFreeFileMemory(memdump.Contents);
        }
#if 1
        // load data section memory
        {
            File datadump       = DEBUGPlatformReadEntireFile("g_data_section.ass");
            unsigned long* w_iter = (unsigned long *)g_data_section_info.g_data_section_start;
            unsigned long* r_iter = (unsigned long *)datadump.Contents;

            for (unsigned long long i = 0; i < (g_data_section_info.g_data_section_size / sizeof(unsigned long)); i++) {
                while (InterlockedCompareExchange(&w_iter[i], r_iter[i], w_iter[i]) != w_iter[i]) {
                }
            }
            DEBUGPlatformFreeFileMemory(datadump.Contents);
        }
#endif
        load_textures();

        // restore game update flag
        *g_game_update_flag = backup_update_flag;
        *g_D3D9_device_dword_252F374 = backup_d3d_device9;
        *someshit = 3;
        //devil3_sdk::area_jump(scn->currentLevel);
#if 0
        for(Devil3Texture* pointer: g_texture_pointers) {
            pointer->texturePointer->UnlockRect(0);
            if(SUCCEEDED(pointer->texturePointer->Release())) {
                pointer->dirty = false;
            }
        }
#endif

        LeaveCriticalSection(cs);
        g_save_state_command = SaveStateCommand::SS_CMD_NOOP;
        return;

    case SaveStateCommand::SS_CMD_LOAD_QUEUE:
        File area           = DEBUGPlatformReadEntireFile("g_current_area.ass");
        devil3_sdk::area_jump((uint16_t)area.Contents);
        DEBUGPlatformFreeFileMemory(area.Contents);
        g_save_state_command = SaveStateCommand::SS_CMD_LOAD_QUEUED;
        clear_texture_pointers();
        return;
    case SaveStateCommand::SS_CMD_NOOP:
        clear_texture_pointers();
        return;
    default:
        break;
    }
    IM_ASSERT("save_state_callback() UNREACHEABLE");
}

static __declspec(naked) void detour_game_main_callback(void) {
    __asm {
        pushad 
        call save_state_callback
        popad
    originalCode:
        mov eax, DWORD PTR [g_game_update_flag]
        mov al, BYTE PTR [eax]
        jmp DWORD PTR [detour_game_main_jmp_back]
    }
}

#endif
#pragma endregion

#pragma region SCENE_LOADING
#if 0

static void scene_loaded_callback() {
#if 0
    style_switch_efx_clear_textures();
    style_switch_efx_load_textures();
    if(g_save_state_command == SaveStateCommand::SS_CMD_LOAD_QUEUED) {
        g_save_state_command = SaveStateCommand::SS_CMD_LOAD;
    }
#endif
    IM_ASSERT("UNIMPLEMENTED");
}

static uintptr_t detour_scene_loaded_jump_back {};
static __declspec(naked) void detour_scene_loaded(void) {
    __asm {
        pushad 
        call scene_loaded_callback
        popad
    originalCode:
        mov     [ecx+18738h], dl
        jmp DWORD PTR [detour_scene_loaded_jump_back]
    }
}
#endif
#pragma endregion

static __declspec(naked) void push_0megs(void) {
    _asm {
		push 0; // cant use cool constexpr size literals here smh
    }
}

static __declspec(naked) void push_16megs(void) {
    _asm {
		push (16*1024*1024); // cant use cool constexpr size literals here smh
    }
}

static __declspec(naked) void push_32megs(void) {
    _asm {
		push(32 * 1024 * 1024); // cant use cool constexpr size literals here smh
    }
}

static __declspec(naked) void push_xmegs(void) {
    _asm {
		push 1432AA0h; // cant use cool constexpr size literals here smh
    }
}
static __declspec(naked) void push_64megs(void) {
    _asm {
		push(64 * 1024 * 1024); // cant use cool constexpr size literals here smh
    }
}

static __declspec(naked) void push_128megs(void) {
    _asm {
		push(128 * 1024 * 1024); // cant use cool constexpr size literals here smh
    }
}

// dmc3se.exe+25B80B
static __declspec(naked) void push_00000800_asm(void) {
    _asm {
		push 0x000800
    }
}
// dmc3se.exe+25B82C
static __declspec(naked) void push_002b0000_asm(void) {
    _asm {
		push 0x2B0000
    }
}
// dmc3se.exe+25B852
static __declspec(naked) void push_001309c0_asm(void) {
    _asm {
		push 0x1309C0
    }
}

#if 0 // VirtualAlloc2 didnt work either :(

static inline char* align_upwards(const char* stack, size_t align) {
    assert(align > 0 && (align & (align - 1)) == 0); /* Power of 2 */
    assert(stack != 0);

    auto addr = reinterpret_cast<uintptr_t>(stack);
    if (addr % align != 0)
        addr += align - addr % align;
    assert(addr >= reinterpret_cast<uintptr_t>(stack));
    return reinterpret_cast<char*>(addr);
}

static inline char* align_downwards(const char* stack, size_t align) {
    assert(align > 0 && (align & (align - 1)) == 0); /* Power of 2 */
    assert(stack != 0);

    auto addr = reinterpret_cast<uintptr_t>(stack);
    addr -= addr % align;
    assert(addr <= reinterpret_cast<uintptr_t>(stack));
    return reinterpret_cast<char*>(addr);
}
// based on:
// https://stackoverflow.com/questions/54223343/virtualalloc2-with-memextendedparameteraddressrequirements-always-produces-error
LPVOID bounded_virtual_alloc(uintptr_t min, uintptr_t max, size_t size, SYSTEM_INFO& sys_info) noexcept {

    return nullptr;

    MEM_ADDRESS_REQUIREMENTS address_reqs = {0};
    MEM_EXTENDED_PARAMETER param          = {0};

    address_reqs.Alignment             = 0;                // any alignment
    address_reqs.LowestStartingAddress = (PVOID)min;       // PAGE_SIZE aligned
    address_reqs.HighestEndingAddress  = (PVOID)(max - 1); // PAGE_SIZE aligned, exclusive so -1

    param.Type    = MemExtendedParameterAddressRequirements;
    param.Pointer = &address_reqs;

    HMODULE kernelbase_handle = LoadLibrary("kernelbase.dll");
    if (kernelbase_handle) {
        auto p_virtual_alloc2 = (decltype(&::VirtualAlloc2))GetProcAddress(kernelbase_handle, "VirtualAlloc2");
        if (p_virtual_alloc2 != nullptr) {
            return p_virtual_alloc2(GetCurrentProcess(), (PVOID)0, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE, &param, 1);
        }
    }
    // NOTE(): slowest path slowest path
    MessageBoxA(NULL, "Could not find VirtualAlloc2 in kernelbase.dll, will try to allocate in a loop so it would be slow", "CustomAlolcator WARNING", MB_ICONINFORMATION);
    const LPVOID capcops_range = (LPVOID)0x0FFFFFFF; // we can only address like 256 mbs :(
    LPVOID lp_address          = (LPVOID)min;
    size_t dw_size             = (size_t)align_downwards((char*)(size_t)capcops_range - (size_t)lp_address, sys_info.dwAllocationGranularity);
    DWORD fl_allocation_type   = MEM_COMMIT | MEM_RESERVE;
    DWORD fl_protect           = PAGE_READWRITE; // idk capcom might be cuhrazy enough to write code in there, ask for executable just to be "safe"
    LPVOID lp_memory           = NULL;
    while (lp_memory == NULL) {
        lp_memory  = VirtualAlloc((LPVOID)lp_address, dw_size, fl_allocation_type, fl_protect);
        lp_address = (LPVOID)((DWORD_PTR)lp_address + sys_info.dwAllocationGranularity);
        dw_size    = dw_size - sys_info.dwAllocationGranularity;
        if ((lp_address >= capcops_range) || (dw_size <= 1)) {
            break;
        }
    }
    return lp_memory;
}

void custom_alolcator_virtual_alloc_in_capcom_range() noexcept {
    SYSTEM_INFO sys_info;
    GetSystemInfo(&sys_info);

    const LPVOID capcops_range = (LPVOID)0x0FFFFFFF;
    // migh as well allign here
    static size_t dw_size = (size_t)align_upwards((char*)1024_MiB, sys_info.dwAllocationGranularity);

    uint32_t min = (uint32_t)align_upwards((char*)sys_info.lpMinimumApplicationAddress, sys_info.dwAllocationGranularity);
    uint32_t max = (uint32_t)align_downwards((char*)capcops_range, sys_info.dwAllocationGranularity);

    LPVOID lp_memory = bounded_virtual_alloc(min, max, dw_size, sys_info);

    IM_ASSERT((lp_memory != NULL) && "Failed to VirtualAlloc memory for CustomAlolcator");
    HMODULE hModule = GetModuleHandle(NULL); // Get handle to current module
    IMAGE_NT_HEADERS* pNtHdr = ImageNtHeader(hModule);
    IMAGE_SECTION_HEADER* pSectionHdr = (IMAGE_SECTION_HEADER*)(pNtHdr + 1);

    for (int i = 0; i < pNtHdr->FileHeader.NumberOfSections; i++)
    {
        char* name = (char*)pSectionHdr->Name;
        if (memcmp(name, ".buffer", 7) == 0)
        {
            // Section found
            // pSectionHdr->VirtualAddress contains the RVA (Relative Virtual Address) of the section
            // hModule is the base address of the module
            char* sectionAddress = (char*)hModule + pSectionHdr->VirtualAddress;
            // pSectionHdr->Misc.VirtualSize contains the size of the section
            int sectionSize = pSectionHdr->Misc.VirtualSize;

            // Do something with sectionAddress and sectionSize
            arena_init(&g_bigass_arena, sectionAddress, sectionSize);
            arena_free_all(&g_bigass_arena);
            // pad cause capcops like to do [eax-4]
            static constexpr auto offset_into_an_alloc = 64;
            void* unused = arena_alloc(&g_bigass_arena, offset_into_an_alloc);
            IM_ASSERT((unused != NULL) && "Arena alloc returned a null pointer");
            return;
        }
        pSectionHdr++;
    }
//    arena_init(&g_bigass_arena, lp_memory, dw_size);
//    arena_free_all(&g_bigass_arena);

    // pad cause capcops like to do [eax-4]
    static constexpr auto offset_into_an_alloc = 64;
    void* unused                               = arena_alloc(&g_bigass_arena, offset_into_an_alloc);
    IM_ASSERT((unused != NULL) && "Arena alloc returned a null pointer");
    /*
bruh:
    g_alloc_hook = std::make_unique<FunctionHook>(0x6D4580, &CustomAlolcator::sub_6d4580);

    if (!g_alloc_hook->create()) {
        IM_ASSERT("Arena alloc returned a null pointer");
    }
    */

}
#endif

bool find_pe_section_init_arena() noexcept {
    HMODULE h_module                    = GetModuleHandle(NULL);
    IMAGE_NT_HEADERS* p_nt_hdr          = ImageNtHeader(h_module); // get PE info
    IMAGE_SECTION_HEADER* p_section_hdr = (IMAGE_SECTION_HEADER*)(p_nt_hdr + 1);

    for (int i = 0; i < p_nt_hdr->FileHeader.NumberOfSections; i++) {
        char* name = (char*)p_section_hdr->Name;
        if (memcmp(name, ".data", 6) == 0) {
            g_data_section_info.g_data_section_start = (uintptr_t)h_module + p_section_hdr->VirtualAddress;
            g_data_section_info.g_data_section_size  = p_section_hdr->Misc.VirtualSize;
        }
        if (memcmp(name, ".reloc", 7) == 0) { // assume we added specific section in PE segments
            // Section found
            // pSectionHdr->VirtualAddress contains the RVA (Relative Virtual Address) of the section
            // hModule is the base address of the module
            char* section_address = (char*)h_module + p_section_hdr->VirtualAddress;

            // pSectionHdr->Misc.VirtualSize contains the size of the section
            int section_size = p_section_hdr->Misc.VirtualSize;

            // Do something with sectionAddress and sectionSize
            memory::arena_init(&memory::g_bigass_arena, section_address, section_size);
            memory::arena_free_all(&memory::g_bigass_arena);

#ifndef NDEBUG
            printf("memory_arena_size: %zu\n", section_size);
#endif

            // pad a bit just in case
            static constexpr auto pad = 64;
            void* unused              = memory::arena_alloc(&memory::g_bigass_arena, pad);
            IM_ASSERT((unused != NULL) && "Arena alloc returned a null pointer");
#ifndef NDEBUG
            printf("arena_alloc: %zu\n", (uintptr_t)unused);
#endif
            g_mem_patch_applied = true;
            return true;
        }
        p_section_hdr++;
    }
    g_mem_patch_applied = false;
    return false;
}

static constexpr uint64_t round_to_pow2_size(uint32_t minimum_size, uint32_t pow2_size) {
    uint64_t result = (minimum_size + pow2_size - 1) & ~(pow2_size - 1);
    return result;
}

std::optional<std::string> CustomAlolcator::on_initialize() {

    // WARNING(): dirty hack to only init once here:
    static bool init = false;
    if (init) {
        return Mod::on_initialize();
    }
    init = true;

    g_custom_alolcator = this;

    if (!g_mem_patch_applied) {
        return Mod::on_initialize();
    }

    m_alloc_hook = std::make_unique<FunctionHook>(0x6D4580, &sub_6d4580);
    m_heap_control_sub_6D0E30_hook = std::make_unique<FunctionHook>(0x006D0E30, &heap_control_something_sub_6D0E30);
    bool nice = m_heap_control_sub_6D0E30_hook->create();

    //m_sub_65B880_hook = std::make_unique<FunctionHook>(0x0065B880, &sub_65B880_internal);
    //nice = m_sub_65B880_hook->create();

    if (!m_alloc_hook->create()) {
        return "Failed to install alolcator hook";
    }

    install_patch_absolute(0x65B806, patch01, (char*)&push_32megs, 5);  // going above 64 here crashes ui shit idk
    install_patch_absolute(0x65B810, patch02, (char*)&push_32megs, 5);  // same
    install_patch_absolute(0x65B82C, patch03, (char*)&push_64megs, 5); // push 002B0000 default
    install_patch_absolute(0x65B836, patch04, (char*)&push_64megs, 5); // push 002B0000 default
    install_patch_absolute(0x65B852, patch05, (char*)&push_32megs, 5); // push 001309C0 default
    install_patch_absolute(0x65B85C, patch06, (char*)&push_32megs, 5); // push 001309C0 default
    install_patch_absolute(0x6D53B2, patch07, (char*)&push_xmegs, 5); // push 00000800 default 

    find_pe_section_init_arena();


#if 0 // WILD SHIT main loop
    // main game loop?
    install_hook_absolute(0x403580, m_main_sub_00403580_hook, &detour_game_main_callback, &detour_game_main_jmp_back, 5);
#endif 

#if 0 // WILD SHIT textures
    // texture loading routine
    install_hook_absolute(0x006E0B10, m_d3d9_load_texture_hook_sub_006E0B10, 
        &detour_d3d9_load_texture_sub_006E0B10, &detour_d3d9_load_texture_jump_back, 6);
    //HRESULT __fastcall CustomAlolcator::d3d_sets_texture_maybe_sub_6E0DF0(Devil3Texture* texture) {
    m_hook_d3d_sets_texture_maybe_sub_6E0DF0 = std::make_unique<FunctionHook>(0x006E0DF0, &d3d_sets_texture_maybe_sub_6E0DF0);
    m_hook_d3d_sets_texture_maybe_sub_6E0DF0->create();
#endif

#if 0 // wild shit scene
    install_hook_absolute(0x005E02C3, m_scene_loaded_sub_005E02C0, &detour_scene_loaded, &detour_scene_loaded_jump_back, 6);
#endif

#if 0 // TODO(): in case someone contributes full custom memory allocator
    // second bunch
    install_patch_absolute(0x6D5420, patch08, (char*)&push_32megs, 5); // push 0000021C default
    install_patch_absolute(0x6D5455, patch09, (char*)&push_32megs, 5); // push 00000221 default 
    install_patch_absolute(0x6D54A0, patch10, (char*)&push_32megs, 5); // push 00000FC0 default
    install_patch_absolute(0x6D54AD, patch11, (char*)&push_32megs, 5); // push 00000FCF default
    // sound fx shit i think
    patch01 = Patch::create(0x65B806, (char*)&g_bigass_arena_push_asm, true); //address 
    patch02 = Patch::create(0x65B80B, {0x68, 0x00, 0x08, 0x00, 0x00},  true);  // push 00000800 default 
    patch03 = Patch::create(0x65B810, (char*)&g_bigass_arena_push_asm, true); // address 
    patch04 = Patch::create(0x65B82C, {0x68, 0x00, 0x00, 0x00, 0x2B},  true);  // push 002B0000 default 
    patch05 = Patch::create(0x65B80B, {0x68, 0x00, 0x08, 0x00, 0x00},  true);  // push 00000800 default 
    patch06 = Patch::create(0x65B836, {0x68, 0x00, 0x08, 0x00, 0x00},  true);  // push 00000800 default 
    patch07 = Patch::create(0x65B852, {0x68, 0xC0, 0x09, 0x13, 0x00},  true);  // push 001309C0 default
    patch08 = Patch::create(0x65B85C, {0x68, 0xC0, 0x09, 0x13, 0x00},  true);  // push 001309C0 default
#endif
    // hack to check if we running more memory patch outside
    g_mem_patch_applied = true;


    //DWORD oldprotect{};
    //VirtualProtect((void*)0x00C36980, 0x800, PAGE_NOACCESS, &oldprotect);
    //VirtualProtect((void*)0x01C8A600, 0x80000,    PAGE_NOACCESS, &oldprotect);
    //VirtualProtect((void*)0x01F38A80, 0x1309C0,   PAGE_NOACCESS, &oldprotect);

    return Mod::on_initialize();
}

#define sub_6D4AF0(name) char __cdecl name(uint32_t*, uint32_t)
typedef sub_6D4AF0(sub_6D4AF0_t);

static sub_6D4AF0_t* sub_6d4af0 = (sub_6D4AF0_t*)0x6D4AF0;

uintptr_t __fastcall CustomAlolcator::sub_6d4580_internal(SomeMemoryManagerShit* p_this, uintptr_t unused, size_t size) noexcept {
#if 1
    if ((size != 32_MiB) && (size != 64_MiB)) {
        return m_alloc_hook->get_original<decltype(sub_6d4580)>()(p_this, unused, size);
    }
#endif                                       // TEMP
    struct SomeStackFramePointerMaybe* ptr1; // eax
    int ptr3;                                // edi
    ptr1 = p_this->ptr1;
    if (p_this->ptr1) {
        if (p_this->uint1 == ptr1->uint1) {
            ptr3 = (int)ptr1->ptr3;
            sub_6d4af0(&size, ptr1->uint2); // aligns per buffer or some shit

            auto res = memory::arena_alloc(&memory::g_bigass_arena, size); //(char*)p_this->ptr1->ptr3 + (unsigned int)size;
            IM_ASSERT((res != NULL) && "arena_alloc failed!");
            static bool onceflag = false;
            if (!onceflag) {
                p_this->ptr1->ptr1 = res;
                p_this->ptr1->ptr3 = (void*)((uintptr_t)res + (uintptr_t)size); //(char*)p_this->ptr1->ptr3 + (unsigned int)size;
            }
            else {
                p_this->ptr1->ptr3 = res; //(char*)p_this->ptr1->ptr3 + (unsigned int)size;
            }
            IM_ASSERT(((uint32_t)p_this->ptr1->ptr3 >> 0x1C == 0x0) && "Allocation got outside the range addressable by CAPCOMs restriction. Try again and hope executable gets mapped into lower memory location");

#ifndef _NDEBUG
            printf("sub_6D4580(this=%p, unk=%x, size?=%x)\n", (void*)p_this, unused, size);
            printf("g_bigass_arena->size_left= %d\n", (memory::g_bigass_arena.buf_len - memory::g_bigass_arena.curr_offset));
            printf("arena_alloc >> 0x1C=%d\n", (uint32_t)p_this->ptr1->ptr3 >> 0x1C);
#endif
            return (uintptr_t)res;
        }
#ifndef _NDEBUG
        printf("Memory manager error 01\n");
#endif // !_NDEBUG
        return 0;
    }
#ifndef _NDEBUG
    printf("Memory manager error 01\n");
#endif // !_NDEBUG
    return 0;
}

uintptr_t __fastcall CustomAlolcator::sub_6d4580(SomeMemoryManagerShit* p_this, uintptr_t unused, size_t size) noexcept {
    return g_custom_alolcator->sub_6d4580_internal(p_this, unused, size);
}

struct benis {
    void* a1;
    void* a2;
    void* a3;
};

int32_t* g1;
int32_t g2;
uintptr_t __cdecl CustomAlolcator::heap_control_something_sub_6D0E30_internal(uint32_t a1) noexcept {
    // 0x6d0e30
    return 0;
    int32_t v1{}; // bp-52, 0x6d0e30
    g1 = &v1;
    int32_t v2{}; // bp-56, 0x6d0e30
    int32_t* v3 = &v2; // 0x6d0e39
    benis v4; // bp-44, 0x6d0e30
    //__asm_rep_stosd_memset((char*)&v4, 0, 11);
    uintptr_t(_cdecl * sub_6C72F0)(int32_t*, int, int, int, int) = (uintptr_t(_cdecl *)(int32_t*, int, int, int, int))0x006C72F0;
    int (_cdecl *sub_6C7440)(int a1, int a2)  = (int(_cdecl *)(int a1, int a2))0x006C7440;
    int32_t* (__cdecl * sub_6C73E0)(int32_t * a1, int32_t a2, int32_t a3, int32_t a4) = (int32_t * (__cdecl *)(int32_t * a1, int32_t a2, int32_t a3, int32_t a4))0x006C73E0;
    int32_t* (__cdecl * sub_6CDFF0)(int32_t a1, int32_t * a2, int32_t a3) = (int32_t * (__cdecl *)(int32_t a1, int32_t * a2, int32_t a3))0x006CDFF0;
    v4.a1 = (void*)a1;
    int32_t v5 = 16 * a1; // 0x6d0e4d
    int32_t v6 = *(int32_t*)(v5 + (int32_t)0x0252CA10 + 12); // 0x6d0e63
    int32_t* v7 = (int32_t*)(v3 - 4);
    int32_t* v8 = (int32_t*)(v3 + 24);
    int32_t* v9 = (int32_t*)(v3 + 20);
    int32_t* v10 = (int32_t*)(v3 + 28);
    int32_t v11 = *(int32_t*)(v5 + (int32_t)0x0252CA10 + 8); // 0x6d0e7b
    int32_t v12 = v6 + 16; // 0x6d0e7b
    int32_t v13 = v6; // 0x6d0e7b
    int32_t v14; // 0x6d0e30
    int32_t v15; // 0x6d0e30
    int32_t v16; // 0x6d0e30
    while (true) {
    lab_0x6d0e7f:
        // 0x6d0e7f
        v16 = v13;
        v15 = v12;
        uint32_t v17 = *(int32_t*)v11; // 0x6d0e7f
        v14 = v17;
        v12 = v15;
        v13 = v16;
        switch (v17 / 0x10000000) {
        case 15: {
            goto lab_0x6d0eb6;
        }
        case 0: {
            goto lab_0x6d0ea8;
        }
        default: {
            // 0x6d0e8d
            *v7 = *v3 + 12;
            v13 = *v8;
            v12 = *v9;
            v14 = *(int32_t*)*v10;
            goto lab_0x6d0ea8;
        }
        }
    }
lab_0x6d0eb6:;
    int32_t* v18 = (int32_t*)(v3 + 16); // 0x6d0eb6
    if (v15 == *v18 + 16) {
        // 0x6d0f51
        return (uintptr_t)v15;
    }
    int32_t v19 = 4; // 0x6d0ece
    switch (*g1) {
    case 1: {
        // 0x6d0edc
        v19 = 13;
    }
    case 0: {
    lab_0x6d0efd:;
        // 0x6d0efd
        int32_t* v20; // 0x6d0e30
        int32_t* v21; // 0x6d0e30
        int32_t* v22; // 0x6d0e30
        int32_t* v23; // 0x6d0e30
        int32_t* v24; // 0x6d0e30
        int32_t* v25; // 0x6d0e30
        int32_t v26; // 0x6d0e30
        if (v15 == v16 + 16) {
            // 0x6d0f2a
            *v9 = v16;
            v23 = (int32_t*)(v3 - 28);
            v22 = (int32_t*)(v3 - 24);
            v21 = (int32_t*)(v3 - 20);
            v20 = (int32_t*)(v3 - 16);
            v25 = (int32_t*)(v3 - 12);
            v24 = (int32_t*)(v3 - 8);
            v26 = v16;
        }
        else {
            // 0x6d0f0b
            *v7 = 0;
            int32_t v27 = (v15 - v16 >> 4) - 1; // 0x6d0f0d
            int32_t* v28 = (int32_t*)(v3 - 8);
            *v28 = 0;
            int32_t* v29 = (int32_t*)(v3 - 12);
            *v29 = v15;
            int32_t* v30 = (int32_t*)(v3 - 16);
            *v30 = v27;
            int32_t* v31 = (int32_t*)(v3 - 20);
            *v31 = v16;
            sub_6C72F0(&g2, (int32_t)&g2, (int32_t)&g2, (int32_t)&g2, (int32_t)&g2);
            int32_t* v32 = (int32_t*)(v3 - 24);
            *v32 = v27;
            int32_t* v33 = (int32_t*)(v3 - 28);
            *v33 = *v8;
            sub_6C7440((int32_t)&g2, (int32_t)&g2);
            v23 = v33;
            v22 = v32;
            v21 = v31;
            v20 = v30;
            v25 = v29;
            v24 = v28;
            v26 = *v9;
        }
        // 0x6d0f2e
        *v7 = 0;
        *v24 = 1;
        *v25 = 0;
        *v20 = v26;
        sub_6C73E0(&g2, (int32_t)&g2, (int32_t)&g2, (int32_t)&g2);
        *v21 = v19;
        *v22 = *v9;
        *v23 = *v18;
        int32_t* v34 = sub_6CDFF0((int32_t)&g2, &g2, (int32_t)&g2); // 0x6d0f49
        // 0x6d0f51
        return (uintptr_t)v34;
    }
    case 2: {
        // 0x6d0ee3
        v19 = 15;
        // branch (via goto) -> 0x6d0efd
        goto lab_0x6d0efd;
    }
    case 3: {
        // 0x6d0eea
        v19 = 19;
        // branch (via goto) -> 0x6d0efd
        goto lab_0x6d0efd;
    }
    case 4: {
        // 0x6d0ef1
        v19 = 22;
        // branch (via goto) -> 0x6d0efd
        goto lab_0x6d0efd;
    }
    case 5: {
        // 0x6d0ef8
        v19 = 25;
        // branch (via goto) -> 0x6d0efd
        goto lab_0x6d0efd;
    }
    default: {
        // 0x6d0f51
        return (uintptr_t)v15;
    }
    }
lab_0x6d0ea8:
    // 0x6d0ea8
    v11 = v14 & 0xfffffff;
    *v10 = v11;
    goto lab_0x6d0e7f;
}

uintptr_t __cdecl CustomAlolcator::heap_control_something_sub_6D0E30(uint32_t a1) noexcept {
    return g_custom_alolcator->heap_control_something_sub_6D0E30_internal(a1);
}

void* __cdecl CustomAlolcator::sub_65B880(int a1, size_t sz, int a3) {
    return malloc(sz + 10_KiB);
}

void* CustomAlolcator::sub_65B880_internal(int a1, size_t sz, int a3)
{
    return g_custom_alolcator->sub_65B880(a1, sz, a3);
}

#if 0 // WILD SHIT textures
HRESULT CustomAlolcator::d3d_sets_texture_maybe_sub_6E0DF0_internal(Devil3Texture* texture) {
    auto original_func = (decltype(CustomAlolcator::d3d_sets_texture_maybe_sub_6E0DF0)*)m_hook_d3d_sets_texture_maybe_sub_6E0DF0->get_original();

    if (g_skip_textures) {
        if (texture->texturePointer) { 
            texture->texturePointer->Release();
            texture->texturePointer = NULL;
        }
        return 0x80004005;
    }
    HRESULT res = original_func(texture);
    return res;
}

HRESULT __fastcall CustomAlolcator::d3d_sets_texture_maybe_sub_6E0DF0(Devil3Texture* texture) {
    return g_custom_alolcator->d3d_sets_texture_maybe_sub_6E0DF0_internal(texture);
}
#endif

void CustomAlolcator::on_config_load(const utility::Config& cfg) {
    for (IModValue& option : m_options) {
        option.config_load(cfg);
    }
}

void CustomAlolcator::on_config_save(utility::Config& cfg) {
    for (IModValue& option : m_options) {
        option.config_save(cfg);
    }
}

void CustomAlolcator::on_draw_debug_ui() {

    void* base     = memory::g_bigass_arena.buf;
    size_t size    = memory::g_bigass_arena.buf_len;
    size_t cursor  = memory::g_bigass_arena.curr_offset;
    size_t sz_left = size - cursor;


    ImGui::Text("Memory allocator:\n\tbase=%p\n\tsize=%zu\n\tcursor=%zu\n\tsize_left=%zu\n\t", base, size / 1024, cursor, sz_left / 1024);

#if 0 // WILD SHIT textures
    ImGui::Text("texture_num: %zu", g_texture_pointers.size());
    ImGui::Checkbox("skip_textures", &g_skip_textures);

    if (ImGui::TreeNode("texture_list")) {
        for (Devil3Texture* texture : g_texture_pointers) {
            if (ImGui::TreeNode((void*)texture, "Texture %p", texture)) {
                ImGui::InputInt("width", (int*)&texture->width);
                ImGui::InputInt("height", (int*)&texture->height);
                ImGui::Text("decodeDataPointer: %p", texture->decodeDataPointer);
                ImGui::Text("decodeFunctionPointer: %p", texture->decodeFunctionPointer);
                ImGui::InputInt("format", (int*)&texture->format);
                if (ImGui::TreeNode((void*)texture->texturePointer, "d3dtexture %p", texture->texturePointer)) {
                    ImGui::Image(texture->texturePointer, ImVec2(texture->width, texture->height));
                    ImGui::TreePop();
                }
                ImGui::Checkbox("dirty", &texture->dirty);
                ImGui::TreePop();
            }
        }
        ImGui::TreePop();
    }
#endif
}
// will show up in main window, dump ImGui widgets you want here

void CustomAlolcator::on_draw_ui() {
    if (ImGui::CollapsingHeader("Memory Alolcator Adjustments")) {
        if(g_mem_patch_applied) {
            ImGui::TextColored(ImColor(IM_COL32(85,217,133,255)), "Memory allocation patches are applied!");
        }
        else {
            ImGui::Text("Missing .pe section memory patches are not applied!");
        }
        ImGui::Text("Saved memory states:");
#if 0 // WILD SHIT savestate
        if (ImGui::Button("Dump memory")) {
            g_save_state_command = SaveStateCommand::SS_CMD_SAVE;
        }
        if (ImGui::Button("Load memory")) {
            g_save_state_command = SaveStateCommand::SS_CMD_LOAD;
        }
#endif
    }
}
