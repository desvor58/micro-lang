#ifndef LEXER_H
#define LEXER_H

#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "types.h"
#include "config.h"

typedef enum
{
    TT_NULL,
    TT_TYPE_NAME,
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
    TT_SEMICOLON,
} token_type;

typedef struct
{
    token_type type;
    char       val[MAX_SYMBOL_SIZE];
    size_t     line_ref;
    size_t     chpos_ref;
} token_t;

error_t *lexer_err_stk;
size_t   lexer_err_stk_size;
size_t __lexer_err_stk_real_size;

token_t *toks;
size_t   toks_size;
size_t   __toks_real_size;

void lexer_init()
{
    lexer_err_stk = (error_t*)malloc(sizeof(error_t) * ERROR_STACK_EXTEND_SIZE);
    lexer_err_stk_size = 0;
    __lexer_err_stk_real_size = ERROR_STACK_EXTEND_SIZE;

    toks = (token_t*)malloc(sizeof(token_t) * TOKEN_BUFFER_EXTEND_SIZE);
    toks_size = 0;
    __toks_real_size = TOKEN_BUFFER_EXTEND_SIZE;
}

void lexer_delete()
{
    free(toks);
    free(lexer_err_stk);
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

void __lexer_err_stk_size_check(size_t offset)
{
    if (lexer_err_stk_size + offset >= __lexer_err_stk_real_size) {
        error_t *new_stk = (error_t*)malloc(sizeof(error_t) * (__lexer_err_stk_real_size += offset / ERROR_STACK_EXTEND_SIZE + 1));
        memcpy(new_stk, lexer_err_stk, sizeof(error_t) * lexer_err_stk_size);
        free(lexer_err_stk);
        lexer_err_stk = new_stk;
    }
}

#define push_tok(tok) __toks_size_check(1); toks[toks_size++] = tok
#define push_err(err) __lexer_err_stk_size_check(1); lexer_err_stk[lexer_err_stk_size++] = err

void lexing(const char *text, size_t text_size)
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
        if (text[pos] == '\\') {
            while (text[++pos] != '\\') {
                if (text[pos] == '\n') {
                    line++;
                    chpos = 0;
                }
                pos++;
                chpos++;
                if (pos >= text_size) {
                    error_t err = {.msg = "Expected closing '\\' character for comment", .line_ref = line, .chpos_ref = chpos};
                    push_err(err);
                    goto err_exit;
                }
            }
        }
        if (isalpha(text[pos]) || text[pos] == '_') {
            size_t tok_start_chpos = chpos;
            size_t i = 0;
            while ((isalnum(text[pos]) || text[pos] == '_') && pos < text_size) {
                buf[i++] = text[pos++];
                chpos++;
            }
            buf[i] = '\0';

            if (!strcmp(buf, "var")) {
                token_t tok = {.type = TT_KW_VAR, .val = 0, .line_ref = line, .chpos_ref = tok_start_chpos};
                push_tok(tok);
            } else
            if (!strcmp(buf, "fun")) {
                token_t tok = {.type = TT_KW_FUN, .val = 0, .line_ref = line, .chpos_ref = tok_start_chpos};
                push_tok(tok);
            } else
            if (!strcmp(buf, "i8")
             || !strcmp(buf, "u8")
             || !strcmp(buf, "i16")
             || !strcmp(buf, "u16")
             || !strcmp(buf, "i32")
             || !strcmp(buf, "u32")
             || !strcmp(buf, "f32")
             || !strcmp(buf, "ptr")) {
                token_t tok = {.type = TT_TYPE_NAME, .line_ref = line, .chpos_ref = tok_start_chpos};
                strcpy(tok.val, buf);
                push_tok(tok);
            } else {
                token_t tok = {.type = TT_IDENT, .line_ref = line, .chpos_ref = tok_start_chpos};
                strcpy(tok.val, buf);
                push_tok(tok);
            }
            pos--;
            chpos--;
        } else
        if (isdigit(text[pos]) || (text[pos] == '-' && isdigit(text[pos + 1]))) {
            size_t tok_start_chpos = chpos;
            token_type type = TT_LIT_INT;
            size_t i = 0;
            do {
                buf[i++] = text[pos++];
                chpos++;
                if (text[pos] == '.') {
                    buf[i++] = text[pos++];
                    chpos++;
                    type = TT_LIT_FLOAT;
                }
            } while (isdigit(text[pos]) && pos < text_size);
            buf[i] = '\0';

            token_t tok = {.type = type, .line_ref = line, .chpos_ref = tok_start_chpos};
            strcpy(tok.val, buf);
            push_tok(tok);
            pos--;
        } else
        if (text[pos] == '"') {
            size_t tok_start_chpos = chpos;
            pos++;
            size_t i = 0;
            while (text[pos] != '"') {
                buf[i++] = text[pos++];
                chpos++;
                
                if (pos >= text_size) {
                    error_t err = {.msg = "Expected closing '\"' character for the string literal", .line_ref = line, .chpos_ref = chpos};
                    push_err(err);
                    goto err_exit;
                }
            }
            buf[i] = '\0';

            token_t tok = {.type = TT_LIT_STR, .line_ref = line, .chpos_ref = tok_start_chpos};
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
        } else
        if (text[pos] == ';') {
            token_t tok = {.type = TT_SEMICOLON, .val = 0, .line_ref = line, .chpos_ref = chpos};
            push_tok(tok);
        }
        
        pos++;
        chpos++;
    }
err_exit:
}

#endif