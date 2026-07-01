#include <micro/instrgen.h>

void micro_instrgen_parse_lbl(micro_instrgen_t *instrgen)
{
    micro_token_t *name_tok = sct_vector_get(instrgen->toks, instrgen->pos++);
    if (!name_tok || name_tok->type != MICRO_TOK_IDENT) {
        micro_push_err((micro_error_t){
            .msg = "Expected ident for label",
            .line_ref = name_tok ? name_tok->line_ref : 0,
            .chpos_ref = name_tok ? name_tok->chpos_ref : 0
        });
        return;
    }
    
    micro_token_t *colon_tok = sct_vector_get(instrgen->toks, instrgen->pos);
    if (!colon_tok || colon_tok->type != MICRO_TOK_COLON) {
        micro_push_err((micro_error_t){
            .msg = "Expected ':'",
            .line_ref = colon_tok ? colon_tok->line_ref : 0,
            .chpos_ref = colon_tok ? colon_tok->chpos_ref : 0
        });
        return;
    }

    micro_instruction_lbl_t lbl;
    strcpy(lbl.name, name_tok->val);

    sct_vector_push(&instrgen->instructions, &(micro_instruction_t){
        .type = MICRO_INSTR_LBL,
        .lbl = lbl
    });
}
