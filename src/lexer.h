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

    MICRO_TT_PLUS,
    MICRO_TT_MINUS,
    MICRO_TT_STAR,
    MICRO_TT_SLASH,
    MICRO_TT_DOT,
    MICRO_TT_COMA,
    MICRO_TT_COLON,
    MICRO_TT_SEMICOLON,
} micro_token_type;

typedef struct
{
    micro_token_type type;
    char       val[MICRO_MAX_SYMBOL_SIZE];
    size_t     line_ref;
    size_t     chpos_ref;
} micro_token_t;

micro_error_t *micro_lexer_err_stk;
size_t         micro_lexer_err_stk_size;
size_t       __micro_lexer_err_stk_real_size;

micro_token_t *micro_toks;
size_t         micro_toks_size;
size_t       __micro_toks_real_size;

void micro_lexer_init()
{
    micro_lexer_err_stk = (micro_error_t*)malloc(sizeof(micro_error_t) * MICRO_ERROR_STACK_EXTEND_SIZE);
    micro_lexer_err_stk_size = 0;
    __micro_lexer_err_stk_real_size = MICRO_ERROR_STACK_EXTEND_SIZE;

    micro_toks = (micro_token_t*)malloc(sizeof(micro_token_t) * MICRO_TOKEN_BUFFER_EXTEND_SIZE);
    micro_toks_size = 0;
    __micro_toks_real_size = MICRO_TOKEN_BUFFER_EXTEND_SIZE;
}

void micro_lexer_delete()
{
    free(micro_toks);
    free(micro_lexer_err_stk);
}

void __micro_toks_size_check(size_t offset)
{
    if (micro_toks_size + offset >= __micro_toks_real_size) {
        micro_token_t *new_toks = (micro_token_t*)malloc(sizeof(micro_token_t) * (__micro_toks_real_size += offset / MICRO_TOKEN_BUFFER_EXTEND_SIZE + 1));
        memcpy(new_toks, micro_toks, sizeof(micro_token_t) * micro_toks_size);
        free(micro_toks);
        micro_toks = new_toks;
    }
}

void __micro_lexer_err_stk_size_check(size_t offset)
{
    if (micro_lexer_err_stk_size + offset >= __micro_lexer_err_stk_real_size) {
        micro_error_t *new_stk = (micro_error_t*)malloc(sizeof(micro_error_t) * (__micro_lexer_err_stk_real_size += offset / MICRO_ERROR_STACK_EXTEND_SIZE + 1));
        memcpy(new_stk, micro_lexer_err_stk, sizeof(micro_error_t) * micro_lexer_err_stk_size);
        free(micro_lexer_err_stk);
        micro_lexer_err_stk = new_stk;
    }
}

#define __micro_push_tok(tok) __micro_toks_size_check(1); micro_toks[micro_toks_size++] = tok
#define __micro_push_err(err) __micro_lexer_err_stk_size_check(1); micro_lexer_err_stk[micro_lexer_err_stk_size++] = err

void micro_lexing(const char *text, size_t text_size)
{
    size_t pos = 0;
    size_t line = 0;
    size_t chpos = 1;

    char *buf = (char*)malloc(sizeof(char) * MICRO_MAX_SYMBOL_SIZE);

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
                    micro_error_t err = {.msg = "Expected closing '\\' character for comment", .line_ref = line, .chpos_ref = chpos};
                    __micro_push_err(err);
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
                micro_token_t tok = {.type = MICRO_TT_KW_VAR, .val = 0, .line_ref = line, .chpos_ref = tok_start_chpos};
                __micro_push_tok(tok);
            } else
            if (!strcmp(buf, "fun")) {
                micro_token_t tok = {.type = MICRO_TT_KW_FUN, .val = 0, .line_ref = line, .chpos_ref = tok_start_chpos};
                __micro_push_tok(tok);
            } else
            if (!strcmp(buf, "set")) {
                micro_token_t tok = {.type = MICRO_TT_KW_SET, .val = 0, .line_ref = line, .chpos_ref = tok_start_chpos};
                __micro_push_tok(tok);
            } else
            if (!strcmp(buf, "if")) {
                micro_token_t tok = {.type = MICRO_TT_KW_IF, .val = 0, .line_ref = line, .chpos_ref = tok_start_chpos};
                __micro_push_tok(tok);
            } else
            if (!strcmp(buf, "else")) {
                micro_token_t tok = {.type = MICRO_TT_KW_ELSE, .val = 0, .line_ref = line, .chpos_ref = tok_start_chpos};
                __micro_push_tok(tok);
            } else
            if (!strcmp(buf, "while")) {
                micro_token_t tok = {.type = MICRO_TT_KW_WHILE, .val = 0, .line_ref = line, .chpos_ref = tok_start_chpos};
                __micro_push_tok(tok);
            } else
            if (!strcmp(buf, "start")) {
                micro_token_t tok = {.type = MICRO_TT_KW_START, .val = 0, .line_ref = line, .chpos_ref = tok_start_chpos};
                __micro_push_tok(tok);
            } else
            if (!strcmp(buf, "end")) {
                micro_token_t tok = {.type = MICRO_TT_KW_END, .val = 0, .line_ref = line, .chpos_ref = tok_start_chpos};
                __micro_push_tok(tok);
            } else
            if (!strcmp(buf, "ret")) {
                micro_token_t tok = {.type = MICRO_TT_KW_RET, .val = 0, .line_ref = line, .chpos_ref = tok_start_chpos};
                __micro_push_tok(tok);
            } else
            if (!strcmp(buf, "call")) {
                micro_token_t tok = {.type = MICRO_TT_KW_CALL, .val = 0, .line_ref = line, .chpos_ref = tok_start_chpos};
                __micro_push_tok(tok);
            } else
            if (!strcmp(buf, "i8")
             || !strcmp(buf, "u8")
             || !strcmp(buf, "i16")
             || !strcmp(buf, "u16")
             || !strcmp(buf, "i32")
             || !strcmp(buf, "u32")
             || !strcmp(buf, "f32")
             || !strcmp(buf, "ptr")) {
                micro_token_t tok = {.type = MICRO_TT_TYPE_NAME, .line_ref = line, .chpos_ref = tok_start_chpos};
                strcpy(tok.val, buf);
                __micro_push_tok(tok);
            } else {
                micro_token_t tok = {.type = MICRO_TT_IDENT, .line_ref = line, .chpos_ref = tok_start_chpos};
                strcpy(tok.val, buf);
                __micro_push_tok(tok);
            }
            pos--;
            chpos--;
        } else
        if (isdigit(text[pos]) || (text[pos] == '-' && isdigit(text[pos + 1]))) {
            size_t tok_start_chpos = chpos;
            micro_token_type type = MICRO_TT_LIT_INT;
            size_t i = 0;
            do {
                buf[i++] = text[pos++];
                chpos++;
                if (text[pos] == '.') {
                    buf[i++] = text[pos++];
                    chpos++;
                    type = MICRO_TT_LIT_FLOAT;
                }
            } while (isdigit(text[pos]) && pos < text_size);
            buf[i] = '\0';

            micro_token_t tok = {.type = type, .line_ref = line, .chpos_ref = tok_start_chpos};
            strcpy(tok.val, buf);
            __micro_push_tok(tok);
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
                    micro_error_t err = {.msg = "Expected closing '\"' character for the string literal", .line_ref = line, .chpos_ref = chpos};
                    __micro_push_err(err);
                    goto err_exit;
                }
            }
            buf[i] = '\0';

            micro_token_t tok = {.type = MICRO_TT_LIT_STR, .line_ref = line, .chpos_ref = tok_start_chpos};
            strcpy(tok.val, buf);
            __micro_push_tok(tok);
        } else
        if (text[pos] == '+') {
            micro_token_t tok = {.type = MICRO_TT_PLUS, .val = 0, .line_ref = line, .chpos_ref = chpos};
            __micro_push_tok(tok);
        } else
        if (text[pos] == '-') {
            micro_token_t tok = {.type = MICRO_TT_MINUS, .val = 0, .line_ref = line, .chpos_ref = chpos};
            __micro_push_tok(tok);
        } else
        if (text[pos] == '*') {
            micro_token_t tok = {.type = MICRO_TT_STAR, .val = 0, .line_ref = line, .chpos_ref = chpos};
            __micro_push_tok(tok);
        } else
        if (text[pos] == '/') {
            micro_token_t tok = {.type = MICRO_TT_SLASH, .val = 0, .line_ref = line, .chpos_ref = chpos};
            __micro_push_tok(tok);
        } else
        if (text[pos] == '.') {
            micro_token_t tok = {.type = MICRO_TT_DOT, .val = 0, .line_ref = line, .chpos_ref = chpos};
            __micro_push_tok(tok);
        } else
        if (text[pos] == ',') {
            micro_token_t tok = {.type = MICRO_TT_COMA, .val = 0, .line_ref = line, .chpos_ref = chpos};
            __micro_push_tok(tok);
        } else
        if (text[pos] == ':') {
            micro_token_t tok = {.type = MICRO_TT_COLON, .val = 0, .line_ref = line, .chpos_ref = chpos};
            __micro_push_tok(tok);
        } else
        if (text[pos] == ';') {
            micro_token_t tok = {.type = MICRO_TT_SEMICOLON, .val = 0, .line_ref = line, .chpos_ref = chpos};
            __micro_push_tok(tok);
        }
        
        pos++;
        chpos++;
    }
err_exit:
}

#undef __micro_push_tok
#undef __micro_push_err

#endif