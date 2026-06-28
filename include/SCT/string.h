#ifndef SCT_STRING_H
#define SCT_STRING_H

#include <stdio.h>
#include <stdarg.h>
#include "common.h"

#define SCT_STRING_ALLOC_SIZE 128

typedef struct {
    char  *cstr;
    size_t size;
    size_t cap;
} sct_string_t;

void sct_string_init(sct_string_t *str);

void sct_string_deinit(sct_string_t *str);

void sct_string_push(sct_string_t *str, char c);

void sct_string_cat(sct_string_t *str, char *fmt, ...);

void sct_string_insert(sct_string_t *str, size_t index, char *fmt, ...);

void sct_string_replace(sct_string_t *str, size_t start, size_t end, char *fmt, ...);

#endif