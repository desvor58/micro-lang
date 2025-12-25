#ifndef STRING_H

#include <malloc.h>
#include <stdarg.h>

typedef struct
{
    char  *str;
    size_t size;
    size_t aloc_size;
} string_t;

string_t *string_create()
{
    string_t *str = malloc(sizeof(string_t));
    str->str = malloc(sizeof(char) * 512);
    str->str[0] = '\0';
    str->size = 0;
    str->aloc_size = 512;
    return str;
}

void __string_realoc(string_t *str)
{
    char *new_str = malloc(sizeof(char) * (str->aloc_size += 512));
    for (size_t i = 0; i < str->size; i++) {
        new_str[i] = str->str[i]; 
    }
    free(str->str);
    str->str = new_str;
}

void string_push_back(string_t *str, char c)
{
    if (str->size + 2 >= str->aloc_size) {
        __string_realoc(str);
    }
    str->str[str->size++] = c;
    str->str[str->size]   = '\0';
}

void string_cat(string_t *str, char *fmt, ...)
{
    char *buf = malloc(4*1024);
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, 4*1024, fmt, args);
    va_end(args);

    char *new_str = malloc(sizeof(char) * (str->size + (str->aloc_size += ((strlen(buf) + 1) / 512 + 1)*512)));
    size_t i = 0;
    while (i < str->size) {
        new_str[i] = str->str[i]; 
        i++;
    }
    size_t j = 0;
    while (j < strlen(buf)) {
        new_str[i] = buf[j];
        str->size++;
        i++;
        j++;
    }
    free(str->str);
    str->str = new_str;
    free(buf);
}

void string_insert(string_t *str, size_t index, char *fmt, ...)
{
    char *buf = malloc(4*1024);
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, 4*1024, fmt, args);
    va_end(args);

    char *new_str = malloc(sizeof(char) * (str->aloc_size += ((strlen(buf) + 1) / 512 + 1)*512));
    size_t i = 0;
    while (i < index) {
        new_str[i] = str->str[i]; 
        i++;
    }
    size_t j = 0;
    while (j < strlen(buf)) {
        new_str[i] = buf[j];
        str->size++;
        i++;
        j++;
    }
    size_t k = index;
    while (k < str->size) {
        new_str[i] = str->str[k];
        i++;
        k++;
    }
    free(str->str);
    str->str = new_str;
    free(buf);
}

void string_replace(string_t *str, size_t start, size_t end, char *fmt, ...)
{
    char *buf = malloc(4*1024);
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, 4*1024, fmt, args);
    va_end(args);
    size_t size_acc = str->size;

    char *new_str = malloc(sizeof(char) * (str->aloc_size += ((strlen(buf) + 1) / 512 + 1)*512));
    str->size = 0;
    size_t i = 0;
    while (i < start) {
        new_str[i] = str->str[i]; 
        str->size++;
        i++;
    }
    size_t j = 0;
    while (j < strlen(buf)) {
        new_str[i] = buf[j];
        str->size++;
        i++;
        j++;
    }
    size_t k = end;
    while (k < size_acc) {
        new_str[i] = str->str[k];
        str->size++;
        i++;
        k++;
    }
    free(str->str);
    str->str = new_str;
    free(buf);
}

void string_free(string_t *str)
{
    free(str->str);
    free(str);
}

#endif