#include <chrono>
#include <thread>
#include <Windows.h>

#include "ModFramework.hpp"
#include "mods/CustomAlolcator.hpp"
#include "utility/ExceptionHandler.hpp"
#include "mods/RendererReplace.hpp"
#include "mods/CustomAlolcator.hpp"

static HMODULE g_dinput;
static HMODULE g_styleswitcher;
//static Mod* g_renderer_replace{ nullptr };

#if 1
extern "C" {
// DirectInput8Create wrapper for dinput8.dll
__declspec(dllexport) HRESULT WINAPI direct_input8_create(HINSTANCE hinst, DWORD dw_version, const IID& riidltf, LPVOID* ppv_out, LPUNKNOWN punk_outer) {
// This needs to be done because when we include dinput.h in DInputHook,
// It is a redefinition, so we assign an export by not using the original name
#pragma comment(linker, "/EXPORT:DirectInput8Create=_direct_input8_create@20")
    return ((decltype(direct_input8_create)*)GetProcAddress(g_dinput, "DirectInput8Create"))(hinst, dw_version, riidltf, ppv_out, punk_outer);
}
}
#endif

static constexpr uint64_t round_to_pow2_size(uint32_t minimum_size, uint32_t pow2_size) {
    uint64_t result = (minimum_size + pow2_size - 1) & ~(pow2_size - 1);
    return result;
}

void failed() {
    MessageBox(0, "DMC3 ModFramework: Unable to load the original version.dll. Please report this to the developer.", "ModFramework", 0);
    ExitProcess(0);
}

void WINAPI startup_thread() {
#ifndef NDEBUG
    AllocConsole();
    HANDLE handle_out = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD console_mode;
    GetConsoleMode(handle_out, &console_mode);
    console_mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    console_mode |= DISABLE_NEWLINE_AUTO_RETURN;
    SetConsoleMode(handle_out, console_mode);

    FILE* f_inp;
    FILE* f_out;

    freopen_s(&f_inp, "CONIN$", "r", stdin);
    freopen_s(&f_out, "CONOUT$", "w", stdout);
    freopen_s(&f_out, "CONOUT$", "w", stderr);

#endif

#if 1
    wchar_t buffer[MAX_PATH]{0};
    if (GetSystemDirectoryW(buffer, MAX_PATH) != 0) {
        // Load the original dinput8.dll
        if ((g_dinput = LoadLibraryW((std::wstring{buffer} + L"\\dinput8.dll").c_str())) == NULL) {
            failed();
        }
        //g_framework = std::make_unique<ModFramework>();
#ifdef NDEBUG
        reframework::setup_exception_handler();
#endif
    } else {
        failed();
    }
    g_framework = std::make_unique<ModFramework>();
#else
#endif
}

#if 0
HANDLE filemapping {};
PULONG_PTR page_array = NULL;
ULONG_PTR number_of_pages = NULL;

static BOOL EnableAWE() {
    HANDLE hToken;
    TOKEN_PRIVILEGES tp;

    // Open the process token with TOKEN_ADJUST_PRIVILEGES and TOKEN_QUERY
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
        return FALSE;
    }

    tp.PrivilegeCount = 1;
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    // Lookup the LUID for SeLockMemoryPrivilege
    if (!LookupPrivilegeValue(NULL, SE_LOCK_MEMORY_NAME, &(tp.Privileges[0].Luid))) {
        CloseHandle(hToken);
        return FALSE;
    }

    // Adjust the token privileges
    if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), NULL, NULL)) {
        CloseHandle(hToken);
        return FALSE;
    }

    CloseHandle(hToken);
    return TRUE;
}
#endif

BOOL APIENTRY DllMain(HMODULE handle, DWORD reason, LPVOID reserved) {
    if (reason == DLL_PROCESS_ATTACH) {
#ifndef NDEBUG
        MessageBox(NULL, "Debug attach opportunity", "DMC3", MB_ICONINFORMATION);
#endif
        bool pe_section_found = find_pe_section_init_arena();
        if (!pe_section_found) {
            g_styleswitcher = LoadLibraryA("StyleSwitcher.dll");
        }
        else {
            g_styleswitcher = LoadLibraryA("StyleSwitcherPatched.dll");
        }
#if 0
        SYSTEM_INFO info;
        MEMORY_BASIC_INFORMATION mbi{};
        VirtualQuery((LPCVOID)0x00C36980, &mbi, sizeof(mbi));
        GetSystemInfo(&info);
        DWORD memory_size = 0x8200000;
#endif
#if 0
        number_of_pages = memory_size / info.dwPageSize;
        page_array = new ULONG_PTR[number_of_pages];
        if(!EnableAWE()) {
            fprintf(stderr, "Failed to EnableSEPrivelege\n");
            DWORD err = GetLastError();
        }
        if (!AllocateUserPhysicalPages(GetCurrentProcess(), &number_of_pages, page_array)) {
            fprintf(stderr, "Failed to AllocateUserPhysicalPage\n");
            DWORD err = GetLastError();
        }
        auto lpMemReserved = VirtualAlloc((void*) 0x00c36980,
            memory_size,
            MEM_RESERVE | MEM_PHYSICAL,
            PAGE_READWRITE );
        if (!lpMemReserved) {
            return TRUE;
        }
        PVOID virtual_address = (PVOID)0x00C36980;
        if (!MapUserPhysicalPages(lpMemReserved, number_of_pages, page_array)) {
            fprintf(stderr, "Failed to MapUserPhysicalPages(0x%X)\n", (DWORD)virtual_address);
            DWORD err = GetLastError();
        }

        virtual_address = (PVOID)0x2537000;
        if (!MapUserPhysicalPages(virtual_address, number_of_pages, page_array)) {
            fprintf(stderr, "Failed to MapUserPhysicalPages(0x%X)\n", (DWORD)virtual_address);
            DWORD err = GetLastError();
        }
#endif


#if 0
        uint64_t data_size = round_to_pow2_size(0x8200000, info.dwAllocationGranularity);
        filemapping = CreateFileMapping(INVALID_HANDLE_VALUE, 0, PAGE_READWRITE,
            (DWORD)(data_size >> 32), (DWORD)(data_size & 0xffffffff), 0);

        if (filemapping != INVALID_HANDLE_VALUE) {
            if(filemapping == nullptr) { goto continuers; }
            uint64_t dmc3_offset_p2 = round_to_pow2_size(0x00C36980, info.dwAllocationGranularity);
            uint32_t dmc3offset = (dmc3_offset_p2 & 0xffffffff);
            if (!MapViewOfFile3(filemapping, 0, (void*)(dmc3offset),
                0, round_to_pow2_size(0x1432AC0, info.dwAllocationGranularity), MEM_REPLACE_PLACEHOLDER, PAGE_READWRITE, 0, 0)) {
                DWORD er = GetLastError();
                printf("eror with MapViewOfFile3(filemapping, 0, (void*)0x00C36980, 0, 0x1432AC0, MEM_REPLACE_PLACEHOLDER, PAGE_READWRITE, 0, 0)\n");
            }
#if 0
            if (!MapViewOfFile3(filemapping, 0, (void*)0x2537000,
                0, data_size, MEM_REPLACE_PLACEHOLDER, PAGE_READWRITE, 0, 0)) {
                DWORD er = GetLastError();
                printf("eror with MapViewOfFile3(filemapping, 0, (void*)0x2537000, 0, data_size, MEM_REPLACE_PLACEHOLDER, PAGE_READWRITE, 0, 0)\n");
            }
#endif
        }
#endif

        //g_renderer_replace = new RendererReplace();
        //g_renderer_replace->on_initialize();
        CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)startup_thread, nullptr, 0, nullptr);
    }
    if (reason == DLL_PROCESS_DETACH) {
        FreeLibrary(g_styleswitcher);
        FreeLibrary(g_dinput);
    }
    return TRUE;
}
