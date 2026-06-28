#ifndef MICRO_LEXER_H
#define MICRO_LEXER_H

#include <ctype.h>

#include <micro/common.h>
#include <SCT/vector.h>

typedef enum {
    MICRO_TOK_NULL,

    MICRO_TOK_PLUS,
    MICRO_TOK_MINUS,
    MICRO_TOK_STAR,
    MICRO_TOK_SLASH,
    MICRO_TOK_DOT,
    MICRO_TOK_COMA,
    MICRO_TOK_COLON,
    MICRO_TOK_SEMICOLON,
    MICRO_TOK_AMPERSAND,
    MICRO_TOK_DOLLAR,
    MICRO_TOK_HASH,
    MICRO_TOK_APOSTROPHE,
    MICRO_TOK_TILDE,
    MICRO_TOK_EQ,
    MICRO_TOK_EXCLAMATION,
    MICRO_TOK_NOT_EQ,
    MICRO_TOK_GREAT,
    MICRO_TOK_LESS,
    MICRO_TOK_GREAT_OR_EQ,
    MICRO_TOK_LESS_OR_EQ,

    MICRO_TOK_TYPE_NAME,
    MICRO_TOK_IDENT,

    MICRO_TOK_LIT_INT,
    MICRO_TOK_LIT_FLOAT,
    MICRO_TOK_LIT_STR,

    MICRO_TOK_KW_FUN,
    MICRO_TOK_KW_SET,
    MICRO_TOK_KW_IF,
    MICRO_TOK_KW_ELSE,
    MICRO_TOK_KW_WHILE,
    MICRO_TOK_KW_START,
    MICRO_TOK_KW_END,
    MICRO_TOK_KW_RET,
    MICRO_TOK_KW_CALL,
    MICRO_TOK_KW_GOTO,
} micro_token_type_t;

extern char *micro_token_type2str[];

typedef struct {
    micro_token_type_t type;
    char             val[MICRO_MAX_SYMBOL_SIZE];
    size_t           line_ref;
    size_t           chpos_ref;
} micro_token_t;

#define _micro_single_chlex(ch, tt)  \
    if (text[pos] == (ch)) {  \
        sct_vector_push(toks, &(micro_token_t){  \
            .type = tt,  \
            .val = 0,  \
            .line_ref = line,  \
            .chpos_ref = chpos  \
        });  \
    }

void micro_tokenize(const char *text, size_t text_size, sct_vector_t *toks);

#endif