#include <microc/instrgen.h>

void mc_instrgen_parse_set(mc_instrgen_t *instrgen)
{
    int is_drset = 0;

    mc_token_t *set_tok = sct_vector_get(instrgen->toks, instrgen->pos++);
    if (unlikely(!set_tok || set_tok->type != MC_TOK_KW_SET)) {
        micro_push_err((micro_error_t){
            .err = MICRO_ERROR_EXPECTED_SET_KW,
            .instr = MICRO_INSTR_SET
        });
        goto exit;
    }

    if (!instrgen->code_in_function) {
        micro_push_err((micro_error_t){
            .err = MICRO_ERROR_SET_OUTSIDE_FUNCTION,
            .instr = MICRO_INSTR_SET
        });
        goto exit;
    }

    mc_token_t *type_tok = sct_vector_get(instrgen->toks, instrgen->pos++);
    if (!type_tok || type_tok->type != MC_TOK_TYPE_NAME) {
        micro_push_err((micro_error_t){
            .err = MICRO_ERROR_EXPECTED_TYPE_NAME,
            .instr = MICRO_INSTR_SET
        });
        goto exit;
    }

    micro_type_t type = mc_type_str_parse(type_tok->val);
    if (!type) {
        micro_push_err((micro_error_t){
            .err = MICRO_ERROR_UNDEFINED_TYPE_NAME,
            .instr = MICRO_INSTR_SET
        });
        goto exit;
    }

    mc_token_t *name_tok = sct_vector_get(instrgen->toks, instrgen->pos++);
    if (name_tok->type == MC_TOK_DOLLAR) {
        is_drset = 1;
        name_tok = sct_vector_get(instrgen->toks, instrgen->pos++);
    }

    if (!name_tok || name_tok->type != MC_TOK_IDENT) {
        micro_push_err((micro_error_t){
            .err = MICRO_ERROR_EXPECTED_VREG_NAME,
            .instr = MICRO_INSTR_SET
        });
        goto exit;
    }

    mc_token_t *expr_start_tok = sct_vector_get(instrgen->toks, instrgen->pos);
    if (!expr_start_tok || (!_micro_expr_is_expr_start(expr_start_tok->type) && expr_start_tok->type != MC_TOK_SEMICOLON)) {
        micro_push_err((micro_error_t){
            .err = MICRO_ERROR_EXPECTED_EXPRESSION,
            .instr = MICRO_INSTR_SET
        });
        goto exit;
    }
    size_t expr_size = 0;
    if (expr_start_tok->type == MC_TOK_SEMICOLON) {
        expr_start_tok = 0;
    } else {
        expr_size = mc_scroll_expr(instrgen->toks, instrgen->pos);
        if (!expr_size) {
            goto exit;
        }
    }

    mc_token_t *semicolon_tok = sct_vector_get(instrgen->toks, instrgen->pos + expr_size);
    if (!semicolon_tok || semicolon_tok->type != MC_TOK_SEMICOLON) {
        micro_push_err((micro_error_t) {
            .err = MICRO_ERROR_EXPECTED_SEMICOLON,
            .instr = MICRO_INSTR_SET
        });
        goto exit;
    }

    micro_instruction_t instr;
    if (is_drset) {
        micro_instruction_drset_t drset_instr;
        drset_instr.type = type;
        strcpy(drset_instr.reg_name, name_tok->val);
        drset_instr.val_expr = (micro_expr_tok_t*)expr_start_tok;

        instr = (micro_instruction_t){
            .type = MICRO_INSTR_DRSET,
            .drset = drset_instr
        };
    } else {
        micro_instruction_set_t set_instr;
        set_instr.type = type;
        strcpy(set_instr.reg_name, name_tok->val);
        set_instr.val_expr = (micro_expr_tok_t*)expr_start_tok;

        instr = (micro_instruction_t){
            .type = MICRO_INSTR_SET,
            .set = set_instr
        };
    }
    sct_vector_push(&instrgen->instructions, &instr);

exit:
    while (instrgen->pos < instrgen->toks->size && ((mc_token_t*)sct_vector_get(instrgen->toks, instrgen->pos))->type != MC_TOK_SEMICOLON) {
        instrgen->pos++;
    }
}
