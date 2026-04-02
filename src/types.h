#ifndef MICRO_TYPES_H
#define MICRO_TYPES_H

#include <inttypes.h>
#include "config.h"
#include "stddef.h"

typedef int8_t   i8;
typedef uint8_t  u8;
typedef int16_t  i16;
typedef uint16_t u16;
typedef int32_t  i32;
typedef uint32_t u32;

typedef struct {
    char msg[MICRO_MAX_ERROR_MESSAGE_SIZE];
    size_t line_ref;
    size_t chpos_ref;
} micro_error_t;

typedef struct {
    i32 val;
    u8  bytes[4];
} micro_imm_be_t, micro_addr_be_t;

micro_imm_be_t micro_imm_be_gen(i32 val);

typedef struct {
    i32 val;
    u8  bytes[4];
} micro_imm_le_t, micro_addr_le_t;

micro_imm_le_t micro_imm_le_gen(i32 val);

#endif