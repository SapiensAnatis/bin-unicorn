#include "parsing/arena.hpp"

#include <array>
#include <cassert>
#include <cstdio>

namespace bin_unicorn {

static Arena *current_arena = nullptr;

void arena_set_current(Arena &arena) { current_arena = &arena; }

void *arena_malloc(size_t size) {
    assert(current_arena != nullptr);

    size_t alignment = alignof(max_align_t);
    size_t aligned_offset = ((current_arena->offset + alignment - 1) / alignment) * alignment;

    if (aligned_offset + size > current_arena->capacity) {
        fprintf(stderr, "Arena OOM: need %zu, have %zu\n", size,
                current_arena->capacity - aligned_offset);
        return nullptr;
    }

    void *ptr = current_arena->buffer + aligned_offset;
    current_arena->offset = aligned_offset + size;
    return ptr;
}

void arena_free(void *ptr) {
    // no-op: memory is freed when arena is destroyed
}

void arena_unset_current() { current_arena = nullptr; }

} // namespace bin_unicorn