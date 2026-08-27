#include <microc/instrgen.h>

void mc_instrgen_parse_call(mc_instrgen_t *instrgen)
{
    mc_token_t *call_tok = sct_vector_get(instrgen->toks, instrgen->pos++);
    if (unlikely(!call_tok || call_tok->type != MC_TOK_KW_CALL)) {
        micro_push_err((micro_error_t){
            .err = MICRO_ERROR_EXPECTED_CALL_KW,
            .instr = MICRO_INSTR_CALL
        });
        goto exit;
    }

    if (!instrgen->code_in_function) {
        micro_push_err((micro_error_t){
            .err = MICRO_ERROR_CALL_OUTSIDE_FUNCTION,
            .instr = MICRO_INSTR_CALL
        });
        goto exit;
    }

    mc_token_t *ret_reg_tok = sct_vector_get(instrgen->toks, instrgen->pos++);
    if (!ret_reg_tok || ret_reg_tok->type != MC_TOK_IDENT) {
        micro_push_err((micro_error_t){
            .err = MICRO_ERROR_EXPECTED_RESULT_REG,
            .instr = MICRO_INSTR_CALL
        });
        goto exit;
    }

    mc_token_t *fun_name_tok = sct_vector_get(instrgen->toks, instrgen->pos++);
    if (!fun_name_tok || fun_name_tok->type != MC_TOK_IDENT) {
        micro_push_err((micro_error_t){
            .err = MICRO_ERROR_EXPECTED_FUN_NAME,
            .instr = MICRO_INSTR_CALL
        });
        goto exit;
    }

    micro_instruction_call_t call_instr;
    strcpy(call_instr.ret_reg_name, ret_reg_tok->val);
    strcpy(call_instr.fun_name, fun_name_tok->val);
    sct_vector_init(&call_instr.arg_exprs, sizeof(micro_expr_tok_t*));

    mc_token_t *tok = sct_vector_get(instrgen->toks, instrgen->pos);
    for (;;) {
        if (!tok) {
            micro_push_err((micro_error_t){
                .err = MICRO_ERROR_EXPECTED_SEMICOLON,
                .instr = MICRO_INSTR_CALL
            });
            return;
        }
        if (tok->type == MC_TOK_SEMICOLON) {
            break;
        }
        if (!_micro_expr_is_expr_start(tok->type)) {
            micro_push_err((micro_error_t){
                .err = MICRO_ERROR_EXPECTED_EXPRESSION,
                .instr = MICRO_INSTR_CALL
            });
            goto exit;
        }
        micro_expr_tok_t *arg_expr = (micro_expr_tok_t*)tok;
        sct_vector_push(&call_instr.arg_exprs, &arg_expr);
        size_t expr_size = mc_scroll_expr(instrgen->toks, instrgen->pos);
        if (!expr_size) {
            goto exit;
        }
        instrgen->pos += expr_size;
        tok = sct_vector_get(instrgen->toks, instrgen->pos);
    }

    sct_vector_push(&instrgen->instructions, &(micro_instruction_t){
        .type = MICRO_INSTR_CALL,
        .call = call_instr
    });

exit:
    while (instrgen->pos < instrgen->toks->size && ((mc_token_t*)sct_vector_get(instrgen->toks, instrgen->pos))->type != MC_TOK_SEMICOLON) {
        instrgen->pos++;
    }
}
