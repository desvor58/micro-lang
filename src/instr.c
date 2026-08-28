#include <ctype.h>
#include <string.h>

#include <micro/instr.h>

#define parse_opch(C, T, O)  \
    if (c == (C)) {  \
        sct_vector_push(expr_dst, &(micro_expr_tok_t){  \
            .type = (T)  \
        });  \
        expected_toks_num += (O);  \
        pos++;  \
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
            while ((c && isalnum(c)) || c == '_') {
                if (i >= MICRO_MAX_SYMBOL_SIZE) {
                    micro_push_err((micro_error_t){
                        .err = MICRO_ERROR_IDENT_NAME_TOO_LONG,
                    });
                    return 0;
                }
                buf[i++] = c;
                c = str_expr[++pos];
            }
            buf[i] = '\0';

            micro_expr_tok_t tok = {
                .type = MICRO_EXPR_TOK_IDENT,
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
                    });
                    return 0;
                }
                buf[i] = str_expr[pos];
                prev_c = str_expr[pos];
            }
            buf[i] = '\0';

            micro_expr_tok_t tok = {
                .type = MICRO_EXPR_TOK_LIT_STR,
            };
            strcpy(tok.val, buf);
            sct_vector_push(expr_dst, &tok);
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
            });
            expected_toks_num++;
            pos += 2;
        } else
        if (c == '<' && str_expr[pos + 1] == '=') {
            sct_vector_push(expr_dst, &(micro_expr_tok_t){
                .type = MICRO_EXPR_TOK_LESS_OR_EQ,
            });
            expected_toks_num++;
            pos += 2;
        } else
        parse_opch('>', MICRO_EXPR_TOK_GREAT, 1)        else
        parse_opch('<', MICRO_EXPR_TOK_LESS, 1)

        pos++;
    }

    if (expected_toks_num) {
        micro_push_err((micro_error_t){
            .err = MICRO_ERROR_EXPECTED_EXPRESSION,
        });
        return 0;
    }
    return expr_dst->size;
}

#undef parse_opch

int micro_instr_gen_set(sct_vector_t *instrs, micro_type_t type, const char *name, sct_vector_t *expr)
{
    if (!instrs || type == MICRO_TYPE_NULL || !name || strlen(name) > MICRO_MAX_SYMBOL_SIZE - 1) {
        return 1;
    }

    micro_instruction_set_t instr_set;
    strcpy(instr_set.reg_name, name);
    instr_set.type = type;
    instr_set.val_expr = expr ? (micro_expr_tok_t*)expr->data : 0;

    sct_vector_push(instrs, &(micro_instruction_t){
        .type = MICRO_INSTR_SET,
        .set = instr_set,
    });

    return 0;
}

int micro_instr_gen_drset(sct_vector_t *instrs, micro_type_t type, const char *name, sct_vector_t *expr)
{
    if (!instrs || type == MICRO_TYPE_NULL || !name || strlen(name) > MICRO_MAX_SYMBOL_SIZE - 1) {
        return 1;
    }

    micro_instruction_drset_t instr_drset;
    strcpy(instr_drset.reg_name, name);
    instr_drset.type = type;
    instr_drset.val_expr = expr ? (micro_expr_tok_t*)expr->data : 0;

    sct_vector_push(instrs, &(micro_instruction_t){
        .type = MICRO_INSTR_DRSET,
        .drset = instr_drset,
    });

    return 0;
}

int micro_instr_gen_fun(sct_vector_t *instrs, const char *name, sct_vector_t *args, micro_type_t ret_type, sct_vector_t *body)
{
    if (!instrs || !name || strlen(name) > MICRO_MAX_SYMBOL_SIZE - 1 || !args || !body) {
        return 1;
    }

    micro_instruction_fun_t instr_fun;
    strcpy(instr_fun.name, name);
    instr_fun.ret_type = ret_type;
    instr_fun.args = *args;
    instr_fun.body = *body;

    sct_vector_push(instrs, &(micro_instruction_t){
        .type = MICRO_INSTR_FUN,
        .fun = instr_fun,
    });

    return 0;
}

int micro_instr_gen_ret(sct_vector_t *instrs, sct_vector_t *expr)
{
    if (!instrs) {
        return 1;
    }

    micro_instruction_ret_t instr_ret;
    instr_ret.val_expr = expr ? (micro_expr_tok_t*)expr->data : 0;

    sct_vector_push(instrs, &(micro_instruction_t){
        .type = MICRO_INSTR_RET,
        .ret = instr_ret,
    });

    return 0;
}

int micro_instr_gen_call(sct_vector_t *instrs, const char *reg_name, const char *fun_name, sct_vector_t *args)
{
    if (!instrs || !reg_name || !fun_name || strlen(reg_name) > MICRO_MAX_SYMBOL_SIZE - 1 || strlen(fun_name) > MICRO_MAX_SYMBOL_SIZE - 1 || !args) {
        return 1;
    }

    micro_instruction_call_t instr_call;
    strcpy(instr_call.ret_reg_name, reg_name);
    strcpy(instr_call.fun_name, fun_name);
    instr_call.arg_exprs = *args;
    
    sct_vector_push(instrs, &(micro_instruction_t){
        .type = MICRO_INSTR_CALL,
        .call = instr_call,
    });

    return 0;
}

int micro_instr_gen_lbl(sct_vector_t *instrs, const char *name)
{
    if (!instrs || !name || strlen(name) > MICRO_MAX_SYMBOL_SIZE - 1) {
        return 1;
    }

    micro_instruction_lbl_t instr_lbl;
    strcpy(instr_lbl.name, name);

    sct_vector_push(instrs, &(micro_instruction_t){
        .type = MICRO_INSTR_LBL,
        .lbl = instr_lbl,
    });

    return 0;
}

int micro_instr_gen_goto(sct_vector_t *instrs, const char *name)
{
    if (!instrs || !name || strlen(name) > MICRO_MAX_SYMBOL_SIZE - 1) {
        return 1;
    }

    micro_instruction_goto_t instr_goto;
    strcpy(instr_goto.lbl, name);

    sct_vector_push(instrs, &(micro_instruction_t){
        .type = MICRO_INSTR_GOTO,
        .goto_lbl = instr_goto,
    });

    return 0;
}

int micro_instr_gen_if(sct_vector_t *instrs, sct_vector_t *cond_expr, const char *lbl_name)
{
    if (!instrs || !cond_expr || !lbl_name || strlen(lbl_name) > MICRO_MAX_SYMBOL_SIZE - 1) {
        return 1;
    }

    micro_instruction_if_t instr_if;
    instr_if.cond_expr = (micro_expr_tok_t*)cond_expr->data;
    strcpy(instr_if.lbl_name, lbl_name);

    sct_vector_push(instrs, &(micro_instruction_t){
        .type = MICRO_INSTR_IF,
        .if_goto = instr_if,
    });

    return 0;
}
