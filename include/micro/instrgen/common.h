#ifndef MICRO_INSTRGEN_COMMON_H
#define MICRO_INSTRGEN_COMMON_H

#include "../common.h"
#include "../lexer.h"

typedef enum {
    MICRO_MT_NULL = 0,
    MICRO_MT_I8,
    MICRO_MT_U8,
    MICRO_MT_I16,
    MICRO_MT_U16,
    MICRO_MT_I32,
    MICRO_MT_U32,
    MICRO_MT_F32,
    MICRO_MT_PTR,
} micro_instrgen_micro_type;

typedef enum {
    MICRO_SZ_8  = 0,
    MICRO_SZ_16 = 1,
    MICRO_SZ_32 = 2,
} micro_instrgen_size;


typedef enum {
    MICRO_IT_VAR,
    MICRO_IT_SET,
    MICRO_IT_FUN,
    MICRO_IT_RET,
    MICRO_IT_GOTO,
    MICRO_IT_IF,
    MICRO_IT_CALL,
} micro_instruction_type;

typedef struct {
    micro_instrgen_micro_type type;
    char name[MICRO_MAX_SYMBOL_SIZE];
    micro_token_t *val_expr;
} micro_instruction_var_t;

typedef struct {
    char name[MICRO_MAX_SYMBOL_SIZE];
    micro_token_t *val_expr;
} micro_instruction_fun_t;

typedef struct {
    char name[MICRO_MAX_SYMBOL_SIZE];
    micro_token_t *val_expr;
} micro_instruction_set_t;

typedef struct {
    micro_instruction_type type;
    union {

    };
} micro_instruction_t;

typedef struct {
    micro_tok_vec_t        *toks;
    size_t                  toks_pos;
} micro_instrgen_t;

#endif