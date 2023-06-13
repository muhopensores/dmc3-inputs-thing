#include "CustomAlolcator.hpp"

inline std::uint32_t
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


// to check if allocation succeded in other mods (LDK so far)
CustomAlolcator* g_custom_alolcator{ nullptr };

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

// code stolen from gingerBills article on memory allocators
// https://www.gingerbill.org/article/2019/02/08/memory-allocation-strategies-002/

#include <stddef.h>
#include <stdint.h>

#if !defined(__cplusplus)
#if (defined(_MSC_VER) && _MSC_VER < 1800) || (!defined(_MSC_VER) && !defined(__STDC_VERSION__))
#ifndef true
#define true  (0 == 0)
#endif
#ifndef false
#define false (0 != 0)
#endif
typedef unsigned char bool;
#else
#include <stdbool.h>
#endif
#endif

#include <stdio.h>
#include <assert.h>
#include <string.h>

bool is_power_of_two(uintptr_t x) {
	return (x & (x - 1)) == 0;
}

uintptr_t align_forward(uintptr_t ptr, size_t align) {
	uintptr_t p, a, modulo;

	assert(is_power_of_two(align));

	p = ptr;
	a = (uintptr_t)align;
	// Same as (p % a) but faster as 'a' is a power of two
	modulo = p & (a - 1);

	if (modulo != 0) {
		// If 'p' address is not aligned, push the address to the
		// next value which is aligned
		p += a - modulo;
	}
	return p;
}

#ifndef DEFAULT_ALIGNMENT
#define DEFAULT_ALIGNMENT (2*sizeof(void *))
#endif

typedef struct Arena Arena;
struct Arena {
	unsigned char* buf;
	size_t         buf_len;
	size_t         prev_offset; // This will be useful for later on
	size_t         curr_offset;
};

void arena_init(Arena* a, void* backing_buffer, size_t backing_buffer_length) {
	a->buf = (unsigned char*)backing_buffer;
	a->buf_len = backing_buffer_length;
	a->curr_offset = 0;
	a->prev_offset = 0;
#if 0 // was checking for float math fucking up 0x7ff8dead is NaN when interpreted as float, the more you know
	uint32_t* iter = (uint32_t*)a->buf;
	for (size_t i = 0; i < (a->buf_len / sizeof(int)); i++) {
		iter[i] = 0x7ff8dead;
	}
#endif
}


void* arena_alloc_align(Arena* a, size_t size, size_t align) {
	// Align 'curr_offset' forward to the specified alignment
	uintptr_t curr_ptr = (uintptr_t)a->buf + (uintptr_t)a->curr_offset;
	uintptr_t offset = align_forward(curr_ptr, align);
	offset -= (uintptr_t)a->buf; // Change to relative offset

	// Check to see if the backing memory has space left
	if (offset + size <= a->buf_len) {
		void* ptr = &a->buf[offset];
		a->prev_offset = offset;
		a->curr_offset = offset + size;

		// Zero new memory by default
		memset(ptr, 0, size);
		return ptr;
	}
	// Return NULL if the arena is out of memory (or handle differently)
	return NULL;
}

// Because C doesn't have default parameters
void* arena_alloc(Arena* a, size_t size) {
	return arena_alloc_align(a, size, DEFAULT_ALIGNMENT);
}

void arena_free(Arena* a, void* ptr) {
	// Do nothing
}

void* arena_resize_align(Arena* a, void* old_memory, size_t old_size, size_t new_size, size_t align) {
	unsigned char* old_mem = (unsigned char*)old_memory;

	assert(is_power_of_two(align));

	if (old_mem == NULL || old_size == 0) {
		return arena_alloc_align(a, new_size, align);
	}
	else if (a->buf <= old_mem && old_mem < a->buf + a->buf_len) {
		if (a->buf + a->prev_offset == old_mem) {
			a->curr_offset = a->prev_offset + new_size;
			if (new_size > old_size) {
				// Zero the new memory by default
				memset(&a->buf[a->curr_offset], 0, new_size - old_size);
			}
			return old_memory;
		}
		else {
			void* new_memory = arena_alloc_align(a, new_size, align);
			size_t copy_size = old_size < new_size ? old_size : new_size;
			// Copy across old memory to the new memory
			memmove(new_memory, old_memory, copy_size);
			return new_memory;
		}

	}
	else {
		assert(0 && "Memory is out of bounds of the buffer in this arena");
		return NULL;
	}

}

// Because C doesn't have default parameters
void* arena_resize(Arena* a, void* old_memory, size_t old_size, size_t new_size) {
	return arena_resize_align(a, old_memory, old_size, new_size, DEFAULT_ALIGNMENT);
}

void arena_free_all(Arena* a) {
	a->curr_offset = 0;
	a->prev_offset = 0;
}

// Extra Features
typedef struct Temp_Arena_Memory Temp_Arena_Memory;
struct Temp_Arena_Memory {
	Arena* arena;
	size_t prev_offset;
	size_t curr_offset;
};

Temp_Arena_Memory temp_arena_memory_begin(Arena* a) {
	Temp_Arena_Memory temp;
	temp.arena = a;
	temp.prev_offset = a->prev_offset;
	temp.curr_offset = a->curr_offset;
	return temp;
}

void temp_arena_memory_end(Temp_Arena_Memory temp) {
	temp.arena->prev_offset = temp.prev_offset;
	temp.arena->curr_offset = temp.curr_offset;
}
#if 0
int main(int argc, char** argv) {
	int i;

	unsigned char backing_buffer[256];
	Arena a = { 0 };
	arena_init(&a, backing_buffer, 256);

	for (i = 0; i < 10; i++) {
		int* x;
		float* f;
		char* str;

		// Reset all arena offsets for each loop
		arena_free_all(&a);

		x = (int*)arena_alloc(&a, sizeof(int));
		f = (float*)arena_alloc(&a, sizeof(float));
		str = arena_alloc(&a, 10);

		*x = 123;
		*f = 987;
		memmove(str, "Hellope", 7);

		printf("%p: %d\n", x, *x);
		printf("%p: %f\n", f, *f);
		printf("%p: %s\n", str, str);

		str = arena_resize(&a, str, 10, 16);
		memmove(str + 7, " world!", 7);
		printf("%p: %s\n", str, str);
	}

	arena_free_all(&a);

	return 0;
}
#endif

static Arena g_bigass_arena = { 0 };

std::optional<std::string> CustomAlolcator::on_initialize() {
	
	// WARNING(): dirty hack to only init once here:
	static bool init = false;
	if (init) {
		return Mod::on_initialize();
	}
	init = true;

	g_custom_alolcator = this;


	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);
	LPVOID lpMinimumApplicationAddress = sysInfo.lpMinimumApplicationAddress;
	LPVOID lpMaximumApplicationAddress = sysInfo.lpMaximumApplicationAddress;
	printf("lpMinimumApplicationAddress=%p;\tlpMaximumApplicationAddress=%p\n", lpMinimumApplicationAddress, lpMaximumApplicationAddress);

	LPVOID lpAddress = (void*)(256_MiB >> 1);//(void*)(2048*sysInfo.dwAllocationGranularity);//(void*)align_forward(1_GiB, DEFAULT_ALIGNMENT); // any address doesnt fucking work wth microsoft?!
	constexpr auto dwSize = 512_MiB;
	DWORD flAllocationType = MEM_COMMIT | MEM_RESERVE;
	DWORD flProtect = PAGE_EXECUTE_READWRITE; // idk capcom might be cuhrazy enough to write code in there, ask for executable just to be "safe"
	LPVOID lpMemory = VirtualAlloc(lpAddress, dwSize, flAllocationType, flProtect);
	IM_ASSERT((lpAddress < lpMaximumApplicationAddress) && (lpAddress > lpMinimumApplicationAddress));
	IM_ASSERT(lpMemory != NULL);

	if (lpMemory == NULL) {
		printf("Virtual alloc failed with code 0x%X.\n", GetLastError());
		printf("failed to virtual alloc\n");
		return "failed to allocate memory arena";
	}
	printf("\n================= COOL MEMORY ARENA start: %p =================\n", lpMemory);

	m_alloc_hook = std::make_unique<FunctionHook>(0x6D4580, &sub_6D4580);

	arena_init(&g_bigass_arena, lpMemory, dwSize);
	arena_free_all(&g_bigass_arena);

	// pad cause capcops like to do [eax-4]
	static constexpr auto offset_into_an_alloc = 1024;
	void* unused = arena_alloc(&g_bigass_arena, offset_into_an_alloc);

	IM_ASSERT(unused != NULL);

	if (!m_alloc_hook->create()) {
		return "Failed to install alolcator hook";
	}
	// hack to check if we running more memory patch outside
	mem_patch_applied = true;

	return Mod::on_initialize();
}


uintptr_t __fastcall CustomAlolcator::sub_6D4580_internal(uintptr_t p_this, uintptr_t a2, uintptr_t a3)
{
	uintptr_t res = (uintptr_t)arena_alloc(&g_bigass_arena, a3*2);
	*(DWORD*)(p_this + 8) += a2;

#ifndef _NDEBUG
	printf("sub_6D4580(this=%p, unk=%d, size?=%d)\n", (void*)p_this, a2, a3);
	printf("g_bigass_arena->size_left= %d\n", (g_bigass_arena.buf_len - g_bigass_arena.curr_offset));
	IM_ASSERT(res != NULL && "oh no arena_alloc returned a NULL ");
#endif
	if (res == NULL) { // call orig in case we ran out of 1 gig or some wacky shit happens
		res = m_alloc_hook->get_original<decltype(sub_6D4580)>()(p_this, a2, a3);
	}

	return res;
	//return (uintptr_t)arena_alloc(&g_bigass_arena,a3);
}

uintptr_t __fastcall CustomAlolcator::sub_6D4580(uintptr_t p_this, uintptr_t a2, uintptr_t a3) {
	return g_custom_alolcator->sub_6D4580_internal(p_this, a2, a3);
}

// during load
//void CustomAlolcator::on_config_load(const utility::Config &cfg) {
//	for (IModValue& option : m_options) {
//		option.config_load(cfg);
//	}
//}
// during save
//void CustomAlolcator::on_config_save(utility::Config &cfg) {
//	for (IModValue& option : m_options) {
//		option.config_save(cfg);
//	}
//}
// do something every frame
//void CustomAlolcator::on_frame() {}
// will show up in debug window, dump ImGui widgets you want here
//void CustomAlolcator::on_draw_debug_ui() {}
// will show up in main window, dump ImGui widgets you want here
void CustomAlolcator::on_draw_ui() {
	if (ImGui::CollapsingHeader("Memory Alolcator Adjustments")) {
		ImGui::Text("TODOOOOO");
		if (ImGui::Button("Dump memory")) {
			DEBUGPlatformWriteEntireFile("g_bigass_arena.ass", g_bigass_arena.buf_len, g_bigass_arena.buf);
		}
		if (ImGui::Button("Load memory")) {
			static File memdump = DEBUGPlatformReadEntireFile("g_bigass_arena.ass");
			unsigned long long* w_iter = (unsigned long long*)g_bigass_arena.buf;
			unsigned long long* r_iter = (unsigned long long*)memdump.Contents;
			
			for (unsigned long long i = 0; i < (g_bigass_arena.buf_len / sizeof(unsigned long long)); i++) {
				while (InterlockedCompareExchange(&w_iter[i], r_iter[i], w_iter[i]) != w_iter[i]) {
				}
			}
			DEBUGPlatformFreeFileMemory(memdump.Contents);
		}
	}
}