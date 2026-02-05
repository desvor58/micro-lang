#ifndef MICRO_TYPES_H
#define MICRO_TYPES_H

#include "config.h"
#include "stddef.h"

typedef          char  i8;
typedef unsigned char  u8;

typedef long long offset_t;

typedef struct
{
    char msg[MICRO_MAX_ERROR_MESSAGE_SIZE];
    size_t line_ref;
    size_t chpos_ref;
} micro_error_t;

/// @brief put to the buffer 16 bits immediante value from size_t in leatle endiane
/// @param buf buf with size 2 to put the immVal
/// @param imm size_t value for put
void micro_gen16imm_le(u8 *buf, offset_t imm);

/// @brief put to the buffer 32 bits immediante value from size_t in leatle endiane
/// @param buf buf with size 4 to put the immVal
/// @param imm size_t value for put
void micro_gen32imm_le(u8 *buf, offset_t imm);

/// @brief put to the buffer 16 bits immediante value from size_t in big endiane
/// @param buf buf with size 2 to put the immVal
/// @param imm size_t value for put
void micro_gen16imm_be(u8 *buf, offset_t imm);

/// @brief put to the buffer 32 bits immediante value from size_t in big endiane
/// @param buf buf with size 4 to put the immVal
/// @param imm size_t value for put
void micro_gen32imm_be(u8 *buf, offset_t imm);

#endif