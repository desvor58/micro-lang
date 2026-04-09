#include <micro/lexer.h>

char *micro_token_type2str[] = {
    [MICRO_TT_NULL]       = "null",
    [MICRO_TT_PLUS]       = "plus",
    [MICRO_TT_MINUS]      = "minus",
    [MICRO_TT_STAR]       = "star",
    [MICRO_TT_SLASH]      = "slash",
    [MICRO_TT_DOT]        = "dot",
    [MICRO_TT_COMA]       = "coma",
    [MICRO_TT_COLON]      = "colon",
    [MICRO_TT_SEMICOLON]  = "semicolon",
    [MICRO_TT_AMPERSAND]  = "ampersand",
    [MICRO_TT_DOLLAR]     = "dollar",
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

void micro_create_tok_vec(micro_tok_vec_t *vec)
{
    vec->toks = malloc(MICRO_TOKEN_VEC_EXTEND_SIZE * sizeof(micro_token_t));
    vec->size = 0;
    vec->real_size = MICRO_TOKEN_VEC_EXTEND_SIZE;
}

void micro_push_tok(micro_tok_vec_t *vec, micro_token_t tok)
{
    if (vec->size + 1 >= vec->real_size) {
        micro_token_t *new_toks = malloc((vec->real_size += MICRO_TOKEN_VEC_EXTEND_SIZE) * sizeof(micro_token_t));
        memcpy(new_toks, vec->toks, sizeof(micro_token_t) * vec->size);
        free(vec->toks);
        vec->toks = new_toks;
    }
    vec->toks[vec->size++] = tok;
}

void micro_free_tok_vec(micro_tok_vec_t *vec)
{
    free(vec->toks);
}

void micro_tokenize(const char *text, size_t text_size, micro_tok_vec_t *toks)
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
                    micro_push_err((micro_error_t){
                        .msg = "Expected closing '\\' character for comment",
                        .line_ref = line,
                        .chpos_ref = chpos
                    });
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
                micro_push_tok(toks, (micro_token_t){
                    .type = MICRO_TT_KW_VAR,
                    .val = 0,
                    .line_ref = line,
                    .chpos_ref = tok_start_chpos
                });
            } else
            if (!strcmp(buf, "fun")) {
                micro_push_tok(toks, (micro_token_t){
                    .type = MICRO_TT_KW_FUN,
                    .val = 0,
                    .line_ref = line,
                    .chpos_ref = tok_start_chpos
                });
            } else
            if (!strcmp(buf, "set")) {
                micro_push_tok(toks, (micro_token_t){
                    .type = MICRO_TT_KW_SET,
                    .val = 0,
                    .line_ref = line,
                    .chpos_ref = tok_start_chpos
                });
            } else
            if (!strcmp(buf, "if")) {
                micro_push_tok(toks, (micro_token_t){
                    .type = MICRO_TT_KW_IF,
                    .val = 0,
                    .line_ref = line,
                    .chpos_ref = tok_start_chpos
                });
            } else
            if (!strcmp(buf, "else")) {
                micro_push_tok(toks, (micro_token_t){
                    .type = MICRO_TT_KW_ELSE,
                    .val = 0,
                    .line_ref = line,
                    .chpos_ref = tok_start_chpos
                });
            } else
            if (!strcmp(buf, "while")) {
                micro_push_tok(toks, (micro_token_t){
                    .type = MICRO_TT_KW_WHILE,
                    .val = 0,
                    .line_ref = line,
                    .chpos_ref = tok_start_chpos
                });
            } else
            if (!strcmp(buf, "start")) {
                micro_push_tok(toks, (micro_token_t){
                    .type = MICRO_TT_KW_START,
                    .val = 0,
                    .line_ref = line,
                    .chpos_ref = tok_start_chpos
                });
            } else
            if (!strcmp(buf, "end")) {
                micro_push_tok(toks, (micro_token_t){
                    .type = MICRO_TT_KW_END,
                    .val = 0,
                    .line_ref = line,
                    .chpos_ref = tok_start_chpos
                });
            } else
            if (!strcmp(buf, "ret")) {
                micro_push_tok(toks, (micro_token_t){
                    .type = MICRO_TT_KW_RET,
                    .val = 0,
                    .line_ref = line,
                    .chpos_ref = tok_start_chpos
                });
            } else
            if (!strcmp(buf, "call")) {
                micro_push_tok(toks, (micro_token_t){
                    .type = MICRO_TT_KW_CALL,
                    .val = 0,
                    .line_ref = line,
                    .chpos_ref = tok_start_chpos
                });
            } else
            if (!strcmp(buf, "goto")) {
                micro_push_tok(toks, (micro_token_t){
                    .type = MICRO_TT_KW_GOTO,
                    .val = 0,
                    .line_ref = line,
                    .chpos_ref = tok_start_chpos
                });
            } else
            if (!strcmp(buf, "i8")
             || !strcmp(buf, "u8")
             || !strcmp(buf, "i16")
             || !strcmp(buf, "u16")
             || !strcmp(buf, "i32")
             || !strcmp(buf, "u32")
             || !strcmp(buf, "f32")
             || !strcmp(buf, "ptr")) {
                micro_token_t tok = (micro_token_t){
                    .type = MICRO_TT_TYPE_NAME,
                    .line_ref = line,
                    .chpos_ref = tok_start_chpos
                };
                strcpy(tok.val, buf);
                micro_push_tok(toks, tok);
            } else {
                micro_token_t tok = {
                    .type = MICRO_TT_IDENT,
                    .line_ref = line,
                    .chpos_ref = tok_start_chpos
                };
                strcpy(tok.val, buf);
                micro_push_tok(toks, tok);
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

            micro_token_t tok = {
                .type = type,
                .line_ref = line,
                .chpos_ref = tok_start_chpos
            };
            strcpy(tok.val, buf);
            micro_push_tok(toks, tok);
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
                    micro_push_err((micro_error_t){
                        .msg = "Expected closing '\"' character for the string literal",
                        .line_ref = line,
                        .chpos_ref = chpos
                    });
                    goto err_exit;
                }
            }
            buf[i] = '\0';

            micro_token_t tok = {
                .type = MICRO_TT_LIT_STR,
                .line_ref = line,
                .chpos_ref = tok_start_chpos
            };
            strcpy(tok.val, buf);
            micro_push_tok(toks, tok);
        } else
        __micro_single_chlex('+', MICRO_TT_PLUS)       else
        __micro_single_chlex('-', MICRO_TT_MINUS)      else
        __micro_single_chlex('*', MICRO_TT_STAR)       else
        __micro_single_chlex('/', MICRO_TT_SLASH)      else
        __micro_single_chlex('.', MICRO_TT_DOT)        else
        __micro_single_chlex(',', MICRO_TT_COMA)       else
        __micro_single_chlex(':', MICRO_TT_COLON)      else
        __micro_single_chlex(';', MICRO_TT_SEMICOLON)  else
        __micro_single_chlex('&', MICRO_TT_AMPERSAND)  else
        __micro_single_chlex('$', MICRO_TT_DOLLAR)     else
        __micro_single_chlex('#', MICRO_TT_HASH)       else
        __micro_single_chlex('`', MICRO_TT_APOSTROPHE) else
        __micro_single_chlex('~', MICRO_TT_TILDE)
        
        pos++;
        chpos++;
    }
err_exit:;
}