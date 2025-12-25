#ifndef HASHMAP_H
#define HASHMAP_H

#include "list.h"
#include "crc32.h"

#define HASHMAP_KEY_SIZE    64
#define HASHMAP_BUCKETS_NUM 64
#define HASHMAP_KEYS_NUM    256

#define genhashmap(Ty)  \
typedef struct {  \
    char key[HASHMAP_KEY_SIZE];  \
    Ty *val;  \
} hashmap_##Ty##_container_t;  \
genlist(hashmap_##Ty##_container_t)  \
typedef struct {  \
    list_hashmap_##Ty##_container_t_pair_t *buckets[HASHMAP_BUCKETS_NUM];  \
    char keys[(HASHMAP_KEY_SIZE + 1) * HASHMAP_KEYS_NUM];  \
    size_t keys_top;  \
} hashmap_##Ty##_t;  \
hashmap_##Ty##_t *hashmap_##Ty##_create()  \
{  \
    hashmap_##Ty##_t *map = malloc(sizeof(hashmap_##Ty##_t));  \
    for (size_t i = 0; i < HASHMAP_BUCKETS_NUM; i++) {  \
        map->buckets[i] = list_hashmap_##Ty##_container_t_create();  \
    }  \
    map->keys_top = 0;  \
    return map;  \
}  \
int hashmap_##Ty##_set(hashmap_##Ty##_t *map, const char *key, Ty *val) /* val обязательно выделен в куче! */ \
{  \
    if (map == 0 || !key || strlen(key) > HASHMAP_KEY_SIZE || val == 0) {  \
        return 1;  \
    }  \
    strcpy(map->keys + ((map->keys_top++) * (HASHMAP_KEY_SIZE + 1)), key);  \
    crc32_t bkt = crc32(key, strlen(key)) % HASHMAP_BUCKETS_NUM;  \
    hashmap_##Ty##_container_t *container = malloc(sizeof(hashmap_##Ty##_container_t));  \
    strcpy(container->key, key);  \
    container->val = val;  \
    foreach (list_hashmap_##Ty##_container_t_pair_t, map->buckets[bkt]) {  \
        if (!strcmp(cur->val->key, key)) {  \
            /* cur->val->val остаеться выделеным в случае если он был выделен в куче */  \
            free(cur->val);  \
            cur->val = container;  \
            return 0;  \
        }  \
    }  \
    list_hashmap_##Ty##_container_t_add(map->buckets[bkt], container);  \
    return 0;  \
}  \
Ty *hashmap_##Ty##_get(hashmap_##Ty##_t *map, const char *key)  \
{  \
    if (!map || !key || strlen(key) > HASHMAP_KEY_SIZE) {  \
        return 0;  \
    }  \
    crc32_t bkt = crc32(key, strlen(key)) % HASHMAP_BUCKETS_NUM;  \
    foreach (list_hashmap_##Ty##_container_t_pair_t, map->buckets[bkt]) {  \
        if (!strcmp(cur->val->key, key)) {  \
            return cur->val->val;  \
        }  \
    }  \
    return 0;  \
}  \
int hashmap_##Ty##_delete(hashmap_##Ty##_t *map, const char *key)  \
{  \
    if (!map || !key || strlen(key) > 63) {  \
        return 1;  \
    }  \
    crc32_t bkt = crc32(key, strlen(key)) % HASHMAP_BUCKETS_NUM;  \
    size_t i = 0;  \
    foreach(list_hashmap_##Ty##_container_t_pair_t, map->buckets[bkt]) {  \
        if (!strcmp(cur->val->key, key)) {  \
            list_hashmap_##Ty##_container_t_delete(map->buckets[bkt], i);  \
            return 0;  \
        }  \
        i++;  \
    }  \
    return 1;  \
}  \
int hashmap_##Ty##_free(hashmap_##Ty##_t *map)  \
{  \
    for (size_t i = 0; i < HASHMAP_BUCKETS_NUM; i++) {  \
        list_hashmap_##Ty##_container_t_free(map->buckets[i]);  \
    }  \
    free(map);  \
}  \
int hashmap_##Ty##_full_free(hashmap_##Ty##_t *map)  \
{  \
    for (size_t i = 0; i < HASHMAP_BUCKETS_NUM; i++) {  \
        list_hashmap_##Ty##_container_t_full_free(map->buckets[i]);  \
    }  \
    free(map);  \
}

#endif