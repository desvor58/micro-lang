#include <microc/lexer.h>

char *mc_token_type2str[] = {
    [MC_TOK_NULL]        = "null",
    [MC_TOK_PLUS]        = "plus",
    [MC_TOK_MINUS]       = "minus",
    [MC_TOK_STAR]        = "star",
    [MC_TOK_SLASH]       = "slash",
    [MC_TOK_DOT]         = "dot",
    [MC_TOK_COMA]        = "coma",
    [MC_TOK_COLON]       = "colon",
    [MC_TOK_SEMICOLON]   = "semicolon",
    [MC_TOK_AMPERSAND]   = "ampersand",
    [MC_TOK_DOLLAR]      = "dollar",
    [MC_TOK_HASH]        = "hash",
    [MC_TOK_APOSTROPHE]  = "apostrophe",
    [MC_TOK_TILDE]       = "tilde",
    [MC_TOK_EXCLAMATION] = "exclamation",
    [MC_TOK_EQ]          = "eq",
    [MC_TOK_GREAT]       = "great",
    [MC_TOK_LESS]        = "less",
    [MC_TOK_GREAT_OR_EQ] = "great or eq",
    [MC_TOK_LESS_OR_EQ]  = "less or eq",

    [MC_TOK_TYPE_NAME]   = "type name",
    [MC_TOK_IDENT]       = "ident",
    [MC_TOK_LIT_INT]     = "lit int",
    [MC_TOK_LIT_FLOAT]   = "lit float",
    [MC_TOK_LIT_STR]     = "lit str",

    [MC_TOK_KW_FUN]      = "fun",
    [MC_TOK_KW_SET]      = "set",
    [MC_TOK_KW_IF]       = "if",
    [MC_TOK_KW_ELSE]     = "else",
    [MC_TOK_KW_WHILE]    = "while",
    [MC_TOK_KW_START]    = "start",
    [MC_TOK_KW_END]      = "end",
    [MC_TOK_KW_RET]      = "ret",
    [MC_TOK_KW_CALL]     = "call",
    [MC_TOK_KW_GOTO]     = "goto",
};

void mc_tokenize(const char *text, size_t text_size, sct_vector_t *toks)
{
    size_t pos = 0;
    size_t line = 1;
    size_t chpos = 1;

    char *buf = amalloc(sizeof(char) * MICRO_MAX_SYMBOL_SIZE);

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
                        .err = MICRO_ERROR_EXPECTED_COMMENT_CLOSE,
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

            if (!strcmp(buf, "fun")) {
                sct_vector_push(toks, &(mc_token_t){
                    .type = MC_TOK_KW_FUN,
                    .val = 0,
                    .line_ref = line,
                    .chpos_ref = tok_start_chpos
                });
            } else
            if (!strcmp(buf, "set")) {
                sct_vector_push(toks, &(mc_token_t){
                    .type = MC_TOK_KW_SET,
                    .val = 0,
                    .line_ref = line,
                    .chpos_ref = tok_start_chpos
                });
            } else
            if (!strcmp(buf, "if")) {
                sct_vector_push(toks, &(mc_token_t){
                    .type = MC_TOK_KW_IF,
                    .val = 0,
                    .line_ref = line,
                    .chpos_ref = tok_start_chpos
                });
            } else
            if (!strcmp(buf, "else")) {
                sct_vector_push(toks, &(mc_token_t){
                    .type = MC_TOK_KW_ELSE,
                    .val = 0,
                    .line_ref = line,
                    .chpos_ref = tok_start_chpos
                });
            } else
            if (!strcmp(buf, "while")) {
                sct_vector_push(toks, &(mc_token_t){
                    .type = MC_TOK_KW_WHILE,
                    .val = 0,
                    .line_ref = line,
                    .chpos_ref = tok_start_chpos
                });
            } else
            if (!strcmp(buf, "start")) {
                sct_vector_push(toks, &(mc_token_t){
                    .type = MC_TOK_KW_START,
                    .val = 0,
                    .line_ref = line,
                    .chpos_ref = tok_start_chpos
                });
            } else
            if (!strcmp(buf, "end")) {
                sct_vector_push(toks, &(mc_token_t){
                    .type = MC_TOK_KW_END,
                    .val = 0,
                    .line_ref = line,
                    .chpos_ref = tok_start_chpos
                });
            } else
            if (!strcmp(buf, "ret")) {
                sct_vector_push(toks, &(mc_token_t){
                    .type = MC_TOK_KW_RET,
                    .val = 0,
                    .line_ref = line,
                    .chpos_ref = tok_start_chpos
                });
            } else
            if (!strcmp(buf, "call")) {
                sct_vector_push(toks, &(mc_token_t){
                    .type = MC_TOK_KW_CALL,
                    .val = 0,
                    .line_ref = line,
                    .chpos_ref = tok_start_chpos
                });
            } else
            if (!strcmp(buf, "goto")) {
                sct_vector_push(toks, &(mc_token_t){
                    .type = MC_TOK_KW_GOTO,
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
                mc_token_t tok = (mc_token_t){
                    .type = MC_TOK_TYPE_NAME,
                    .line_ref = line,
                    .chpos_ref = tok_start_chpos
                };
                strcpy(tok.val, buf);
                sct_vector_push(toks, &tok);
            } else {
                mc_token_t tok = {
                    .type = MC_TOK_IDENT,
                    .line_ref = line,
                    .chpos_ref = tok_start_chpos
                };
                strcpy(tok.val, buf);
                sct_vector_push(toks, &tok);
            }
            pos--;
            chpos--;
        } else
        if (isdigit(text[pos]) || (text[pos] == '-' && isdigit(text[pos + 1]))) {
            size_t tok_start_chpos = chpos;
            mc_token_type_t type = MC_TOK_LIT_INT;
            size_t i = 0;
            do {
                buf[i++] = text[pos++];
                chpos++;
                if (text[pos] == '.') {
                    buf[i++] = text[pos++];
                    chpos++;
                    type = MC_TOK_LIT_FLOAT;
                }
            } while (isdigit(text[pos]) && pos < text_size);
            buf[i] = '\0';

            mc_token_t tok = {
                .type = type,
                .line_ref = line,
                .chpos_ref = tok_start_chpos
            };
            strcpy(tok.val, buf);
            sct_vector_push(toks, &tok);
            pos--;
            chpos--;
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
                        .err = MICRO_ERROR_EXPECTED_STRING_CLOSE,
                        .line_ref = line,
                        .chpos_ref = chpos
                    });
                    goto err_exit;
                }
            }
            buf[i] = '\0';

            mc_token_t tok = {
                .type = MC_TOK_LIT_STR,
                .line_ref = line,
                .chpos_ref = tok_start_chpos
            };
            strcpy(tok.val, buf);
            sct_vector_push(toks, &tok);
        } else
        _mc_single_chlex('+', MC_TOK_PLUS)        else
        _mc_single_chlex('-', MC_TOK_MINUS)       else
        _mc_single_chlex('*', MC_TOK_STAR)        else
        _mc_single_chlex('/', MC_TOK_SLASH)       else
        _mc_single_chlex('.', MC_TOK_DOT)         else
        _mc_single_chlex(',', MC_TOK_COMA)        else
        _mc_single_chlex(':', MC_TOK_COLON)       else
        _mc_single_chlex(';', MC_TOK_SEMICOLON)   else
        _mc_single_chlex('&', MC_TOK_AMPERSAND)   else
        _mc_single_chlex('$', MC_TOK_DOLLAR)      else
        _mc_single_chlex('#', MC_TOK_HASH)        else
        _mc_single_chlex('`', MC_TOK_APOSTROPHE)  else
        _mc_single_chlex('=', MC_TOK_EQ)          else
        _mc_single_chlex('~', MC_TOK_TILDE)
        
        if (text[pos] == '>' && text[pos + 1] == '=') {
            sct_vector_push(toks, &(mc_token_t){
                .type = MC_TOK_GREAT_OR_EQ,
                .line_ref = line,
                .chpos_ref = chpos,
            });
            pos += 2;
            chpos += 2;
            continue;
        }
        if (text[pos] == '<' && text[pos + 1] == '=') {
            sct_vector_push(toks, &(mc_token_t){
                .type = MC_TOK_LESS_OR_EQ,
                .line_ref = line,
                .chpos_ref = chpos,
            });
            pos += 2;
            chpos += 2;
            continue;
        }
        
        _mc_single_chlex('!', MC_TOK_EXCLAMATION) else
        _mc_single_chlex('>', MC_TOK_GREAT)       else
        _mc_single_chlex('<', MC_TOK_LESS)
        
        pos++;
        chpos++;
    }
err_exit:
    free(buf);
}