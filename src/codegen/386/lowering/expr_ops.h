#ifndef MICRO_CODEGEN386_EXPR_OPS_H
#define MICRO_CODEGEN386_EXPR_OPS_H

#include "expr_ops/common.h"
#include "expr_ops/plus.h"
#include "expr_ops/minus.h"
#include "expr_ops/star.h"
#include "expr_ops/dollar.h"
#include "expr_ops/slash.h"
#include "expr_ops/ampersand.h"
#include "expr_ops/pipe.h"
#include "expr_ops/caret.h"
#include "expr_ops/logical.h"
#include "expr_ops/hash.h"
#include "expr_ops/tilde.h"
#include "expr_ops/apostrophe.h"

#include "expr_ops/cond_cmp.h"

typedef struct {
    expr_info_t (*handler)(micro_codegen_t *codegen, micro_codegen386_storage_t dst, micro_expr_tok_t *start);
    int is_cond;
} op_info_t;

static op_info_t op_tbl[] = {
    [MICRO_EXPR_TOK_PLUS]        = { .handler = op_plus_handler,       .is_cond = 0 },
    [MICRO_EXPR_TOK_MINUS]       = { .handler = op_minus_handler,      .is_cond = 0 },
    [MICRO_EXPR_TOK_STAR]        = { .handler = op_star_handler,       .is_cond = 0 },
    [MICRO_EXPR_TOK_SLASH]       = { .handler = op_slash_handler,      .is_cond = 0 },
    [MICRO_EXPR_TOK_AMPERSAND]   = { .handler = op_ampersand_handler,  .is_cond = 0 },
    [MICRO_EXPR_TOK_PIPE]        = { .handler = op_pipe_handler,       .is_cond = 0 },
    [MICRO_EXPR_TOK_CARET]       = { .handler = op_caret_handler,      .is_cond = 0 },
    [MICRO_EXPR_TOK_ANDAND]      = { .handler = op_andand_handler,     .is_cond = 0 },
    [MICRO_EXPR_TOK_OROR]        = { .handler = op_oror_handler,       .is_cond = 0 },
    [MICRO_EXPR_TOK_DOLLAR]      = { .handler = op_dollar_handler,     .is_cond = 0 },
    [MICRO_EXPR_TOK_HASH]        = { .handler = op_hash_handler,       .is_cond = 0 },
    [MICRO_EXPR_TOK_TILDE]       = { .handler = op_tilde_handler,      .is_cond = 0 },
    [MICRO_EXPR_TOK_APOSTROPHE]  = { .handler = op_apostrophe_handler, .is_cond = 0 },
    [MICRO_EXPR_TOK_EQ]          = { .handler = cond_op_cmp_handler,   .is_cond = 1 },
    [MICRO_EXPR_TOK_LESS]        = { .handler = cond_op_cmp_handler,   .is_cond = 1 },
    [MICRO_EXPR_TOK_GREAT]       = { .handler = cond_op_cmp_handler,   .is_cond = 1 },
    [MICRO_EXPR_TOK_LESS_OR_EQ]  = { .handler = cond_op_cmp_handler,   .is_cond = 1 },
    [MICRO_EXPR_TOK_GREAT_OR_EQ] = { .handler = cond_op_cmp_handler,   .is_cond = 1 },
};

#endif