#include "../../statements.h"

void micro_codegen_386__var()
{
    micro_token_t tok_type = __micro_peek(1);
    micro_token_t tok_name = __micro_peek(2);
    
    if (tok_type.type != MICRO_TT_TYPE_NAME || tok_type.type == MICRO_TT_NULL) {
        micro_error_t err = {.msg = "Expected type name after 'var' keyword",
                             .line_ref = tok_type.line_ref,
                             .chpos_ref = tok_type.chpos_ref};
        __micro_push_err(err);
        goto err_exit;
    }
    if (tok_name.type != MICRO_TT_IDENT || tok_name.type == MICRO_TT_NULL) {
        micro_error_t err = {.msg = "Expected identifier after type name",
                             .line_ref = tok_name.line_ref,
                             .chpos_ref = tok_name.chpos_ref};
        __micro_push_err(err);
        goto err_exit;
    }

    micro_codegen_386_var_info_t var_info = {0};
    strcpy(var_info.name, tok_name.val);
    var_info.type = micro_str2mt(tok_type.val);
    var_info.storage_info = (micro_codegen_386_storage_info_t) {
        .type = MICRO_ST_STACK,
        .size = micro_mt_size[var_info.type],
        .offset = micro_top_stack_offset
    };
    micro_codegen_386_ident_info_t *ident_info = malloc(sizeof(micro_codegen_386_ident_info_t));
    ident_info->type = IT_VAR;
    ident_info->var_info = var_info;

    micro_top_stack_offset -= micro_mt_size[var_info.type];

    micro_codegen_386_ident_info_t *old_ident_info = sct_hashmap_set(micro_codegen_386_idents, var_info.name, ident_info);
    if (old_ident_info) {
        micro_error_t err = {.msg = "Double decralation of variable",
                             .line_ref = tok_name.line_ref,
                             .chpos_ref = tok_name.chpos_ref};
        __micro_push_err(err);
        goto err_exit;
    }

    sct_string_t *str_name = sct_string_create();
    sct_string_cat(str_name, "%s", var_info.name);
    sct_list_push_back(micro_codegen_386_local_vars_list, str_name);

    if (__micro_peek(3).type == MICRO_TT_SEMICOLON) {
        micro_pos += 3;
        return;
    }
    
    if (micro_tokislit(__micro_peek(3))) {
        if (__micro_peek(4).type != MICRO_TT_SEMICOLON) {
            micro_error_t err = {.msg = "Expected ';'", .line_ref = tok_name.line_ref, .chpos_ref = tok_name.chpos_ref};
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
        
        if (micro_mt_size[lit_type] == 4) {
            u8 offset[4];
            micro_gen32imm_le(offset, var_info.storage_info.offset);

            u8 val[4];
            micro_gen32imm_le(val, strtoll(__micro_peek(3).val, (char**)0, 10));

            u8 instruction[] = asm_movS32I32(offset, val);
            push_instruction(instruction);
        } else
        if (micro_mt_size[lit_type] == 2) {
            u8 offset[4];
            micro_gen32imm_le(offset, var_info.storage_info.offset);

            u8 val[2];
            micro_gen16imm_le(val, strtoll(__micro_peek(3).val, (char**)0, 10));

            u8 instruction[] = asm_movS32I16(offset, val);
            push_instruction(instruction);
        } else
        if (micro_mt_size[lit_type] == 1) {
            u8 offset[4];
            micro_gen32imm_le(offset, var_info.storage_info.offset);

            u8 val[1];
            val[0] = strtoll(__micro_peek(3).val, (char**)0, 10);

            u8 instruction[] = asm_movS32I8(offset, val);
            push_instruction(instruction);
        } else {
            micro_error_t err = {.msg = "Wrong literal type size",
                                 .line_ref = __micro_peek(3).line_ref,
                                 .chpos_ref = __micro_peek(3).chpos_ref};
            __micro_push_err(err);
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