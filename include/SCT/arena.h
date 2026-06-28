#ifndef SCT_ARENA_H
#define SCT_ARENA_H

#include "common.h"

// size of every allocating of sct_arena_t (including first allocating)
#define SCT_ARENA_ALLOC_SIZE 2 * 1024
// size limit after which sct_arena_free completely reallocates the arena as if sct_arena_init was called
#define SCT_ARENA_FREE_NONALLOC_LIMIT (SCT_ARENA_ALLOC_SIZE * 2)

typedef struct {
    u8    *arena;
    size_t size;
    size_t cap;
} sct_arena_t;

void sct_arena_init(sct_arena_t *arena);

void sct_arena_deinit(sct_arena_t *arena);

void *sct_arena_alloc(sct_arena_t *arena, size_t size);

void sct_arena_free(sct_arena_t *arena);

#endif