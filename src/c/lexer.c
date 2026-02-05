#include "../lexer.h"
char *__micro_token_type2str[] = {
    [MICRO_TT_NULL]       = "null",
    [MICRO_TT_PLUS]       = "plus",
    [MICRO_TT_MINUS]      = "minus",
    [MICRO_TT_STAR]       = "star",
    [MICRO_TT_SLASH]      = "slash",
    [MICRO_TT_DOT]        = "dot",
    [MICRO_TT_COMA]       = "coma",
    [MICRO_TT_COLON]      = "colon",
    [MICRO_TT_SEMICOLON]  = "semicolon",
    [MICRO_TT_AMPERRSAND] = "ampersand",
    [MICRO_TT_DOLAR]      = "dolar",
    [MICRO_TT_HASH]       = "hash",
    [MICRO_TT_APOSTROPHE] = "apostrophe",
    [MICRO_TT_TILDE]      = "tilde",

    [MICRO_TT_TYPE_NAME]  = "type name",
    [MICRO_TT_IDENT]      = "ident",
    [MICRO_TT_LIT_INT]    = "lit int",
    [MICRO_TT_LIT_FLOAT]  = "lit float",
    [MICRO_TT_LIT_STR]    = "lit str",

    [MICRO_TT_KW_VAR]     = "var",
    [MICRO_TT_KW_FUN]     = "fun",
    [MICRO_TT_KW_SET]     = "set",
    [MICRO_TT_KW_IF]      = "if",
    [MICRO_TT_KW_ELSE]    = "else",
    [MICRO_TT_KW_WHILE]   = "while",
    [MICRO_TT_KW_START]   = "start",
    [MICRO_TT_KW_END]     = "end",
    [MICRO_TT_KW_RET]     = "ret",
    [MICRO_TT_KW_CALL]    = "call",
    [MICRO_TT_KW_GOTO]    = "goto",
};

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

void micro_lexing(const char *text, size_t text_size)
{
    size_t pos = 0;
    size_t line = 1;
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
                chpos++;
                if (pos >= text_size) {
                    micro_error_t err = {.msg = "Expected closing '\\' character for comment", .line_ref = line, .chpos_ref = chpos};
                    __micro_lexer_push_err(err);
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
                __micro_lexer_push_tok(tok);
            } else
            if (!strcmp(buf, "fun")) {
                micro_token_t tok = {.type = MICRO_TT_KW_FUN, .val = 0, .line_ref = line, .chpos_ref = tok_start_chpos};
                __micro_lexer_push_tok(tok);
            } else
            if (!strcmp(buf, "set")) {
                micro_token_t tok = {.type = MICRO_TT_KW_SET, .val = 0, .line_ref = line, .chpos_ref = tok_start_chpos};
                __micro_lexer_push_tok(tok);
            } else
            if (!strcmp(buf, "if")) {
                micro_token_t tok = {.type = MICRO_TT_KW_IF, .val = 0, .line_ref = line, .chpos_ref = tok_start_chpos};
                __micro_lexer_push_tok(tok);
            } else
            if (!strcmp(buf, "else")) {
                micro_token_t tok = {.type = MICRO_TT_KW_ELSE, .val = 0, .line_ref = line, .chpos_ref = tok_start_chpos};
                __micro_lexer_push_tok(tok);
            } else
            if (!strcmp(buf, "while")) {
                micro_token_t tok = {.type = MICRO_TT_KW_WHILE, .val = 0, .line_ref = line, .chpos_ref = tok_start_chpos};
                __micro_lexer_push_tok(tok);
            } else
            if (!strcmp(buf, "start")) {
                micro_token_t tok = {.type = MICRO_TT_KW_START, .val = 0, .line_ref = line, .chpos_ref = tok_start_chpos};
                __micro_lexer_push_tok(tok);
            } else
            if (!strcmp(buf, "end")) {
                micro_token_t tok = {.type = MICRO_TT_KW_END, .val = 0, .line_ref = line, .chpos_ref = tok_start_chpos};
                __micro_lexer_push_tok(tok);
            } else
            if (!strcmp(buf, "ret")) {
                micro_token_t tok = {.type = MICRO_TT_KW_RET, .val = 0, .line_ref = line, .chpos_ref = tok_start_chpos};
                __micro_lexer_push_tok(tok);
            } else
            if (!strcmp(buf, "call")) {
                micro_token_t tok = {.type = MICRO_TT_KW_CALL, .val = 0, .line_ref = line, .chpos_ref = tok_start_chpos};
                __micro_lexer_push_tok(tok);
            } else
            if (!strcmp(buf, "goto")) {
                micro_token_t tok = {.type = MICRO_TT_KW_GOTO, .val = 0, .line_ref = line, .chpos_ref = tok_start_chpos};
                __micro_lexer_push_tok(tok);
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
                __micro_lexer_push_tok(tok);
            } else {
                micro_token_t tok = {.type = MICRO_TT_IDENT, .line_ref = line, .chpos_ref = tok_start_chpos};
                strcpy(tok.val, buf);
                __micro_lexer_push_tok(tok);
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
            __micro_lexer_push_tok(tok);
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
                    __micro_lexer_push_err(err);
                    goto err_exit;
                }
            }
            buf[i] = '\0';

            micro_token_t tok = {.type = MICRO_TT_LIT_STR, .line_ref = line, .chpos_ref = tok_start_chpos};
            strcpy(tok.val, buf);
            __micro_lexer_push_tok(tok);
        } else
        if (text[pos] == '+') {
            micro_token_t tok = {.type = MICRO_TT_PLUS, .val = 0, .line_ref = line, .chpos_ref = chpos};
            __micro_lexer_push_tok(tok);
        } else
        if (text[pos] == '-') {
            micro_token_t tok = {.type = MICRO_TT_MINUS, .val = 0, .line_ref = line, .chpos_ref = chpos};
            __micro_lexer_push_tok(tok);
        } else
        if (text[pos] == '*') {
            micro_token_t tok = {.type = MICRO_TT_STAR, .val = 0, .line_ref = line, .chpos_ref = chpos};
            __micro_lexer_push_tok(tok);
        } else
        if (text[pos] == '/') {
            micro_token_t tok = {.type = MICRO_TT_SLASH, .val = 0, .line_ref = line, .chpos_ref = chpos};
            __micro_lexer_push_tok(tok);
        } else
        if (text[pos] == '.') {
            micro_token_t tok = {.type = MICRO_TT_DOT, .val = 0, .line_ref = line, .chpos_ref = chpos};
            __micro_lexer_push_tok(tok);
        } else
        if (text[pos] == ',') {
            micro_token_t tok = {.type = MICRO_TT_COMA, .val = 0, .line_ref = line, .chpos_ref = chpos};
            __micro_lexer_push_tok(tok);
        } else
        if (text[pos] == ':') {
            micro_token_t tok = {.type = MICRO_TT_COLON, .val = 0, .line_ref = line, .chpos_ref = chpos};
            __micro_lexer_push_tok(tok);
        } else
        if (text[pos] == ';') {
            micro_token_t tok = {.type = MICRO_TT_SEMICOLON, .val = 0, .line_ref = line, .chpos_ref = chpos};
            __micro_lexer_push_tok(tok);
        } else
        if (text[pos] == '&') {
            micro_token_t tok = {.type = MICRO_TT_AMPERRSAND, .val = 0, .line_ref = line, .chpos_ref = chpos};
            __micro_lexer_push_tok(tok);
        } else
        if (text[pos] == '$') {
            micro_token_t tok = {.type = MICRO_TT_DOLAR, .val = 0, .line_ref = line, .chpos_ref = chpos};
            __micro_lexer_push_tok(tok);
        } else
        if (text[pos] == '#') {
            micro_token_t tok = {.type = MICRO_TT_HASH, .val = 0, .line_ref = line, .chpos_ref = chpos};
            __micro_lexer_push_tok(tok);
        } else
        if (text[pos] == '`') {
            micro_token_t tok = {.type = MICRO_TT_APOSTROPHE, .val = 0, .line_ref = line, .chpos_ref = chpos};
            __micro_lexer_push_tok(tok);
        } else
        if (text[pos] == '~') {
            micro_token_t tok = {.type = MICRO_TT_TILDE, .val = 0, .line_ref = line, .chpos_ref = chpos};
            __micro_lexer_push_tok(tok);
        }
        
        pos++;
        chpos++;
    }
err_exit:;
}