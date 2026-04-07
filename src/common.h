#ifndef MICRO_COMMON_H
#define MICRO_COMMON_H

#include <malloc.h>
#include <inttypes.h>
#include <stddef.h>
#include <string.h>

// size by which error stack will be extended on each overflow
#define MICRO_ERROR_STACK_EXTEND_SIZE 4
// size by which tokens vector will be extended on each overflow
#define MICRO_TOKEN_VEC_EXTEND_SIZE 32
// max size of symbols
#define MICRO_MAX_SYMBOL_SIZE 128
// max errors and warnings message size
#define MICRO_MAX_ERROR_MESSAGE_SIZE 256
// size of buffer for reading input file
#define MICRO_MAX_INPUT_CODE_SIZE 48 * 1024
// size of buffer for writing instructions before __micro_asm_instructions_put()
#define MICRO_INSTRUCTION_STACK_SIZE 16

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

extern micro_error_t *micro_err_stk;
extern size_t         micro_err_stk_size;
extern size_t       __micro_err_stk_real_size;

void micro_init();

void micro_push_err(micro_error_t err);

void micro_deinit();

#endif