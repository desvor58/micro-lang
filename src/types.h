#ifndef TYPES_H
#define TYPES_H

#include "config.h"

typedef          char  i8;
typedef          short i16;
typedef          int   i32;
typedef          long  i64;
typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;
typedef unsigned long  u64;

typedef struct
{
    char msg[MAX_ERROR_MESSAGE_SIZE];
    size_t line_ref;
    size_t chpos_ref;
} error_t;

#endif