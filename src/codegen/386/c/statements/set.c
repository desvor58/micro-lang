#include "../../statements.h"

void micro_codegen_386__set()
{
    micro_token_t tok_dst = __micro_peek(1);
    micro_token_t tok_src = __micro_peek(2);

    if (tok_dst.type != MICRO_TT_IDENT || tok_dst.type == MICRO_TT_NULL) {
        micro_error_t err = {.msg = "Expected destination variable name after 'set' keyword",
                             .line_ref = tok_dst.line_ref,
                             .chpos_ref = tok_dst.chpos_ref};
        __micro_push_err(err);
        goto err_exit;
    }
    if ((tok_src.type != MICRO_TT_IDENT && !micro_tokislit(tok_src) && !micro_tokisop(tok_src)) || tok_src.type == MICRO_TT_NULL) {
        printf("%u\n", tok_src.type);
        micro_error_t err = {.msg = "Expected source expression after destination variable name",
                             .line_ref = tok_src.line_ref,
                             .chpos_ref = tok_src.chpos_ref};
        __micro_push_err(err);
        goto err_exit;
    }
    
    micro_codegen_386_ident_info_t *ident_info = sct_hashmap_get(micro_codegen_386_idents, tok_dst.val);
    if (!ident_info) {
        micro_error_t err = {.msg = "Undeclareted variable name",
                             .line_ref = tok_dst.line_ref,
                             .chpos_ref = tok_dst.chpos_ref};
        __micro_push_err(err);
        goto err_exit;
    }
    if (ident_info->type != IT_VAR) {
        micro_error_t err = {.msg = "Expected variable",
                             .line_ref = tok_dst.line_ref,
                             .chpos_ref = tok_dst.chpos_ref};
        __micro_push_err(err);
        goto err_exit;
    }

    if (micro_gettype(tok_src, ident_info->var_info.type) == MICRO_MT_NULL) {
        micro_error_t err = {.msg = "Wrong type of sourse expression",
                             .line_ref = tok_src.line_ref,
                             .chpos_ref = tok_src.chpos_ref};
        __micro_push_err(err);
        goto err_exit;
    }

    //__micro_dbg_print_vars();
    int expr_end_offset = micro_codegen_386_expr_parse(micro_pos + 2, ident_info->var_info.storage_info);
    if (!expr_end_offset) {
        goto err_exit;
    }

    if (micro_toks[micro_pos + 2 + expr_end_offset].type != MICRO_TT_SEMICOLON) {
        micro_error_t err = {.msg = "Expected ';'",
                             .line_ref = tok_src.line_ref,
                             .chpos_ref = tok_src.chpos_ref};
        __micro_push_err(err);
    }
    return;

err_exit:
    while (micro_pos < micro_toks_size && micro_toks[micro_pos].type != MICRO_TT_SEMICOLON) {
        micro_pos++;
    }
    return;
}