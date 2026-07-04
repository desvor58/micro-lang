#include <micro/instrgen.h>

void micro_instrgen_parse_goto(micro_instrgen_t *instrgen)
{
    micro_token_t *goto_tok = sct_vector_get(instrgen->toks, instrgen->pos++);
    if (!goto_tok || goto_tok->type != MICRO_TOK_KW_GOTO) {
        micro_push_err((micro_error_t){
            .msg = "Expected 'call' keyword",
            .line_ref = goto_tok ? goto_tok->line_ref : 0,
            .chpos_ref = goto_tok ? goto_tok->chpos_ref : 0
        });
        goto exit;
    }

    if (!instrgen->code_in_function) {
        micro_push_err((micro_error_t){
            .msg = "'goto' instruction can be only in function body",
            .line_ref = goto_tok ? goto_tok->line_ref : 0,
            .chpos_ref = goto_tok ? goto_tok->chpos_ref : 0
        });
        goto exit;
    }

    micro_token_t *lbl_tok = sct_vector_get(instrgen->toks, instrgen->pos++);
    if (!lbl_tok || lbl_tok->type != MICRO_TOK_IDENT) {
        micro_push_err((micro_error_t){
            .msg = "Expected register name for result or '_'",
            .line_ref = lbl_tok ? lbl_tok->line_ref : 0,
            .chpos_ref = lbl_tok ? lbl_tok->chpos_ref : 0
        });
        goto exit;
    }

    micro_instruction_goto_t goto_instr;
    strcpy(goto_instr.lbl, lbl_tok->val);
    sct_vector_push(&instrgen->instructions, &(micro_instruction_t){
        .type = MICRO_INSTR_GOTO,
        .goto_lbl = goto_instr
    });

exit:
    while (instrgen->pos < instrgen->toks->size && ((micro_token_t*)sct_vector_get(instrgen->toks, instrgen->pos))->type != MICRO_TOK_SEMICOLON) {
        instrgen->pos++;
    }
}