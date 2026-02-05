#ifndef SCT_HASHMAP_H
#define SCT_HASHMAP_H

#include <string.h>
#include "list.h"
#include "crc32.h"

#define SCT_HASHMAP_KEY_SIZE     64
#define SCT_HASHMAP_BUCKETS_NUM  2
#define SCT_HASHMAP_KEYS_MAX_NUM 256

typedef struct {
    char  key[SCT_HASHMAP_KEY_SIZE];
    void *val;
} __sct_hashmap_container_t;

typedef struct {
    sct_list_pair_t *buckets_list[SCT_HASHMAP_BUCKETS_NUM];
    sct_list_pair_t *keys;
} sct_hashmap_t;

sct_hashmap_t *sct_hashmap_create();

// возвращает указатель на старое значение под ключом, если его не существовало возвращает 0
void *sct_hashmap_set(sct_hashmap_t *map, char *key, void *val);

// ret 0 если значения не существует
void *sct_hashmap_get(sct_hashmap_t *map, char *key);

// возвращает 0 если элемента не существует
int sct_hashmap_delete(sct_hashmap_t *map, char *key);

// возвращает 0 если элемента не существует
int sct_hashmap_full_delete(sct_hashmap_t *map, char *key);

void sct_hashmap_free(sct_hashmap_t *map);

void sct_hashmap_full_free(sct_hashmap_t *map);

#endif