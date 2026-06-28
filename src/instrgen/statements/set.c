#include <micro/instrgen/instrgen.h>

void micro_instrgen_parse_set(micro_instrgen_t *instrgen)
{
    micro_token_t *set_tok = sct_vector_get(instrgen->toks, instrgen->pos);
    if (!set_tok || set_tok->type != MICRO_TOK_KW_SET) {
        micro_push_err((micro_error_t){
            .msg = "Expected 'set' keyword",
            .line_ref = set_tok ? set_tok->line_ref : 0,
            .chpos_ref = set_tok ? set_tok->line_ref : 0
        });
        return;
    }

    micro_token_t *type_tok = sct_vector_get(instrgen->toks, instrgen->pos + 1);
    if (!type_tok || type_tok->type != MICRO_TOK_TYPE_NAME) {
        micro_push_err((micro_error_t){
            .msg = "Expected type name",
            .line_ref = type_tok ? type_tok->line_ref : 0,
            .chpos_ref = type_tok ? type_tok->line_ref : 0
        });
        return;
    }

    micro_type_t type = micro_type_str_parse(type_tok->val);
    if (!type) {
        micro_push_err((micro_error_t){
            .msg = "Undefined type name",
            .line_ref = type_tok ? type_tok->line_ref : 0,
            .chpos_ref = type_tok ? type_tok->line_ref : 0
        });
        return;
    }

    micro_token_t *name_tok = sct_vector_get(instrgen->toks, instrgen->pos + 2);
    if (!name_tok || name_tok->type != MICRO_TOK_IDENT) {
        micro_push_err((micro_error_t){
            .msg = "Expected virtual register name",
            .line_ref = name_tok ? name_tok->line_ref : 0,
            .chpos_ref = name_tok ? name_tok->line_ref : 0
        });
        return;
    }

    micro_token_t *expr_start_tok = sct_vector_get(instrgen->toks, instrgen->pos + 3);
    if (!expr_start_tok || !_micro_tok_is_expr_start(expr_start_tok->type)) {
        micro_push_err((micro_error_t){
            .msg = "Expected expression",
            .line_ref = name_tok ? name_tok->line_ref : 0,
            .chpos_ref = name_tok ? name_tok->line_ref : 0
        });
        return;
    }

    micro_instruction_set_t set_instr;
    set_instr.type = type;
    strcpy(set_instr.reg_name, name_tok->val);
    set_instr.val_expr = expr_start_tok;

    micro_instruction_t instr = {
        .type = MICRO_INSTR_SET,
        .set = set_instr
    };
    sct_vector_push(&instrgen->instructions, &instr);

    while (instrgen->pos < instrgen->toks->size && ((micro_token_t*)sct_vector_get(instrgen->toks, instrgen->pos))->type != MICRO_TOK_SEMICOLON) {
        instrgen->pos++;
    }
}
