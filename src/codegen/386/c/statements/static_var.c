#include "../../statements.h"

void micro_codegen_386__static_var(micro_codegen_t *codegen)
{
    micro_token_t tok_type = __micro_peek(codegen, 1);
    micro_token_t tok_name = __micro_peek(codegen, 2);

    if (tok_type.type != MICRO_TT_TYPE_NAME || tok_type.type == MICRO_TT_NULL) {
        micro_push_err((micro_error_t){
            .msg = "Expected type name after 'var' keyword",
            .line_ref = tok_type.line_ref,
            .chpos_ref = tok_type.chpos_ref
        });
        goto err_exit;
    }
    if (tok_name.type != MICRO_TT_IDENT || tok_name.type == MICRO_TT_NULL) {
        micro_push_err((micro_error_t){
            .msg = "Expected identifier after type name",
            .line_ref = tok_name.line_ref,
            .chpos_ref = tok_name.chpos_ref
        });
        goto err_exit;
    }

    micro_codegen_386_var_info_t var_info = {0};
    strcpy(var_info.name, tok_name.val);
    var_info.type = micro_str2mt(tok_type.val);
    var_info.storage_info = (micro_codegen_386_storage_info_t){
        .type = MICRO_ST_DATASEG,
        .size = micro_mt_size[var_info.type],
        .offset = codegen->outbuf->size,
        .is_unsigned = micro_mtisunsigned(var_info.type)
    };
    micro_codegen_386_ident_info_t *ident_info = malloc(sizeof(micro_codegen_386_ident_info_t));
    ident_info->type = MICRO_IT_VAR;
    ident_info->var_info = var_info;

    micro_codegen_386_ident_info_t *old_ident_info = sct_hashmap_set(get_codegen_386_ext(codegen)->idents, var_info.name, ident_info);
    if (old_ident_info) {
        micro_push_err((micro_error_t){
            .msg = "Double declaration of variable",
            .line_ref = tok_name.line_ref,
            .chpos_ref = tok_name.chpos_ref
        });
        goto err_exit;
    }

    if (__micro_peek(codegen, 3).type == MICRO_TT_SEMICOLON) {
        for (size_t i = 0; i < micro_sz_real_size[micro_mt_size[var_info.type]]; i++) {
            micro_codegen_outbuf_push(codegen->outbuf, 0);
        }
        codegen->toks_pos += 3;
        return;
    }
    if (micro_tokislit(__micro_peek(codegen, 3))) {
        if (__micro_peek(codegen, 4).type != MICRO_TT_SEMICOLON) {
            micro_push_err((micro_error_t){
                .msg = "Expected ';'",
                .line_ref = tok_name.line_ref,
                .chpos_ref = tok_name.chpos_ref
            });
            codegen->toks_pos += 3;
            return;
        }

        micro_codegen_386_micro_type lit_type = micro_lit2mt(__micro_peek(codegen, 3), var_info.type);
        if (lit_type == MICRO_MT_NULL) {
            micro_push_err((micro_error_t){
                .msg = "Wrong literal type",
                .line_ref = tok_name.line_ref,
                .chpos_ref = tok_name.chpos_ref
            });
            while (codegen->toks->toks[codegen->toks_pos].type != MICRO_TT_SEMICOLON && codegen->toks_pos < codegen->toks->size) {
                codegen->toks_pos++;
            }
            return;
        }
        
        micro_imm_le_t imm_val = micro_imm_le_gen(strtoll(__micro_peek(codegen, 3).val, (char**)0, 10));
        for (size_t i = 0; i < (micro_mt_size[lit_type] == MICRO_SZ_32 ? 4 : micro_mt_size[lit_type] == MICRO_SZ_16 ? 2 : 1); i++) {
            micro_codegen_outbuf_push(codegen->outbuf, imm_val.bytes[i]);
        }
        codegen->toks_pos += 4;
    }
    return;

err_exit:
    while (codegen->toks_pos < codegen->toks->size && codegen->toks->toks[codegen->toks_pos].type != MICRO_TT_SEMICOLON) {
        codegen->toks_pos++;
    }
    return;
}