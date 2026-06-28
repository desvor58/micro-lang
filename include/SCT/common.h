#ifndef SCT_COMMON_H
#define SCT_COMMON_H

#define _CRT_SECURE_NO_WARNINGS

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef int8_t   i8;
typedef uint8_t  u8;
typedef int16_t  i16;
typedef uint16_t u16;
typedef int32_t  i32;
typedef uint32_t u32;
typedef int64_t  i64;
typedef uint64_t u64;
typedef float    f32;
typedef double   f64;

void *amalloc(size_t size);

inline size_t sct_align_up(size_t size)
{
    return (size + 7) & ~((size_t)7);
}

#endif