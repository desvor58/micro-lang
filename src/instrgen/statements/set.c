#include <micro/instrgen.h>

void micro_instrgen_parse_set(micro_instrgen_t *instrgen)
{
    int is_drset = 0;

    micro_token_t *set_tok = sct_vector_get(instrgen->toks, instrgen->pos++);
    if (unlikely(!set_tok || set_tok->type != MICRO_TOK_KW_SET)) {
        micro_push_err((micro_error_t){
            .err = MICRO_ERROR_EXPECTED_SET_KW,
            .line_ref = set_tok ? set_tok->line_ref : 0,
            .chpos_ref = set_tok ? set_tok->chpos_ref : 0
        });
        goto exit;
    }

    if (!instrgen->code_in_function) {
        micro_push_err((micro_error_t){
            .err = MICRO_ERROR_SET_OUTSIDE_FUNCTION,
            .line_ref = set_tok ? set_tok->line_ref : 0,
            .chpos_ref = set_tok ? set_tok->chpos_ref : 0
        });
        goto exit;
    }

    micro_token_t *type_tok = sct_vector_get(instrgen->toks, instrgen->pos++);
    if (!type_tok || type_tok->type != MICRO_TOK_TYPE_NAME) {
        micro_push_err((micro_error_t){
            .err = MICRO_ERROR_EXPECTED_TYPE_NAME,
            .line_ref = type_tok ? type_tok->line_ref : 0,
            .chpos_ref = type_tok ? type_tok->chpos_ref : 0
        });
        goto exit;
    }

    micro_type_t type = micro_type_str_parse(type_tok->val);
    if (!type) {
        micro_push_err((micro_error_t){
            .err = MICRO_ERROR_UNDEFINED_TYPE_NAME,
            .line_ref = type_tok ? type_tok->line_ref : 0,
            .chpos_ref = type_tok ? type_tok->chpos_ref : 0
        });
        goto exit;
    }

    micro_token_t *name_tok = sct_vector_get(instrgen->toks, instrgen->pos++);
    if (name_tok->type == MICRO_TOK_DOLLAR) {
        is_drset = 1;
        name_tok = sct_vector_get(instrgen->toks, instrgen->pos++);
    }

    if (!name_tok || name_tok->type != MICRO_TOK_IDENT) {
        micro_push_err((micro_error_t){
            .err = MICRO_ERROR_EXPECTED_VREG_NAME,
            .line_ref = name_tok ? name_tok->line_ref : 0,
            .chpos_ref = name_tok ? name_tok->chpos_ref : 0
        });
        goto exit;
    }

    micro_token_t *expr_start_tok = sct_vector_get(instrgen->toks, instrgen->pos);
    if (!expr_start_tok || (!_micro_tok_is_expr_start(expr_start_tok->type) && expr_start_tok->type != MICRO_TOK_SEMICOLON)) {
        micro_push_err((micro_error_t){
            .err = MICRO_ERROR_EXPECTED_EXPRESSION,
            .line_ref = expr_start_tok ? expr_start_tok->line_ref : 0,
            .chpos_ref = expr_start_tok ? expr_start_tok->chpos_ref : 0
        });
        goto exit;
    }
    size_t expr_size = 0;
    if (expr_start_tok->type == MICRO_TOK_SEMICOLON) {
        expr_start_tok = 0;
    } else {
        expr_size = micro_scroll_expr(instrgen->toks, instrgen->pos);
        if (!expr_size) {
            goto exit;
        }
    }

    micro_token_t *semicolon_tok = sct_vector_get(instrgen->toks, instrgen->pos + expr_size);
    if (!semicolon_tok || semicolon_tok->type != MICRO_TOK_SEMICOLON) {
        micro_push_err((micro_error_t) {
            .err = MICRO_ERROR_EXPECTED_SEMICOLON,
            .line_ref = semicolon_tok ? semicolon_tok->line_ref : 0,
            .chpos_ref = semicolon_tok ? semicolon_tok->chpos_ref : 0
        });
        goto exit;
    }

    micro_instruction_t instr;
    if (is_drset) {
        micro_instruction_drset_t drset_instr;
        drset_instr.type = type;
        strcpy(drset_instr.reg_name, name_tok->val);
        drset_instr.val_expr = expr_start_tok;

        instr = (micro_instruction_t){
            .type = MICRO_INSTR_DRSET,
            .drset = drset_instr
        };
    } else {
        micro_instruction_set_t set_instr;
        set_instr.type = type;
        strcpy(set_instr.reg_name, name_tok->val);
        set_instr.val_expr = expr_start_tok;

        instr = (micro_instruction_t){
            .type = MICRO_INSTR_SET,
            .start_tok = set_tok,
            .set = set_instr
        };
    }
    sct_vector_push(&instrgen->instructions, &instr);

exit:
    while (instrgen->pos < instrgen->toks->size && ((micro_token_t*)sct_vector_get(instrgen->toks, instrgen->pos))->type != MICRO_TOK_SEMICOLON) {
        instrgen->pos++;
    }
}
