#include <microc/instrgen.h>

void mc_instrgen_parse_ret(mc_instrgen_t *instrgen)
{
    mc_token_t *ret_tok = sct_vector_get(instrgen->toks, instrgen->pos++);
    if (unlikely(!ret_tok || ret_tok->type != MC_TOK_KW_RET)) {
        micro_push_err((micro_error_t){
            .err = MICRO_ERROR_EXPECTED_RET_KW,
            .instr = MICRO_INSTR_RET
        });
        goto exit;
    }
    
    if (!instrgen->code_in_function) {
        micro_push_err((micro_error_t){
            .err = MICRO_ERROR_RET_OUTSIDE_FUNCTION,
            .instr = MICRO_INSTR_RET
        });
        goto exit;
    }

    mc_token_t *expr_start_tok = sct_vector_get(instrgen->toks, instrgen->pos);
    if (!expr_start_tok || (expr_start_tok->type != MC_TOK_SEMICOLON && !_micro_expr_is_expr_start(expr_start_tok->type))) {
        micro_push_err((micro_error_t){
            .err = MICRO_ERROR_EXPECTED_EXPRESSION,
            .instr = MICRO_INSTR_RET
        });
        goto exit;
    }

    if (expr_start_tok->type == MC_TOK_SEMICOLON) {
        expr_start_tok = 0;
    } else {
        size_t expr_size = mc_scroll_expr(instrgen->toks, instrgen->pos);
        if (!expr_size) {
            goto exit;
        }
        mc_token_t *semicolon_tok = sct_vector_get(instrgen->toks, instrgen->pos + expr_size);
        if (!semicolon_tok || semicolon_tok->type != MC_TOK_SEMICOLON) {
            micro_push_err((micro_error_t) {
                .err = MICRO_ERROR_EXPECTED_SEMICOLON,
                .instr = MICRO_INSTR_RET
            });
            goto exit;
        }
    }

    sct_vector_push(&instrgen->instructions, &(micro_instruction_t){
        .type = MICRO_INSTR_RET,
        .ret = {
            .val_expr = (micro_expr_tok_t*)expr_start_tok
        }
    });

exit:
    while (instrgen->pos < instrgen->toks->size && ((mc_token_t*)sct_vector_get(instrgen->toks, instrgen->pos))->type != MC_TOK_SEMICOLON) {
        instrgen->pos++;
    }
}
