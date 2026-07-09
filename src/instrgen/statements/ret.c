#include <micro/instrgen.h>

void micro_instrgen_parse_ret(micro_instrgen_t *instrgen)
{
    micro_token_t *ret_tok = sct_vector_get(instrgen->toks, instrgen->pos++);
    if (!ret_tok || ret_tok->type != MICRO_TOK_KW_RET) {
        micro_push_err((micro_error_t){
            .msg = "Expected 'ret' keyword",
            .line_ref = ret_tok ? ret_tok->line_ref : 0,
            .chpos_ref = ret_tok ? ret_tok->chpos_ref : 0
        });
        goto exit;
    }
    
    if (!instrgen->code_in_function) {
        micro_push_err((micro_error_t){
            .msg = "'ret' instruction can be only in function body",
            .line_ref = ret_tok ? ret_tok->line_ref : 0,
            .chpos_ref = ret_tok ? ret_tok->chpos_ref : 0
        });
        goto exit;
    }

    micro_token_t *expr_start_tok = sct_vector_get(instrgen->toks, instrgen->pos++);
    if (!expr_start_tok || (expr_start_tok->type != MICRO_TOK_SEMICOLON && !_micro_tok_is_expr_start(expr_start_tok->type))) {
        micro_push_err((micro_error_t){
            .msg = "Expected expression",
            .line_ref = expr_start_tok ? expr_start_tok->line_ref : 0,
            .chpos_ref = expr_start_tok ? expr_start_tok->chpos_ref : 0
        });
        goto exit;
    }
    if (expr_start_tok->type == MICRO_TOK_SEMICOLON) {
        instrgen->pos--;
        expr_start_tok = 0;
    }

    sct_vector_push(&instrgen->instructions, &(micro_instruction_t){
        .type = MICRO_INSTR_RET,
        .start_tok = ret_tok,
        .ret = {
            .val_expr = expr_start_tok
        }
    });

exit:
    while (instrgen->pos < instrgen->toks->size && ((micro_token_t*)sct_vector_get(instrgen->toks, instrgen->pos))->type != MICRO_TOK_SEMICOLON) {
        instrgen->pos++;
    }
}
