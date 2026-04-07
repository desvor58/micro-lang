#ifndef MICRO_LEXER_H
#define MICRO_LEXER_H

#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "common.h"

typedef enum {
    MICRO_TT_NULL,

    MICRO_TT_PLUS,
    MICRO_TT_MINUS,
    MICRO_TT_STAR,
    MICRO_TT_SLASH,
    MICRO_TT_DOT,
    MICRO_TT_COMA,
    MICRO_TT_COLON,
    MICRO_TT_SEMICOLON,
    MICRO_TT_AMPERSAND,
    MICRO_TT_DOLLAR,
    MICRO_TT_HASH,
    MICRO_TT_APOSTROPHE,
    MICRO_TT_TILDE,

    MICRO_TT_TYPE_NAME,
    MICRO_TT_IDENT,

    MICRO_TT_LIT_INT,
    MICRO_TT_LIT_FLOAT,
    MICRO_TT_LIT_STR,

    MICRO_TT_KW_VAR,
    MICRO_TT_KW_FUN,
    MICRO_TT_KW_SET,
    MICRO_TT_KW_IF,
    MICRO_TT_KW_ELSE,
    MICRO_TT_KW_WHILE,
    MICRO_TT_KW_START,
    MICRO_TT_KW_END,
    MICRO_TT_KW_RET,
    MICRO_TT_KW_CALL,
    MICRO_TT_KW_GOTO,
} micro_token_type;

extern char *micro_token_type2str[];

typedef struct {
    micro_token_type type;
    char       val[MICRO_MAX_SYMBOL_SIZE];
    size_t     line_ref;
    size_t     chpos_ref;
} micro_token_t;

typedef struct {
    micro_token_t *toks;
    size_t         size;
    size_t         real_size;
} micro_tok_vec_t;

void micro_create_tok_vec(micro_tok_vec_t *vec);
void micro_push_tok(micro_tok_vec_t *vec, micro_token_t tok);
void micro_free_tok_vec(micro_tok_vec_t *vec);

#define __micro_single_chlex(ch, tt)  \
    if (text[pos] == (ch)) {  \
        micro_token_t tok = {  \
            .type = tt,  \
            .val = 0,  \
            .line_ref = line,  \
            .chpos_ref = chpos  \
        };  \
        micro_push_tok(toks, tok);  \
    }

void micro_tokenize(const char *text, size_t text_size, micro_tok_vec_t *toks);

#endif