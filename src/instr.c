#include <micro/instr.h>

#define parse_opch(C, T, O)  \
    if (c == (C)) {  \
        sct_vector_push(expr_dst, &(micro_expr_tok_t){  \
            .type = (T),  \
            .line_ref = 0,  \
            .chpos_ref = pos + 1  \
        });  \
        expected_toks_num += (O);  \
    }

size_t micro_make_expr(sct_vector_t *expr_dst, const char *str_expr)
{
    if (!expr_dst || !str_expr) {
        return 0;
    }

    size_t pos = 0;
    size_t expected_toks_num = 1;

    while (str_expr[pos]) {
        char c = str_expr[pos];

        if (isalpha(c) || c == '_') {
            char buf[MICRO_MAX_SYMBOL_SIZE];
            size_t i = 0;
            while (c && isalnum(c) || c == '_') {
                if (i >= MICRO_MAX_SYMBOL_SIZE) {
                    micro_push_err((micro_error_t){
                        .err = MICRO_ERROR_IDENT_NAME_TOO_LONG,
                        .line_ref = 0,
                        .chpos_ref = pos + 1,
                    });
                    return 0;
                }
                buf[i++] += c;
                c = str_expr[++pos];
            }
            buf[i] = '\0';

            micro_expr_tok_t tok = {
                .type = MICRO_EXPR_TOK_IDENT,
                .line_ref = 0,
                .chpos_ref = pos + 1,
            };
            strcpy(tok.val, buf);
            sct_vector_push(expr_dst, &tok);
            expected_toks_num--;
        } else
        if (isdigit(c) || (c == '-' && isdigit(str_expr[pos + 1]))) {
            micro_expr_tok_type_t type = MICRO_EXPR_TOK_LIT_INT;
            char buf[MICRO_MAX_SYMBOL_SIZE];
            size_t i = 0;
            do {
                if (i >= MICRO_MAX_SYMBOL_SIZE) {
                    micro_push_err((micro_error_t){
                        .err = MICRO_ERROR_DIGIT_TOO_LONG,
                        .line_ref = 0,
                        .chpos_ref = pos + 1,
                    });
                    return 0;
                }
                buf[i++] = str_expr[pos++];
                if (str_expr[pos] == '.') {
                    buf[i++] = str_expr[pos++];
                    type = MICRO_EXPR_TOK_LIT_FLOAT;
                }
            } while (str_expr[pos] && isdigit(str_expr[pos]));
            buf[i] = '\0';

            micro_expr_tok_t tok = {
                .type = type,
                .line_ref = 0,
                .chpos_ref = pos + 1
            };
            strcpy(tok.val, buf);
            sct_vector_push(expr_dst, &tok);
            expected_toks_num--;
        } else
        if (c == '"') {
            char buf[MICRO_MAX_SYMBOL_SIZE];
            size_t i = 0;
            char prev_c = c;
            while (str_expr[++pos] != '"' || prev_c == '\\') {
                if (i >= MICRO_MAX_SYMBOL_SIZE) {
                    micro_push_err((micro_error_t){
                        .err = MICRO_ERROR_STR_LIT_TOO_LONG,
                        .line_ref = 0,
                        .chpos_ref = pos + 1,
                    });
                    return 0;
                }
                buf[i] = str_expr[pos];
                prev_c = str_expr[pos];
            }
            expected_toks_num--;
        } else
        parse_opch('+', MICRO_EXPR_TOK_PLUS, 1)         else
        parse_opch('-', MICRO_EXPR_TOK_MINUS, 1)        else
        parse_opch('/', MICRO_EXPR_TOK_SLASH, 1)        else
        parse_opch('*', MICRO_EXPR_TOK_STAR, 1)         else
        parse_opch('&', MICRO_EXPR_TOK_AMPERSAND, 0)    else
        parse_opch('$', MICRO_EXPR_TOK_DOLLAR, 0)       else
        parse_opch('#', MICRO_EXPR_TOK_HASH, 0)         else
        parse_opch('`', MICRO_EXPR_TOK_APOSTROPHE, 0)   else
        parse_opch('~', MICRO_EXPR_TOK_TILDE, 0)        else
        parse_opch('=', MICRO_EXPR_TOK_EQ, 1)           else
        parse_opch('!', MICRO_EXPR_TOK_EXCLAMATION, 0)  else
        if (c == '>' && str_expr[pos + 1] == '=') {
            sct_vector_push(expr_dst, &(micro_expr_tok_t){
                .type = MICRO_EXPR_TOK_GREAT_OR_EQ,
                .line_ref = 0,
                .chpos_ref = pos + 1,
            });
            expected_toks_num++;
        } else
        if (c == '<' && str_expr[pos + 1] == '=') {
            sct_vector_push(expr_dst, &(micro_expr_tok_t){
                .type = MICRO_EXPR_TOK_LESS_OR_EQ,
                .line_ref = 0,
                .chpos_ref = pos + 1,
            });
            expected_toks_num++;
        } else
        parse_opch('>', MICRO_EXPR_TOK_GREAT, 1)        else
        parse_opch('<', MICRO_EXPR_TOK_LESS, 1)
    }

    if (expected_toks_num) {
        micro_push_err((micro_error_t){
            .err = MICRO_ERROR_EXPECTED_EXPRESSION,
            .line_ref = 0,
            .chpos_ref = pos,
        });
        return 0;
    }
    return expr_dst->size;
}

#undef parse_opch

int micro_instr_gen_set(sct_vector_t *instrs, micro_type_t type, const char *name, sct_vector_t *expr)
{
    if (!instrs || type == MICRO_TYPE_NULL || !name || strlen(name) > MICRO_MAX_SYMBOL_SIZE - 1 || !expr) {
        return 1;
    }

    size_t expr_size = mc_scroll_expr(expr, 0);

    micro_instruction_set_t instr_set;
    strcpy(instr_set.reg_name, name);
    instr_set.type = type;
    instr_set.val_expr = (micro_expr_tok_t*)expr->data;

    sct_vector_push(instrs, &(micro_instruction_t){
        .type = MICRO_INSTR_SET,
        .start_tok = 0,
        .set = instr_set,
    });
}