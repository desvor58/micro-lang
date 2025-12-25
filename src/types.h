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

/// @brief put to the buffer 16 bits immediante value from size_t in leatle endiane
/// @param buf buf with size 2 to put the immVal
/// @param imm size_t value for put
void gen16imm_le(u8 *buf, size_t imm)
{
    buf[0] = (u8)imm;
    buf[1] = (u8)(imm >> 8);
}

/// @brief put to the buffer 32 bits immediante value from size_t in leatle endiane
/// @param buf buf with size 4 to put the immVal
/// @param imm size_t value for put
void gen32imm_le(u8 *buf, size_t imm)
{
    buf[0] = (u8)imm;
    buf[1] = (u8)(imm >> 8);
    buf[2] = (u8)(imm >> 16);
    buf[3] = (u8)(imm >> 24);
}

#endif