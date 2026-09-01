#ifndef MICRO_CODEGEN386_EXPR_OPS_H
#define MICRO_CODEGEN386_EXPR_OPS_H

#include "expr_ops/common.h"
#include "expr_ops/plus.h"
#include "expr_ops/minus.h"

#include "expr_ops/cond_cmp.h"

typedef struct {
    int (*handler)(micro_codegen_t *codegen, micro_codegen386_storage_t dst, micro_expr_tok_t *start);
    int is_cond;
} op_info_t;

op_info_t op_tbl[] = {
    [MICRO_EXPR_TOK_PLUS]  = { .handler = op_plus_handler, .is_cond = 0 },
    [MICRO_EXPR_TOK_MINUS] = { .handler = op_minus_handler, .is_cond = 0 },
};
op_info_t cond_op_tbl[] = {
    [MICRO_EXPR_TOK_PLUS]  = { .handler = op_plus_handler, .is_cond = 0 },
    [MICRO_EXPR_TOK_MINUS] = { .handler = op_minus_handler, .is_cond = 0 },
    [MICRO_EXPR_TOK_EQ]    = { .handler = cond_op_cmp_handler, .is_cond = 1 },
    [MICRO_EXPR_TOK_LESS]  = { .handler = cond_op_cmp_handler, .is_cond = 1 },
    [MICRO_EXPR_TOK_GREAT] = { .handler = cond_op_cmp_handler, .is_cond = 1 },
};

#endif