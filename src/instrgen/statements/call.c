#include <micro/instrgen.h>

void micro_instrgen_parse_call(micro_instrgen_t *instrgen)
{
    micro_token_t *call_tok = sct_vector_get(instrgen->toks, instrgen->pos++);
    if (!call_tok || call_tok->type != MICRO_TOK_KW_CALL) {
        micro_push_err((micro_error_t){
            .msg = "Expected 'call' keyword",
            .line_ref = call_tok ? call_tok->line_ref : 0,
            .chpos_ref = call_tok ? call_tok->chpos_ref : 0
        });
        goto exit;
    }

    if (!instrgen->code_in_function) {
        micro_push_err((micro_error_t){
            .msg = "'call' instruction can be only in function body",
            .line_ref = call_tok ? call_tok->line_ref : 0,
            .chpos_ref = call_tok ? call_tok->chpos_ref : 0
        });
        goto exit;
    }

    micro_token_t *ret_reg_tok = sct_vector_get(instrgen->toks, instrgen->pos++);
    if (!ret_reg_tok || ret_reg_tok->type != MICRO_TOK_IDENT) {
        micro_push_err((micro_error_t){
            .msg = "Expected register name for result or '_'",
            .line_ref = ret_reg_tok ? ret_reg_tok->line_ref : 0,
            .chpos_ref = ret_reg_tok ? ret_reg_tok->chpos_ref : 0
        });
        goto exit;
    }

    micro_token_t *fun_name_tok = sct_vector_get(instrgen->toks, instrgen->pos++);
    if (!fun_name_tok || fun_name_tok->type != MICRO_TOK_IDENT) {
        micro_push_err((micro_error_t){
            .msg = "Expected calling function name",
            .line_ref = fun_name_tok ? fun_name_tok->line_ref : 0,
            .chpos_ref = fun_name_tok ? fun_name_tok->chpos_ref : 0
        });
        goto exit;
    }

    micro_instruction_call_t call_instr;
    strcpy(call_instr.ret_reg_name, ret_reg_tok->val);
    strcpy(call_instr.fun_name, fun_name_tok->val);
    sct_vector_init(&call_instr.arg_exprs, sizeof(micro_token_t*));

    micro_token_t *tok = sct_vector_get(instrgen->toks, instrgen->pos);
    for (;;) {
        if (!tok) {
            micro_push_err((micro_error_t){
                .msg = "Expected ';'",
                .line_ref = 0,
                .chpos_ref = 0
            });
            return;
        }
        if (tok->type == MICRO_TOK_SEMICOLON) {
            break;
        }
        if (!_micro_tok_is_expr_start(tok->type)) {
            micro_push_err((micro_error_t){
                .msg = "Expected expression",
                .line_ref = tok->line_ref,
                .chpos_ref = tok->chpos_ref
            });
            goto exit;
        }
        sct_vector_push(&call_instr.arg_exprs, &tok);
        instrgen->pos += micro_scroll_expr(instrgen->toks, instrgen->pos);
        tok = sct_vector_get(instrgen->toks, instrgen->pos);
    }

    sct_vector_push(&instrgen->instructions, &(micro_instruction_t){
        .type = MICRO_INSTR_CALL,
        .call = call_instr
    });

exit:
    while (instrgen->pos < instrgen->toks->size && ((micro_token_t*)sct_vector_get(instrgen->toks, instrgen->pos))->type != MICRO_TOK_SEMICOLON) {
        instrgen->pos++;
    }
}
