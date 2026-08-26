#include <microc/instrgen.h>

void mc_instrgen_parse_goto(mc_instrgen_t *instrgen)
{
    mc_token_t *goto_tok = sct_vector_get(instrgen->toks, instrgen->pos++);
    if (unlikely(!goto_tok || goto_tok->type != MC_TOK_KW_GOTO)) {
        micro_push_err((micro_error_t){
            .err = MICRO_ERROR_EXPECTED_GOTO_KW,
            .line_ref = goto_tok ? goto_tok->line_ref : 0,
            .chpos_ref = goto_tok ? goto_tok->chpos_ref : 0
        });
        goto exit;
    }

    if (!instrgen->code_in_function) {
        micro_push_err((micro_error_t){
            .err = MICRO_ERROR_GOTO_OUTSIDE_FUNCTION,
            .line_ref = goto_tok ? goto_tok->line_ref : 0,
            .chpos_ref = goto_tok ? goto_tok->chpos_ref : 0
        });
        goto exit;
    }

    mc_token_t *lbl_tok = sct_vector_get(instrgen->toks, instrgen->pos++);
    if (!lbl_tok || lbl_tok->type != MC_TOK_IDENT) {
        micro_push_err((micro_error_t){
            .err = MICRO_ERROR_EXPECTED_LABEL_NAME,
            .line_ref = lbl_tok ? lbl_tok->line_ref : 0,
            .chpos_ref = lbl_tok ? lbl_tok->chpos_ref : 0
        });
        goto exit;
    }

    micro_instruction_goto_t goto_instr;
    strcpy(goto_instr.lbl, lbl_tok->val);
    sct_vector_push(&instrgen->instructions, &(micro_instruction_t){
        .type = MICRO_INSTR_GOTO,
        .start_tok = (micro_expr_tok_t*)goto_tok,
        .goto_lbl = goto_instr
    });

exit:
    while (instrgen->pos < instrgen->toks->size && ((mc_token_t*)sct_vector_get(instrgen->toks, instrgen->pos))->type != MC_TOK_SEMICOLON) {
        instrgen->pos++;
    }
}