#include <microc/instrgen.h>

void mc_instrgen_parse_if(mc_instrgen_t *instrgen)
{
    mc_token_t *if_tok = sct_vector_get(instrgen->toks, instrgen->pos++);
    if (unlikely(!if_tok || if_tok->type != MC_TOK_KW_IF)) {
        micro_push_err((micro_error_t){
            .err = MICRO_ERROR_EXPECTED_IF_KW,
            .instr = MICRO_INSTR_IF
        });
        goto exit;
    }
    
    mc_token_t *expr_tok = sct_vector_get(instrgen->toks, instrgen->pos);
    if (!expr_tok || !_micro_expr_is_expr_start(expr_tok->type)) {
        micro_push_err((micro_error_t){
            .err = MICRO_ERROR_EXPECTED_EXPRESSION,
            .instr = MICRO_INSTR_IF
        });
        goto exit;
    }

    size_t expr_size = mc_scroll_expr(instrgen->toks, instrgen->pos);
    if (!expr_size) {
        goto exit;
    }
    instrgen->pos += expr_size;
    
    mc_token_t *colon_tok = sct_vector_get(instrgen->toks, instrgen->pos++);
    if (!colon_tok || colon_tok->type != MC_TOK_COLON) {
        micro_push_err((micro_error_t){
            .err = MICRO_ERROR_EXPECTED_COLON,
            .instr = MICRO_INSTR_IF
        });
        goto exit;
    }

    mc_token_t *lbl_tok = sct_vector_get(instrgen->toks, instrgen->pos++);
    if (!lbl_tok || lbl_tok->type != MC_TOK_IDENT) {
        micro_push_err((micro_error_t){
            .err = MICRO_ERROR_EXPECTED_LABEL_NAME,
            .instr = MICRO_INSTR_IF
        });
        goto exit;
    }

    micro_instruction_if_t instr_if;
    instr_if.cond_expr = (micro_expr_tok_t*)expr_tok;
    strcpy(instr_if.lbl_name, lbl_tok->val);
    
    sct_vector_push(&instrgen->instructions, &(micro_instruction_t){
        .type = MICRO_INSTR_IF,
        .if_goto = instr_if,
    });

    mc_token_t *semicolon_tok = sct_vector_get(instrgen->toks, instrgen->pos);
    if (!semicolon_tok || semicolon_tok->type != MC_TOK_SEMICOLON) {
        micro_push_err((micro_error_t){
            .err = MICRO_ERROR_EXPECTED_LABEL_NAME,
            .instr = MICRO_INSTR_IF
        });
    }
    return;

exit:
    while (instrgen->pos < instrgen->toks->size && ((mc_token_t*)sct_vector_get(instrgen->toks, instrgen->pos))->type != MC_TOK_SEMICOLON) {
        instrgen->pos++;
    }
}