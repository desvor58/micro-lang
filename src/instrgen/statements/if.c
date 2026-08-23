#include <micro/instrgen.h>

void micro_instrgen_parse_if(micro_instrgen_t *instrgen)
{
    micro_token_t *if_tok = sct_vector_get(instrgen->toks, instrgen->pos++);
    if (unlikely(!if_tok || if_tok->type != MICRO_TOK_KW_IF)) {
        micro_push_err((micro_error_t){
            .err = MICRO_ERROR_EXPECTED_IF_KW,
            .line_ref = if_tok ? if_tok->line_ref : 0,
            .chpos_ref = if_tok ? if_tok->chpos_ref : 0
        });
        goto exit;
    }
    
    micro_token_t *expr_tok = sct_vector_get(instrgen->toks, instrgen->pos);
    if (!expr_tok || !_micro_tok_is_expr_start(expr_tok->type)) {
        micro_push_err((micro_error_t){
            .err = MICRO_ERROR_EXPECTED_EXPRESSION,
            .line_ref = expr_tok ? expr_tok->line_ref : 0,
            .chpos_ref = expr_tok ? expr_tok->chpos_ref : 0
        });
        goto exit;
    }

    size_t expr_size = micro_scroll_expr(instrgen->toks, instrgen->pos);
    if (!expr_size) {
        goto exit;
    }
    instrgen->pos += expr_size;
    
    micro_token_t *colon_tok = sct_vector_get(instrgen->toks, instrgen->pos++);
    if (!colon_tok || colon_tok->type != MICRO_TOK_COLON) {
        micro_push_err((micro_error_t){
            .err = MICRO_ERROR_EXPECTED_COLON,
            .line_ref = colon_tok ? colon_tok->line_ref : 0,
            .chpos_ref = colon_tok ? colon_tok->chpos_ref : 0
        });
        goto exit;
    }

    micro_token_t *lbl_tok = sct_vector_get(instrgen->toks, instrgen->pos++);
    if (!lbl_tok || lbl_tok->type != MICRO_TOK_IDENT) {
        micro_push_err((micro_error_t){
            .err = MICRO_ERROR_EXPECTED_LABEL_NAME,
            .line_ref = lbl_tok ? lbl_tok->line_ref : 0,
            .chpos_ref = lbl_tok ? lbl_tok->chpos_ref : 0
        });
        goto exit;
    }

    micro_instruction_if_t instr_if;
    instr_if.cond_expr = expr_tok;
    strcpy(instr_if.lbl_name, lbl_tok->val);
    
    sct_vector_push(&instrgen->instructions, &(micro_instruction_t){
        .type = MICRO_INSTR_IF,
        .start_tok = if_tok,
        .if_goto = instr_if,
    });

    micro_token_t *semicolon_tok = sct_vector_get(instrgen->toks, instrgen->pos);
    if (!semicolon_tok || semicolon_tok->type != MICRO_TOK_SEMICOLON) {
        micro_push_err((micro_error_t){
            .err = MICRO_ERROR_EXPECTED_LABEL_NAME,
            .line_ref = semicolon_tok ? semicolon_tok->line_ref : 0,
            .chpos_ref = semicolon_tok ? semicolon_tok->chpos_ref : 0
        });
    }
    return;

exit:
    while (instrgen->pos < instrgen->toks->size && ((micro_token_t*)sct_vector_get(instrgen->toks, instrgen->pos))->type != MICRO_TOK_SEMICOLON) {
        instrgen->pos++;
    }
}