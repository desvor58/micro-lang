#include "../../statements.h"

void micro_codegen_386__var(micro_codegen_t *codegen)
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
    var_info.storage_info = (micro_codegen_386_storage_info_t) {
        .type = MICRO_ST_STACK,
        .size = micro_mt_size[var_info.type],
        .offset = get_codegen_386_ext(codegen)->top_stack_offset
    };
    micro_codegen_386_ident_info_t *ident_info = malloc(sizeof(micro_codegen_386_ident_info_t));
    ident_info->type = IT_VAR;
    ident_info->var_info = var_info;

    get_codegen_386_ext(codegen)->top_stack_offset -= micro_mt_size[var_info.type];

    micro_codegen_386_ident_info_t *old_ident_info = sct_hashmap_set(get_codegen_386_ext(codegen)->idents, var_info.name, ident_info);
    if (old_ident_info) {
        micro_push_err((micro_error_t){
            .msg = "Double declaration of variable",
            .line_ref = tok_name.line_ref,
            .chpos_ref = tok_name.chpos_ref
        });
        goto err_exit;
    }

    sct_string_t *str_name = sct_string_create();
    sct_string_cat(str_name, "%s", var_info.name);
    sct_list_push_back(get_codegen_386_ext(codegen)->local_var_list, str_name);

    if (__micro_peek(codegen, 3).type == MICRO_TT_SEMICOLON) {
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

        void (*instr_tbl[])(micro_addr_le_t, micro_imm_le_t) = {
            [MICRO_SZ_8]  = asm386_movS32I8,
            [MICRO_SZ_16] = asm386_movS32I16,
            [MICRO_SZ_32] = asm386_movS32I32,
        };
        instr_tbl[micro_mt_size[lit_type]](micro_imm_le_gen(var_info.storage_info.offset),
                                           micro_imm_le_gen(strtoll(__micro_peek(codegen, 3).val, (char**)0, 10)));
        asm_put_instructions(codegen);

        codegen->toks_pos += 4;
    }
    return;

err_exit:
    while (codegen->toks_pos < codegen->toks->size && codegen->toks->toks[codegen->toks_pos].type != MICRO_TT_SEMICOLON) {
        codegen->toks_pos++;
    }
    return;
}