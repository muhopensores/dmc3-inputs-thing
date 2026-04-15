#pragma once

namespace utility {
typedef struct Arena Arena;
struct Arena {
    unsigned char* buf;
    size_t buf_len;
    size_t prev_offset; // This will be useful for later on
    size_t curr_offset;
};

void  arena_init(Arena* a, void* backing_buffer, size_t backing_buffer_length);
void* arena_alloc(Arena* a, size_t size);
void  arena_free(Arena* a, void* ptr);
void  arena_free_all(Arena* a);
void* arena_resize(Arena* a, void* old_memory, size_t old_size, size_t new_size);
}; // utility
