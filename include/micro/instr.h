#ifndef MICRO_INSTR_H
#define MICRO_INSTR_H

#include <SCT/vector.h>

#include <micro/common.h>
#include <micro/expr.h>

typedef enum {
    MICRO_INSTR_SET,
    MICRO_INSTR_DRSET,  // DeRef&SET
    MICRO_INSTR_FUN,
    MICRO_INSTR_RET,
    MICRO_INSTR_CALL,
    MICRO_INSTR_LBL,
    MICRO_INSTR_GOTO,
    MICRO_INSTR_IF,
} micro_instruction_type_t;

typedef struct {
    micro_type_t      type;
    char              reg_name[MICRO_MAX_SYMBOL_SIZE];
    micro_expr_tok_t *val_expr;
} micro_instruction_set_t;

typedef struct {
    micro_type_t      type;
    char              reg_name[MICRO_MAX_SYMBOL_SIZE];
    micro_expr_tok_t *val_expr;
} micro_instruction_drset_t;

typedef struct {
    micro_type_t type;
    char         name[MICRO_MAX_SYMBOL_SIZE];
} micro_instruction_fun_arg_t;

typedef struct {
    char         name[MICRO_MAX_SYMBOL_SIZE];
    sct_vector_t args;
    micro_type_t ret_type;
    sct_vector_t body;
} micro_instruction_fun_t;

typedef struct {
    micro_expr_tok_t *val_expr;
} micro_instruction_ret_t;

typedef struct {
    char         ret_reg_name[MICRO_MAX_SYMBOL_SIZE];
    char         fun_name[MICRO_MAX_SYMBOL_SIZE];
    sct_vector_t arg_exprs;
} micro_instruction_call_t;

typedef struct {
    char name[MICRO_MAX_SYMBOL_SIZE];
} micro_instruction_lbl_t;

typedef struct {
    char lbl[MICRO_MAX_SYMBOL_SIZE];
} micro_instruction_goto_t;

typedef struct {
    micro_expr_tok_t *cond_expr;
    char              lbl_name[MICRO_MAX_SYMBOL_SIZE];
} micro_instruction_if_t;

typedef struct {
    micro_instruction_type_t type;
    micro_expr_tok_t        *start_tok;
    union {
        micro_instruction_set_t   set;
        micro_instruction_drset_t drset;
        micro_instruction_fun_t   fun;
        micro_instruction_ret_t   ret;
        micro_instruction_call_t  call;
        micro_instruction_lbl_t   lbl;
        micro_instruction_goto_t  goto_lbl;
        micro_instruction_if_t    if_goto;
    };
} micro_instruction_t;

size_t micro_make_expr(sct_vector_t *expr_dst, const char *str_expr);

int micro_instr_gen_set(sct_vector_t *instrs, micro_type_t type, const char *name, sct_vector_t *expr);

#endif