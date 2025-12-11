#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "types.h"
#include "config.h"

typedef enum
{
    TT_NULL,
    TT_IDENT,
    TT_LIT_INT,
    TT_LIT_FLOAT,
    TT_LIT_STR,
    TT_KW_VAR,
    TT_KW_FUN,
    TT_PLUS,
    TT_MINUS,
    TT_STAR,
    TT_SLASH,
} token_type;

typedef struct
{
    token_type type;
    char       val[MAX_SYMBOL_SIZE];
    size_t     line_ref;
    size_t     chpos_ref;
} token_t;

token_t *toks;
size_t   toks_size;
size_t   __toks_real_size;

void tokenizer_init()
{
    toks = (token_t*)malloc(sizeof(token_t) * TOKEN_BUFFER_EXTEND_SIZE);
    toks_size = 0;
    __toks_real_size = TOKEN_BUFFER_EXTEND_SIZE;
}

void tokenizer_delete()
{
    free(toks);
}

void __toks_size_check(size_t offset)
{
    if (toks_size + offset >= __toks_real_size) {
        token_t *new_toks = (token_t*)malloc(sizeof(token_t) * (__toks_real_size += offset / TOKEN_BUFFER_EXTEND_SIZE + 1));
        memcpy(new_toks, toks, sizeof(token_t) * toks_size);
        free(toks);
        toks = new_toks;
    }
}

#define push_tok(tok) __toks_size_check(1); toks[toks_size++] = tok

void tokenization(const char *text, size_t text_size)
{
    size_t pos = 0;
    size_t line = 0;
    size_t chpos = 1;

    char *buf = (char*)malloc(sizeof(char) * MAX_SYMBOL_SIZE);

    while (pos < text_size) {
        if (text[pos] == '\n') {
            line++;
            chpos = 0;
        } else
        if (isalpha(text[pos]) || text[pos] == '_') {
            size_t i = 0;
            while (isalnum(text[pos]) || text[pos] == '_') {
                buf[i++] = text[pos++];
                chpos++;
            }
            buf[i] = '\0';

            if (!strcmp(buf, "var")) {
                token_t tok = {.type = TT_KW_VAR, .val = 0, .line_ref = line, .chpos_ref = chpos};
                push_tok(tok);
            } else
            if (!strcmp(buf, "fun")) {
                token_t tok = {.type = TT_KW_VAR, .val = 0, .line_ref = line, .chpos_ref = chpos};
                push_tok(tok);
            } else {
                token_t tok = {.type = TT_IDENT, .line_ref = line, .chpos_ref = chpos};
                strcpy(tok.val, buf);
                push_tok(tok);
            }
        } else
        if (isdigit(text[pos])) {
            token_type type = TT_LIT_INT;
            size_t i = 0;
            while (isdigit(text[pos])) {
                buf[i++] = text[pos++];
                chpos++;
                if (text[pos] == '.') {
                    buf[i++] = text[pos++];
                    chpos++;
                    type = TT_LIT_FLOAT;
                }
            }
            buf[i] = '\0';

            token_t tok = {.type = type, .line_ref = line, .chpos_ref = chpos};
            strcpy(tok.val, buf);
            push_tok(tok);
        } else
        if (text[pos] == '"') {
            pos++;
            size_t i = 0;
            while (text[pos] != '"') {
                buf[i++] = text[pos++];
                chpos++;
            }
            buf[i] = '\0';

            token_t tok = {.type = TT_LIT_STR, .line_ref = line, .chpos_ref = chpos};
            strcpy(tok.val, buf);
            push_tok(tok);
        } else
        if (text[pos] == '+') {
            token_t tok = {.type = TT_PLUS, .val = 0, .line_ref = line, .chpos_ref = chpos};
            push_tok(tok);
        } else
        if (text[pos] == '-') {
            token_t tok = {.type = TT_MINUS, .val = 0, .line_ref = line, .chpos_ref = chpos};
            push_tok(tok);
        } else
        if (text[pos] == '*') {
            token_t tok = {.type = TT_STAR, .val = 0, .line_ref = line, .chpos_ref = chpos};
            push_tok(tok);
        } else
        if (text[pos] == '/') {
            token_t tok = {.type = TT_SLASH, .val = 0, .line_ref = line, .chpos_ref = chpos};
            push_tok(tok);
        }
        
        pos++;
        chpos++;
    }
}

#endif