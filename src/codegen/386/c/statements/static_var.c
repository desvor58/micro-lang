#include "../../statements.h"

void micro_codegen_386__static_var()
{
    micro_token_t tok_type = __micro_peek(1);
    micro_token_t tok_name = __micro_peek(2);

    if (tok_type.type != MICRO_TT_TYPE_NAME || tok_type.type == MICRO_TT_NULL) {
        micro_error_t err = {.msg = "Expected type name after 'var' keyword", .line_ref = tok_type.line_ref, .chpos_ref = tok_type.chpos_ref};
        __micro_push_err(err);
        goto err_exit;
    }
    if (tok_name.type != MICRO_TT_IDENT || tok_name.type == MICRO_TT_NULL) {
        micro_error_t err = {.msg = "Expected identifier after type name", .line_ref = tok_name.line_ref, .chpos_ref = tok_name.chpos_ref};
        __micro_push_err(err);
        goto err_exit;
    }

    micro_codegen_386_var_info_t var_info = {0};
    strcpy(var_info.name, tok_name.val);
    var_info.type = micro_str2mt(tok_type.val);
    var_info.storage_info = (micro_codegen_386_storage_info_t){
        .type = MICRO_ST_DATASEG,
        .size = micro_mt_size[var_info.type],
        .offset = micro_outbuf->size,
        .is_unsigned = micro_mtisunsigned(var_info.type)
    };
    micro_codegen_386_ident_info_t *ident_info = malloc(sizeof(micro_codegen_386_ident_info_t));
    ident_info->type = IT_VAR;
    ident_info->var_info = var_info;

    micro_codegen_386_ident_info_t *old_ident_info = sct_hashmap_set(micro_codegen_386_idents, var_info.name, ident_info);
    if (old_ident_info) {
        micro_error_t err = {.msg = "Double decralation of variable",
                             .line_ref = tok_name.line_ref,
                             .chpos_ref = tok_name.chpos_ref};
        __micro_push_err(err);
        goto err_exit;
    }

    if (__micro_peek(3).type == MICRO_TT_SEMICOLON) {
        for (size_t i = 0; i < micro_mt_size[var_info.type]; i++) {
            sct_string_push_back(micro_outbuf, 0);
        }
        micro_pos += 3;
        return;
    }
    if (micro_tokislit(__micro_peek(3))) {
        if (__micro_peek(4).type != MICRO_TT_SEMICOLON) {
            micro_error_t err = {.msg = "Expected ';'",
                                 .line_ref = tok_name.line_ref,
                                 .chpos_ref = tok_name.chpos_ref};
            __micro_push_err(err);
            micro_pos += 3;
            return;
        }

        micro_codegen_386_micro_type lit_type = micro_lit2mt(__micro_peek(3), var_info.type);
        if (lit_type == MICRO_MT_NULL) {
            micro_error_t err = {.msg = "Wrong literal type",
                                 .line_ref = tok_name.line_ref,
                                 .chpos_ref = tok_name.chpos_ref};
            __micro_push_err(err);
            while (micro_toks[micro_pos].type != MICRO_TT_SEMICOLON && micro_pos < micro_toks_size) {
                micro_pos++;
            }
            return;
        }
        
        u8 *imm_val = (u8*)alloca(sizeof(u8) * micro_mt_size[lit_type]);
        micro_imm_from_mt(imm_val, lit_type, strtoll(__micro_peek(3).val, (char**)0, 10));
        for (size_t i = 0; i < micro_mt_size[lit_type]; i++) {
            sct_string_push_back(micro_outbuf, imm_val[i]);
        }
        micro_pos += 4;
    }
    return;

err_exit:
    while (micro_pos < micro_toks_size && micro_toks[micro_pos].type != MICRO_TT_SEMICOLON) {
        micro_pos++;
    }
    return;
}