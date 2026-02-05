#ifndef MICRO_LEXER_H
#define MICRO_LEXER_H

#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "types.h"
#include "config.h"

typedef enum
{
    MICRO_TT_NULL,

    MICRO_TT_PLUS,
    MICRO_TT_MINUS,
    MICRO_TT_STAR,
    MICRO_TT_SLASH,
    MICRO_TT_DOT,
    MICRO_TT_COMA,
    MICRO_TT_COLON,
    MICRO_TT_SEMICOLON,
    MICRO_TT_AMPERRSAND,
    MICRO_TT_DOLAR,
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

extern char *__micro_token_type2str[];

typedef struct
{
    micro_token_type type;
    char       val[MICRO_MAX_SYMBOL_SIZE];
    size_t     line_ref;
    size_t     chpos_ref;
} micro_token_t;

extern micro_error_t *micro_lexer_err_stk;
extern size_t         micro_lexer_err_stk_size;
extern size_t       __micro_lexer_err_stk_real_size;

extern micro_token_t *micro_toks;
extern size_t         micro_toks_size;
extern size_t       __micro_toks_real_size;

void micro_lexer_init();

void micro_lexer_delete();

void __micro_toks_size_check(size_t offset);

void __micro_lexer_err_stk_size_check(size_t offset);

#define __micro_lexer_push_tok(tok) __micro_toks_size_check(1); micro_toks[micro_toks_size++] = tok
#define __micro_lexer_push_err(err) __micro_lexer_err_stk_size_check(1); micro_lexer_err_stk[micro_lexer_err_stk_size++] = err

void micro_lexing(const char *text, size_t text_size);

#endif