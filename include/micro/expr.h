#ifndef MICRO_EXPR_H
#define MICRO_EXPR_H

#include <stddef.h>

#include <micro/common.h>

typedef enum {
    MICRO_EXPR_TOK_NULL,

    MICRO_EXPR_TOK_PLUS,
    MICRO_EXPR_TOK_MINUS,
    MICRO_EXPR_TOK_STAR,
    MICRO_EXPR_TOK_SLASH,
    MICRO_EXPR_TOK_AMPERSAND,
    MICRO_EXPR_TOK_DOLLAR,
    MICRO_EXPR_TOK_HASH,
    MICRO_EXPR_TOK_APOSTROPHE,
    MICRO_EXPR_TOK_TILDE,
    MICRO_EXPR_TOK_EQ,
    MICRO_EXPR_TOK_EXCLAMATION,
    MICRO_EXPR_TOK_GREAT,
    MICRO_EXPR_TOK_LESS,
    MICRO_EXPR_TOK_GREAT_OR_EQ,
    MICRO_EXPR_TOK_LESS_OR_EQ,

    MICRO_EXPR_TOK_IDENT,
    MICRO_EXPR_TOK_LIT_INT,
    MICRO_EXPR_TOK_LIT_FLOAT,
    MICRO_EXPR_TOK_LIT_STR,
} micro_expr_tok_type_t;

typedef struct {
    micro_expr_tok_type_t type;
    char                  val[MICRO_MAX_SYMBOL_SIZE];
    size_t                line_ref;
    size_t                chpos_ref;
} micro_expr_tok_t;

#define _micro_expr_is_op(tok_type) ((int)(tok_type) == (int)MICRO_EXPR_TOK_PLUS ||  \
                                     (int)(tok_type) == (int)MICRO_EXPR_TOK_MINUS ||  \
                                     (int)(tok_type) == (int)MICRO_EXPR_TOK_STAR ||  \
                                     (int)(tok_type) == (int)MICRO_EXPR_TOK_SLASH ||  \
                                     (int)(tok_type) == (int)MICRO_EXPR_TOK_AMPERSAND ||  \
                                     (int)(tok_type) == (int)MICRO_EXPR_TOK_DOLLAR ||  \
                                     (int)(tok_type) == (int)MICRO_EXPR_TOK_HASH ||  \
                                     (int)(tok_type) == (int)MICRO_EXPR_TOK_APOSTROPHE ||  \
                                     (int)(tok_type) == (int)MICRO_EXPR_TOK_TILDE ||  \
                                     (int)(tok_type) == (int)MICRO_EXPR_TOK_EQ ||  \
                                     (int)(tok_type) == (int)MICRO_EXPR_TOK_EXCLAMATION ||  \
                                     (int)(tok_type) == (int)MICRO_EXPR_TOK_GREAT ||  \
                                     (int)(tok_type) == (int)MICRO_EXPR_TOK_LESS ||  \
                                     (int)(tok_type) == (int)MICRO_EXPR_TOK_GREAT_OR_EQ ||  \
                                     (int)(tok_type) == (int)MICRO_EXPR_TOK_LESS_OR_EQ)

#define _micro_expr_is_lit(tok_type) ((int)(tok_type) == (int)MICRO_EXPR_TOK_LIT_FLOAT ||  \
                                      (int)(tok_type) == (int)MICRO_EXPR_TOK_LIT_INT ||  \
                                      (int)(tok_type) == (int)MICRO_EXPR_TOK_LIT_STR)

#define _micro_expr_is_expr_start(tok_type) (_micro_expr_is_op(tok_type) || _micro_expr_is_lit(tok_type) || (int)(tok_type) == (int)MICRO_EXPR_TOK_IDENT)

#endif