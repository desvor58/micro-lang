#ifndef MICROC_LEXER_H
#define MICROC_LEXER_H

#include <ctype.h>

#include <micro/common.h>
#include <micro/expr.h>
#include <SCT/vector.h>

typedef enum {
    MC_TOK_NULL = MICRO_EXPR_TOK_NULL,

    MC_TOK_PLUS = MICRO_EXPR_TOK_PLUS,
    MC_TOK_MINUS = MICRO_EXPR_TOK_MINUS,
    MC_TOK_STAR = MICRO_EXPR_TOK_STAR,
    MC_TOK_SLASH = MICRO_EXPR_TOK_SLASH,
    MC_TOK_AMPERSAND = MICRO_EXPR_TOK_AMPERSAND,
    MC_TOK_DOLLAR = MICRO_EXPR_TOK_DOLLAR,
    MC_TOK_HASH = MICRO_EXPR_TOK_HASH,
    MC_TOK_APOSTROPHE = MICRO_EXPR_TOK_APOSTROPHE,
    MC_TOK_TILDE = MICRO_EXPR_TOK_TILDE,
    MC_TOK_EQ = MICRO_EXPR_TOK_EQ,
    MC_TOK_EXCLAMATION = MICRO_EXPR_TOK_EXCLAMATION,
    MC_TOK_GREAT = MICRO_EXPR_TOK_GREAT,
    MC_TOK_LESS = MICRO_EXPR_TOK_LESS,
    MC_TOK_GREAT_OR_EQ = MICRO_EXPR_TOK_GREAT_OR_EQ,
    MC_TOK_LESS_OR_EQ = MICRO_EXPR_TOK_LESS_OR_EQ,

    MC_TOK_IDENT = MICRO_EXPR_TOK_IDENT,
    MC_TOK_LIT_INT = MICRO_EXPR_TOK_LIT_INT,
    MC_TOK_LIT_FLOAT = MICRO_EXPR_TOK_LIT_FLOAT,
    MC_TOK_LIT_STR = MICRO_EXPR_TOK_LIT_STR,

    MC_TOK_DOT,
    MC_TOK_COMA,
    MC_TOK_COLON,
    MC_TOK_SEMICOLON,
    MC_TOK_TYPE_NAME,

    MC_TOK_KW_FUN,
    MC_TOK_KW_SET,
    MC_TOK_KW_IF,
    MC_TOK_KW_ELSE,
    MC_TOK_KW_WHILE,
    MC_TOK_KW_START,
    MC_TOK_KW_END,
    MC_TOK_KW_RET,
    MC_TOK_KW_CALL,
    MC_TOK_KW_GOTO,
} mc_token_type_t;

extern char *mc_token_type2str[];

typedef struct {
    mc_token_type_t type;
    char               val[MICRO_MAX_SYMBOL_SIZE];
    size_t             line_ref;
    size_t             chpos_ref;
} mc_token_t;

#define _mc_single_chlex(ch, tt)  \
    if (text[pos] == (ch)) {  \
        sct_vector_push(toks, &(mc_token_t){  \
            .type = tt,  \
            .val = 0,  \
            .line_ref = line,  \
            .chpos_ref = chpos  \
        });  \
    }

void mc_tokenize(const char *text, size_t text_size, sct_vector_t *toks);

#endif