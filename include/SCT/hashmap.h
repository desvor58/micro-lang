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

sct_hashmap_t *sct_hashmap_create()
{
    sct_hashmap_t *map = malloc(sizeof(sct_hashmap_t));
    for (size_t i = 0; i < SCT_HASHMAP_BUCKETS_NUM; i++) {
        map->buckets_list[i] = sct_list_pair_create(0);
    }
    map->keys = sct_list_pair_create(0);

    return map;
}

// возвращает указатель на старое значение под ключом, если его не существовало возвращает 0
void *sct_hashmap_set(sct_hashmap_t *map, char *key, void *val)
{
    char *key_copy = malloc(sizeof(char) * strlen(key) + 1);
    strcpy(key_copy, key);
    sct_list_push_back(map->keys, key_copy);

    sct_list_pair_t *bkts = map->buckets_list[crc32(key, strlen(key)) % SCT_HASHMAP_BUCKETS_NUM];
    foreach (bkt_it, bkts) {
        if (!strcmp(((__sct_hashmap_container_t*)bkt_it->val)->key, key)) {
            void *val_acc = ((__sct_hashmap_container_t*)bkt_it->val)->val;
            free((__sct_hashmap_container_t*)bkt_it->val);
            ((__sct_hashmap_container_t*)bkt_it->val)->val = val;
            return val_acc;
        }
    }

    __sct_hashmap_container_t *new_bkt = malloc(sizeof(__sct_hashmap_container_t));
    strcpy(new_bkt->key, key);
    new_bkt->val = val;
    sct_list_push_back(bkts, new_bkt);
    return 0;
}

// ret 0 если значения не существует
void *sct_hashmap_get(sct_hashmap_t *map, char *key)
{
    sct_list_pair_t *bkts = map->buckets_list[crc32(key, strlen(key)) % SCT_HASHMAP_BUCKETS_NUM];
    foreach (bkt_it, bkts) {
        if (!strcmp(((__sct_hashmap_container_t*)bkt_it->val)->key, key)) {
            return ((__sct_hashmap_container_t*)bkt_it->val)->val;
        }
    }
    return 0;
}

// возвращает 0 если элемента не существует
int sct_hashmap_delete(sct_hashmap_t *map, char *key)
{
    size_t i = 0;
    foreach (key_it, map->keys) {
        if (!strcmp(key_it->val, key)) {
            map->keys = sct_list_full_delete(map->keys, i);
            break;
        }
        i++;
    }

    sct_list_pair_t *bkts = map->buckets_list[crc32(key, strlen(key)) % SCT_HASHMAP_BUCKETS_NUM];
    i = 0;
    foreach (bkt_it, bkts) {
        if (!strcmp(((__sct_hashmap_container_t*)bkt_it->val)->key, key)) {
            bkts = sct_list_full_delete(bkts, i);
            map->buckets_list[crc32(key, strlen(key)) % SCT_HASHMAP_BUCKETS_NUM] = bkts;
            return 0;
        }
        i++;
    }
    return 1;
}

// возвращает 0 если элемента не существует
int sct_hashmap_full_delete(sct_hashmap_t *map, char *key)
{
    size_t i = 0;
    foreach (key_it, map->keys) {
        if (!strcmp(key_it->val, key)) {
            map->keys = sct_list_full_delete(map->keys, i);
            break;
        }
        i++;
    }

    sct_list_pair_t *bkts = map->buckets_list[crc32(key, strlen(key)) % SCT_HASHMAP_BUCKETS_NUM];
    i = 0;
    foreach (bkt_it, bkts) {
        if (!strcmp(((__sct_hashmap_container_t*)bkt_it->val)->key, key)) {
            free(((__sct_hashmap_container_t*)bkt_it->val)->val);
            bkts = sct_list_full_delete(bkts, i);
            map->buckets_list[crc32(key, strlen(key)) % SCT_HASHMAP_BUCKETS_NUM] = bkts;
            return 0;
        }
        i++;
    }
    return 1;
}

void sct_hashmap_free(sct_hashmap_t *map)
{
    sct_list_full_free(map->keys);
    
    for (size_t i = 0; i < SCT_HASHMAP_BUCKETS_NUM; i++) {
        sct_list_free(map->buckets_list[i]);
    }

    free(map);
}

void sct_hashmap_full_free(sct_hashmap_t *map)
{
    sct_list_full_free(map->keys);
    
    for (size_t i = 0; i < SCT_HASHMAP_BUCKETS_NUM; i++) {
        foreach (bkt_it, map->buckets_list[i]) {
            free(((__sct_hashmap_container_t*)bkt_it->val)->val);
        }
        sct_list_free(map->buckets_list[i]);
    }

    free(map);
}

#endif