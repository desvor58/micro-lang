#ifndef MICRO_COMMON_H
#define MICRO_COMMON_H

#include <stdio.h>
#include <malloc.h>
#include <inttypes.h>
#include <stddef.h>
#include <string.h>

#include <SCT/common.h>

#if defined(__GNUC__) || defined(__clang__)
# define MICRO_ATTRIBUTE_CONST __attribute__((const))
# define likely(x)   __builtin_expect(!!(x), 1)
# define unlikely(x) __builtin_expect(!!(x), 0)
#elif defined(_MSC_VER)
# define MICRO_ATTRIBUTE_CONST __declspec(noalias)
# define likely(x)   (x)
# define unlikely(x) (x)
#else
# define MICRO_ATTRIBUTE_CONST
# define likely(x)   (x)
# define unlikely(x) (x)
#endif

// size by which error stack will be extended on each overflow
#define MICRO_ERROR_STACK_EXTEND_SIZE 4
// size by which tokens vector will be extended on each overflow
#define MICRO_TOKEN_VEC_EXTEND_SIZE 32
// max size of symbols
#define MICRO_MAX_SYMBOL_SIZE 64

typedef enum {
    MICRO_TYPE_NULL = 0,
    MICRO_TYPE_I8,
    MICRO_TYPE_U8,
    MICRO_TYPE_I16,
    MICRO_TYPE_U16,
    MICRO_TYPE_I32,
    MICRO_TYPE_U32,
    MICRO_TYPE_F32,
    MICRO_TYPE_PTR,
} micro_type_t;

typedef enum {
    MICRO_SIZE_8  = 0,
    MICRO_SIZE_16 = 1,
    MICRO_SIZE_32 = 2,
} micro_size_t;

typedef enum {
    MICRO_ERROR_NONE = 0,

    MICRO_ERROR_IDENT_NAME_TOO_LONG,
    MICRO_ERROR_DIGIT_TOO_LONG,
    MICRO_ERROR_STR_LIT_TOO_LONG,

    MICRO_ERROR_EXPECTED_COMMENT_CLOSE,
    MICRO_ERROR_EXPECTED_STRING_CLOSE,

    MICRO_ERROR_EXPECTED_EXPRESSION,
    MICRO_ERROR_EXPECTED_SEMICOLON,
    MICRO_ERROR_EXPECTED_TYPE_NAME,
    MICRO_ERROR_UNDEFINED_TYPE_NAME,
    MICRO_ERROR_UNEXPECTED_TOKEN,
    MICRO_ERROR_UNEXPECTED_END_KW,

    MICRO_ERROR_EXPECTED_SET_KW,
    MICRO_ERROR_EXPECTED_FUN_KW,
    MICRO_ERROR_EXPECTED_RET_KW,
    MICRO_ERROR_EXPECTED_CALL_KW,
    MICRO_ERROR_EXPECTED_GOTO_KW,
    MICRO_ERROR_EXPECTED_START_KW,
    MICRO_ERROR_EXPECTED_END_KW,
    MICRO_ERROR_EXPECTED_IF_KW,
    MICRO_ERROR_EXPECTED_ELSE_KW,

    MICRO_ERROR_EXPECTED_FUN_NAME,
    MICRO_ERROR_EXPECTED_ARG_TYPE,
    MICRO_ERROR_EXPECTED_ARG_NAME,
    MICRO_ERROR_EXPECTED_RET_TYPE,
    MICRO_ERROR_EXPECTED_VREG_NAME,
    MICRO_ERROR_EXPECTED_RESULT_REG,
    MICRO_ERROR_EXPECTED_LABEL_NAME,
    MICRO_ERROR_EXPECTED_COLON,

    MICRO_ERROR_SET_OUTSIDE_FUNCTION,
    MICRO_ERROR_FUN_INSIDE_FUNCTION,
    MICRO_ERROR_RET_OUTSIDE_FUNCTION,
    MICRO_ERROR_CALL_OUTSIDE_FUNCTION,
    MICRO_ERROR_GOTO_OUTSIDE_FUNCTION,
    MICRO_ERROR_LBL_OUTSIDE_FUNCTION,
    MICRO_ERROR_IF_OUTSIDE_FUNCTION,

    MICRO_ERROR_UNDEFINED_IDENT,
    MICRO_ERROR_UNDEFINED_FUN,
    MICRO_ERROR_EXPECTED_VREG_RESULT,
    MICRO_ERROR_RESULT_TYPE_MISMATCH,
    MICRO_ERROR_TOO_FEW_ARGS,
    MICRO_ERROR_TOO_MANY_ARGS,
    MICRO_ERROR_VREG_TYPE_MISMATCH,
    MICRO_ERROR_EXPR_PARSE,

    MICRO_ERROR_IDENT_NOT_VREG,
    MICRO_ERROR_IDENT_NOT_LBL,
    
    MICRO_ERROR_UNDEFINED_LBL,
} micro_error_type_t;

typedef struct {
    micro_error_type_t err;
    size_t             line_ref;
    size_t             chpos_ref;
} micro_error_t;

typedef struct {
    i32 val;
    u8  bytes[4];
} micro_imm_be_t, micro_addr_be_t;

extern micro_size_t micro_type_to_size[9];

MICRO_ATTRIBUTE_CONST
micro_imm_be_t micro_imm_be_gen(i32 val);

typedef struct {
    i32 val;
    u8  bytes[4];
} micro_imm_le_t, micro_addr_le_t;

MICRO_ATTRIBUTE_CONST
micro_imm_le_t micro_imm_le_gen(i32 val);

extern micro_error_t *micro_err_stk;
extern size_t              micro_err_stk_size;
extern size_t             _micro_err_stk_real_size;

void micro_init();

void micro_push_err(micro_error_t err);

void micro_deinit();

#endif