#include "MemArena.hpp"

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

namespace utility {
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

void arena_init(Arena* a, void* backing_buffer, size_t backing_buffer_length) {
    a->buf         = (unsigned char*)backing_buffer;
    a->buf_len     = backing_buffer_length;
    a->curr_offset = 0;
    a->prev_offset = 0;
#if 0 // was checking for float math fucking up;
      //0x7ff8dead is NaN when interpreted as float, the more you know
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
    assert(0 && "MemoryArena ran out!");
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
void* arena_resize(Arena* a, void* old_memory, size_t old_size,
                   size_t new_size) {
    return arena_resize_align(a, old_memory, old_size, new_size,
                              DEFAULT_ALIGNMENT);
}

void arena_free_all(Arena* a) {
    a->curr_offset = 0;
    a->prev_offset = 0;
}

} // namespace utility
