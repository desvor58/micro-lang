#include "../../statements.h"

void micro_codegen_386__fun(micro_codegen_t *codegen)
{
    micro_token_t tok_ident = __micro_get(codegen, 1);
    codegen->toks_pos++;
    if (tok_ident.type != MICRO_TT_IDENT || tok_ident.type == MICRO_TT_NULL) {
        micro_push_err((micro_error_t){
            .msg = "Expected function name after 'fun' keyword",
            .line_ref = tok_ident.line_ref,
            .chpos_ref = tok_ident.chpos_ref
        });
        goto err_exit;
    }

    micro_codegen_386_fun_info_t fun_info = {0};
    strcpy(fun_info.name, tok_ident.val);
    fun_info.args = sct_vector_create(1);
    fun_info.ret_type = MICRO_MT_NULL;
    fun_info.offset = codegen->outbuf->size;

    size_t param_num = 0;
    while (codegen->toks->toks[codegen->toks_pos].type != MICRO_TT_KW_RET && codegen->toks->toks[codegen->toks_pos].type != MICRO_TT_KW_START) {
        micro_token_t tok_param_type = __micro_peek(codegen, 0);
        micro_token_t tok_param_ident = __micro_peek(codegen, 1);
        codegen->toks_pos += 2;
        if (tok_param_type.type != MICRO_TT_TYPE_NAME || tok_param_type.type == MICRO_TT_NULL) {
            micro_push_err((micro_error_t){
                .msg = "Expected parameter type",
                .line_ref = tok_param_type.line_ref,
                .chpos_ref = tok_param_type.chpos_ref
            });
            goto err_exit;
        }
        if (tok_param_ident.type != MICRO_TT_IDENT || tok_param_ident.type == MICRO_TT_NULL) {
            micro_push_err((micro_error_t){
                .msg = "Expected parameter name",
                .line_ref = tok_param_ident.line_ref,
                .chpos_ref = tok_param_ident.chpos_ref
            });
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
        sct_list_push_back(get_codegen_386_ext(codegen)->local_var_list, str_name);
        
        // vector_micro_codegen_386_var_info_t_push_back(fun_info->args, *param_info);
        micro_codegen_386_ident_info_t *old_ident_info = sct_hashmap_set(get_codegen_386_ext(codegen)->idents, tok_param_ident.val, ident_info);
        if (!old_ident_info) {
            micro_push_err((micro_error_t){
                .msg = "Double declaration of parameter",
                .line_ref = tok_param_ident.line_ref,
                .chpos_ref = tok_param_ident.chpos_ref
            });
            goto err_exit;
        }
        param_num++;
    }
    
    if (codegen->toks->toks[codegen->toks_pos].type == MICRO_TT_KW_RET) {
        if (codegen->toks->toks[++codegen->toks_pos].type != MICRO_TT_TYPE_NAME) {
            micro_push_err((micro_error_t){
                .msg = "Expected return type of function after 'ret' keyword",
                .line_ref = codegen->toks->toks[codegen->toks_pos].line_ref,
                .chpos_ref = codegen->toks->toks[codegen->toks_pos].chpos_ref
            });
            goto err_exit;
        }
        fun_info.ret_type = micro_str2mt(codegen->toks->toks[codegen->toks_pos].val);
        codegen->toks_pos++;
    }
    if (codegen->toks->toks[codegen->toks_pos].type != MICRO_TT_KW_START) {
        micro_push_err((micro_error_t){
            .msg = "Expected 'start' keyword",
            .line_ref = codegen->toks->toks[codegen->toks_pos].line_ref,
            .chpos_ref = codegen->toks->toks[codegen->toks_pos].chpos_ref
        });
        goto err_exit;
    }

    asm386_prelude();
    asm_put_instructions(codegen);

    // puts("fun start vars map:");
    // __micro_dbg_print_vars();
    // printf("END\n");
    // puts("fun start vars list:");
    // foreach (var_info_it, micro_codegen_386_local_vars_list) {
    //     printf("%s\n", ((sct_string_t*)var_info_it->val)->str);
    // }
    // printf("END\n");

    get_codegen_386_ext(codegen)->code_in_function = 1;
    get_codegen_386_ext(codegen)->top_stack_offset = -4;
    while (codegen->toks->toks[++codegen->toks_pos].type != MICRO_TT_KW_END) {
        micro_codegen_386_micro_instruction_parse(codegen);
    }
    size_t i = 0;
    size_t micro_pos_acc = codegen->toks_pos;
    foreach (ref_it, get_codegen_386_ext(codegen)->goto_refs) {
        size_t micro_pos_save = codegen->toks_pos;
        codegen->toks_pos = ((micro_goto_ref_t*)ref_it->val)->code_ref;

        micro_token_t tok_lbl = __micro_peek(codegen, 1);
        if (tok_lbl.type != MICRO_TT_IDENT) {
            micro_push_err((micro_error_t){
                .msg = "Expected label name",
                .line_ref = tok_lbl.line_ref,
                .chpos_ref = tok_lbl.chpos_ref
            });
            goto err_exit;
        }

        micro_codegen_386_ident_info_t *ident_info = sct_hashmap_get(get_codegen_386_ext(codegen)->idents, tok_lbl.val);
        if (!ident_info) {
            micro_push_err((micro_error_t){
                .msg = "Undeclared label name",
                .line_ref = tok_lbl.line_ref,
                .chpos_ref = tok_lbl.chpos_ref
            });
            goto err_exit;
        }
        if (ident_info->type != IT_LBL) {
            micro_push_err((micro_error_t){
                .msg = "Expected label name",
                .line_ref = tok_lbl.line_ref,
                .chpos_ref = tok_lbl.chpos_ref
            });
            goto err_exit;
        }
        
        micro_addr_le_t addr = micro_imm_le_gen(ident_info->lbl_info.offset - (((micro_goto_ref_t*)ref_it->val)->outbuf_ref + 5));
        asm386_jmpL32(addr);
        asm_put_instructions_to_addr(codegen, ((micro_goto_ref_t*)ref_it->val)->outbuf_ref);
        
        codegen->toks_pos = micro_pos_save;
        get_codegen_386_ext(codegen)->goto_refs = sct_list_full_delete(get_codegen_386_ext(codegen)->goto_refs, i);
        i++;
    }
    codegen->toks_pos = micro_pos_acc;
    get_codegen_386_ext(codegen)->code_in_function = 0;
    get_codegen_386_ext(codegen)->top_stack_offset = 0;

    for (size_t i = 0; i < fun_info.args->size; i++) {
        sct_hashmap_full_delete(get_codegen_386_ext(codegen)->idents, ((micro_codegen_386_var_info_t*)fun_info.args->arr[i])->name);
    }

    // __micro_dbg_print_idents();

    // puts("vars->keys before freeing:");
    // foreach (key_it, micro_codegen_386_vars->keys) {
    //     printf_s("  %s\n", key_it->val);
    // }

    foreach (var_info_it, get_codegen_386_ext(codegen)->local_var_list) {
        sct_hashmap_full_delete(get_codegen_386_ext(codegen)->idents, ((sct_string_t*)var_info_it->val)->str);
    }
    sct_list_full_free(get_codegen_386_ext(codegen)->local_var_list);
    get_codegen_386_ext(codegen)->local_var_list = sct_list_pair_create(0);

    // puts("vars->keys after freeing:");
    // foreach (key_it, micro_codegen_386_vars->keys) {
    //     printf_s("  %s\n", key_it->val);
    // }

    asm386_epilogue();
    asm_put_instructions(codegen);

    micro_codegen_386_ident_info_t *fun_ident = malloc(sizeof(micro_codegen_386_ident_info_t));
    fun_ident->type = IT_FUN;
    fun_ident->fun_info = fun_info;

    micro_codegen_386_ident_info_t *old_ident_info = sct_hashmap_set(get_codegen_386_ext(codegen)->idents, tok_ident.val, fun_ident);
    if (old_ident_info) {
        micro_push_err((micro_error_t){
            .msg = "Double declaration of function",
            .line_ref = tok_ident.line_ref,
            .chpos_ref = tok_ident.chpos_ref
        });
        goto err_exit;
    }

    return;

err_exit:
    while (codegen->toks_pos < codegen->toks->size && codegen->toks->toks[codegen->toks_pos].type != MICRO_TT_KW_END) {
        codegen->toks_pos++;
    }
    return;
}