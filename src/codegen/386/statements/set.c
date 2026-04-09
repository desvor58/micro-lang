#include <micro/codegen/386/statements.h>

void micro_codegen_386__set(micro_codegen_t *codegen)
{
    micro_token_t tok_dst = __micro_peek(codegen, 1);
    micro_token_t tok_src = __micro_peek(codegen, 2);

    if (tok_dst.type != MICRO_TT_IDENT || tok_dst.type == MICRO_TT_NULL) {
        micro_push_err((micro_error_t){
            .msg = "Expected destination variable name after 'set' keyword",
            .line_ref = tok_dst.line_ref,
            .chpos_ref = tok_dst.chpos_ref
        });
        goto err_exit;
    }
    if ((tok_src.type != MICRO_TT_IDENT && !micro_tokislit(tok_src) && !micro_tokisop(tok_src)) || tok_src.type == MICRO_TT_NULL) {
        micro_push_err((micro_error_t){
            .msg = "Expected source expression after destination variable name",
            .line_ref = tok_src.line_ref,
            .chpos_ref = tok_src.chpos_ref
        });
        goto err_exit;
    }
    
    micro_codegen_386_ident_info_t *ident_info = sct_hashmap_get(get_codegen_386_ext(codegen)->idents, tok_dst.val);
    if (!ident_info) {
        micro_push_err((micro_error_t){
            .msg = "Undeclareted variable name",
            .line_ref = tok_dst.line_ref,
            .chpos_ref = tok_dst.chpos_ref
        });
        goto err_exit;
    }
    if (ident_info->type != MICRO_IT_VAR) {
        micro_push_err((micro_error_t){
            .msg = "Expected variable",
            .line_ref = tok_dst.line_ref,
            .chpos_ref = tok_dst.chpos_ref
        });
        goto err_exit;
    }

    if (micro_gettype(codegen, tok_src, ident_info->var_info.type) == MICRO_MT_NULL) {
        micro_push_err((micro_error_t){
            .msg = "Wrong type of source expression",
            .line_ref = tok_dst.line_ref,
            .chpos_ref = tok_dst.chpos_ref
        });
        goto err_exit;
    }

    int expr_end_offset = micro_codegen_386_expr_parse(codegen, codegen->toks_pos + 2, ident_info->var_info.storage_info);
    if (!expr_end_offset) {
        goto err_exit;
    }

    if (codegen->toks->toks[codegen->toks_pos + 2 + expr_end_offset].type != MICRO_TT_SEMICOLON) {
        micro_push_err((micro_error_t){
            .msg = "Expected ';'",
            .line_ref = tok_dst.line_ref,
            .chpos_ref = tok_dst.chpos_ref
        });
    }
    return;

err_exit:
    while (codegen->toks_pos < codegen->toks->size && codegen->toks->toks[codegen->toks_pos].type != MICRO_TT_SEMICOLON) {
        codegen->toks_pos++;
    }
    return;
}