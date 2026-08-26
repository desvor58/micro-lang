#include <microc/instrgen.h>

void mc_instrgen_parse_lbl(mc_instrgen_t *instrgen)
{
    mc_token_t *name_tok = sct_vector_get(instrgen->toks, instrgen->pos++);
    if (unlikely(!name_tok || name_tok->type != MC_TOK_IDENT)) {
        micro_push_err((micro_error_t){
            .err = MICRO_ERROR_EXPECTED_LABEL_NAME,
            .line_ref = name_tok ? name_tok->line_ref : 0,
            .chpos_ref = name_tok ? name_tok->chpos_ref : 0
        });
        return;
    }

    if (!instrgen->code_in_function) {
        micro_push_err((micro_error_t){
            .err = MICRO_ERROR_LBL_OUTSIDE_FUNCTION,
            .line_ref = name_tok ? name_tok->line_ref : 0,
            .chpos_ref = name_tok ? name_tok->chpos_ref : 0
        });
        return;
    }
    
    mc_token_t *colon_tok = sct_vector_get(instrgen->toks, instrgen->pos);
    if (!colon_tok || colon_tok->type != MC_TOK_COLON) {
        micro_push_err((micro_error_t){
            .err = MICRO_ERROR_EXPECTED_COLON,
            .line_ref = colon_tok ? colon_tok->line_ref : 0,
            .chpos_ref = colon_tok ? colon_tok->chpos_ref : 0
        });
        return;
    }

    micro_instruction_lbl_t lbl;
    strcpy(lbl.name, name_tok->val);

    sct_vector_push(&instrgen->instructions, &(micro_instruction_t){
        .type = MICRO_INSTR_LBL,
        .start_tok = (micro_expr_tok_t*)name_tok,
        .lbl = lbl
    });
}
