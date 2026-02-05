#include "../../statements.h"

void micro_codegen_386__fun()
{
    micro_token_t tok_ident = __micro_get(1);
    micro_pos++;
    if (tok_ident.type != MICRO_TT_IDENT || tok_ident.type == MICRO_TT_NULL) {
        micro_error_t err = {.msg = "Expected function name after 'fun' keyword",
                             .line_ref = tok_ident.line_ref,
                             .chpos_ref = tok_ident.chpos_ref};
        __micro_push_err(err);
        goto err_exit;
    }

    micro_codegen_386_fun_info_t fun_info = {0};
    strcpy(fun_info.name, tok_ident.val);
    fun_info.args = sct_vector_create(1);
    fun_info.ret_type = MICRO_MT_NULL;
    fun_info.offset = micro_outbuf->size;

    size_t param_num = 0;
    while (micro_toks[micro_pos].type != MICRO_TT_KW_RET && micro_toks[micro_pos].type != MICRO_TT_KW_START) {
        micro_token_t tok_param_type = __micro_peek(0);
        micro_token_t tok_param_ident = __micro_peek(1);
        micro_pos += 2;
        if (tok_param_type.type != MICRO_TT_TYPE_NAME || tok_param_type.type == MICRO_TT_NULL) {
            micro_error_t err = {.msg = "Expected parameter type",
                                 .line_ref = tok_param_type.line_ref,
                                 .chpos_ref = tok_param_type.chpos_ref};
            __micro_push_err(err);
            goto err_exit;
        }
        if (tok_param_ident.type != MICRO_TT_IDENT || tok_param_ident.type == MICRO_TT_NULL) {
            micro_error_t err = {.msg = "Expected parameter name",
                                 .line_ref = tok_param_ident.line_ref,
                                 .chpos_ref = tok_param_ident.chpos_ref};
            __micro_push_err(err);
            goto err_exit;
        }

        micro_codegen_386_var_info_t param_info = {0};
        strcpy(param_info.name, tok_param_ident.val);
        param_info.type = micro_str2mt(tok_param_type.val);
        param_info.storage_info = (micro_codegen_386_storage_info_t) {
            .type = MICRO_ST_STACK,
            .size = micro_mt_size[param_info.type],
            .offset = + 4 + (param_num * 4 + micro_mt_size[param_info.type]),
            .is_unsigned = micro_mtisunsigned(param_info.type)
        };

        micro_codegen_386_ident_info_t *ident_info = malloc(sizeof(micro_codegen_386_ident_info_t));
        ident_info->type = IT_VAR;
        ident_info->var_info = param_info;

        sct_string_t *str_name = sct_string_create();
        sct_string_cat(str_name, "%s", param_info.name);
        sct_list_push_back(micro_codegen_386_local_vars_list, str_name);
        
        // vector_micro_codegen_386_var_info_t_push_back(fun_info->args, *param_info);
        micro_codegen_386_ident_info_t *old_ident_info = sct_hashmap_set(micro_codegen_386_idents, tok_param_ident.val, ident_info);
        if (!old_ident_info) {
            micro_error_t err = {.msg = "Double decralation of parametr",
                                .line_ref = tok_param_ident.line_ref,
                                .chpos_ref = tok_param_ident.chpos_ref};
            __micro_push_err(err);
            goto err_exit;
        }
        param_num++;
    }
    
    if (micro_toks[micro_pos].type == MICRO_TT_KW_RET) {
        if (micro_toks[++micro_pos].type != MICRO_TT_TYPE_NAME) {
            micro_error_t err = {.msg = "Expected return type of function after 'ret' keyword",
                                 .line_ref = micro_toks[micro_pos].line_ref,
                                 .chpos_ref = micro_toks[micro_pos].chpos_ref};
            __micro_push_err(err);
            goto err_exit;
        }
        fun_info.ret_type = micro_str2mt(micro_toks[micro_pos].val);
        micro_pos++;
    }
    if (micro_toks[micro_pos].type != MICRO_TT_KW_START) {
        micro_error_t err = {.msg = "Expected 'start' keyword",
                             .line_ref = micro_toks[micro_pos].line_ref,
                             .chpos_ref = micro_toks[micro_pos].chpos_ref};
        __micro_push_err(err);
        goto err_exit;
    }

    u8 prelude_instruction[] = asm_std_prelude;
    push_instruction(prelude_instruction);

    // puts("fun start vars map:");
    // __micro_dbg_print_vars();
    // printf("END\n");
    // puts("fun start vars list:");
    // foreach (var_info_it, micro_codegen_386_local_vars_list) {
    //     printf("%s\n", ((sct_string_t*)var_info_it->val)->str);
    // }
    // printf("END\n");

    micro_code_in_function = 1;
    micro_top_stack_offset = -4;
    while (micro_toks[++micro_pos].type != MICRO_TT_KW_END) {
        micro_codegen_386_micro_instruction_parse();
    }
    size_t i = 0;
    size_t micro_pos_acc = micro_pos;
    foreach (ref_it, micro_goto_refs) {
        size_t micro_pos_save = micro_pos;
        micro_pos = ((micro_goto_ref_t*)ref_it->val)->code_ref;

        micro_token_t tok_lbl = __micro_peek(1);
        if (tok_lbl.type != MICRO_TT_IDENT) {
            micro_error_t err = {.msg = "Expected label name",
                                .line_ref = tok_lbl.line_ref,
                                .chpos_ref = tok_lbl.chpos_ref};
            __micro_push_err(err);
            goto err_exit;
        }

        micro_codegen_386_ident_info_t *ident_info = sct_hashmap_get(micro_codegen_386_idents, tok_lbl.val);
        if (!ident_info) {
            micro_error_t err = {.msg = "Undeclared label name",
                                .line_ref = tok_lbl.line_ref,
                                .chpos_ref = tok_lbl.chpos_ref};
            __micro_push_err(err);
            goto err_exit;
        }
        if (ident_info->type != IT_LBL) {
            micro_error_t err = {.msg = "Expected label name",
                                .line_ref = tok_lbl.line_ref,
                                .chpos_ref = tok_lbl.chpos_ref};
            __micro_push_err(err);
            goto err_exit;
        }
        
        u8 addr[4];
        micro_gen32imm_le(addr, ident_info->lbl_info.offset - (((micro_goto_ref_t*)ref_it->val)->outbuf_ref + 5));
        u8 instruction[] = asm_jmpL32(addr);
        push_instruction2addr(instruction, ((micro_goto_ref_t*)ref_it->val)->outbuf_ref);
        
        micro_pos = micro_pos_save;
        micro_goto_refs = sct_list_full_delete(micro_goto_refs, i);
        i++;
    }
    micro_pos = micro_pos_acc;
    micro_code_in_function = 0;
    micro_top_stack_offset = 0;

    for (size_t i = 0; i < fun_info.args->size; i++) {
        sct_hashmap_full_delete(micro_codegen_386_idents, ((micro_codegen_386_var_info_t*)fun_info.args->arr[i])->name);
    }

    // __micro_dbg_print_idents();

    // puts("vars->keys before freeing:");
    // foreach (key_it, micro_codegen_386_vars->keys) {
    //     printf_s("  %s\n", key_it->val);
    // }

    foreach (var_info_it, micro_codegen_386_local_vars_list) {
        sct_hashmap_full_delete(micro_codegen_386_idents, ((sct_string_t*)var_info_it->val)->str);
    }
    sct_list_full_free(micro_codegen_386_local_vars_list);
    micro_codegen_386_local_vars_list = sct_list_pair_create(0);

    // puts("vars->keys after freeing:");
    // foreach (key_it, micro_codegen_386_vars->keys) {
    //     printf_s("  %s\n", key_it->val);
    // }

    u8 epilogue_instruction[] = asm_std_epilogue;
    push_instruction(epilogue_instruction);

    micro_codegen_386_ident_info_t *fun_ident = malloc(sizeof(micro_codegen_386_ident_info_t));
    fun_ident->type = IT_FUN;
    fun_ident->fun_info = fun_info;

    micro_codegen_386_ident_info_t *old_ident_info = sct_hashmap_set(micro_codegen_386_idents, tok_ident.val, fun_ident);
    if (old_ident_info) {
        micro_error_t err = {.msg = "Double decralation of function",
                            .line_ref = tok_ident.line_ref,
                            .chpos_ref = tok_ident.chpos_ref};
        __micro_push_err(err);
        goto err_exit;
    }

    return;

err_exit:
    while (micro_pos < micro_toks_size && micro_toks[micro_pos].type != MICRO_TT_KW_END) {
        micro_pos++;
    }
    return;
}