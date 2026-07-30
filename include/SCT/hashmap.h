#ifndef SCT_HASHMAP_H
#define SCT_HASHMAP_H

#include "list.h"
#include "vector.h"
#include "arena.h"

#define SCT_HASHMAP_BUCKETS_NUM 128

#define SCT_HASHMAP_CONTAINER_SIZE sizeof(char*) + map->_item_size

typedef struct {
    sct_list_t   buckets[SCT_HASHMAP_BUCKETS_NUM];
    sct_arena_t  arena;
    sct_vector_t keys;
    size_t       _item_size;
} sct_hashmap_t;

void sct_hashmap_init(sct_hashmap_t *map, size_t item_size);

void sct_hashmap_deinit(sct_hashmap_t *map);

int sct_hashmap_constains(sct_hashmap_t *map, const char *key);

void sct_hashmap_add(sct_hashmap_t *map, const char *key, void *item);

void *sct_hashmap_get(sct_hashmap_t *map, const char *key);

void sct_hasmap_remove(sct_hashmap_t *map, const char *key);

#endif