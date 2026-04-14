#ifndef PARSING_ARENA_H_
#define PARSING_ARENA_H_

#include <cstddef>

namespace bin_unicorn {

/*
 * This arena is useful for allowing cJSON to allocate without using malloc/free or accessing the
 * heap.
 *
 * It is not thread-safe, as we need to stash an arena pointer in a global variable to be able to
 * pass allocator hooks to cJSON, which are C function pointers and so can't capture a particular
 * instance of an arena.
 */

struct Arena {
    Arena(std::byte *buffer, size_t size) : buffer(buffer), capacity(size), offset(0) {}

    std::byte *buffer;
    size_t capacity;
    size_t offset;
};

void arena_set_current(Arena &arena);

void *arena_malloc(size_t size);

void arena_free(void *ptr);

void arena_unset_current();

} // namespace bin_unicorn

#endif // PARSING_ARENA_H_