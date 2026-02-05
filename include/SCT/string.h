#ifndef STRING_H

#include <stdio.h>
#include <malloc.h>
#include <stdarg.h>
#include <string.h>

typedef struct
{
    char  *str;
    size_t size;
    size_t aloc_size;
} sct_string_t;

sct_string_t *sct_string_create();

void __sct_string_realoc(sct_string_t *str);

void sct_string_push_back(sct_string_t *str, char c);

void sct_string_cat(sct_string_t *str, char *fmt, ...);

void sct_string_insert(sct_string_t *str, size_t index, char *fmt, ...);

void sct_string_replace(sct_string_t *str, size_t start, size_t end, char *fmt, ...);

void sct_string_free(sct_string_t *str);

#endif