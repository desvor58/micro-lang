#ifndef MICRO_CODEGEN_386_STATEMENTS_H
#define MICRO_CODEGEN_386_STATEMENTS_H

#include "common.h"
#include "expr.h"

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

void micro_codegen_386__call()
{
    if (!micro_code_in_function) {
        micro_error_t err = {.msg = "Instruction 'call' can be usage only in function body",
                             .line_ref = micro_toks[micro_pos].line_ref,
                             .chpos_ref = micro_toks[micro_pos].chpos_ref};
        __micro_push_err(err);
        goto err_exit;
    }

    micro_token_t tok_dst_var_name = __micro_get(1);
    micro_token_t tok_fun_name     = __micro_get(1);
    micro_pos++;

    if (tok_dst_var_name.type != MICRO_TT_IDENT || tok_dst_var_name.type == MICRO_TT_NULL) {
        micro_error_t err = {.msg = "Expected destination variable name after 'call' keyword",
                             .line_ref = micro_toks[micro_pos].line_ref,
                             .chpos_ref = micro_toks[micro_pos].chpos_ref};
        __micro_push_err(err);
        goto err_exit;
    }
    if (tok_fun_name.type != MICRO_TT_IDENT || tok_fun_name.type == MICRO_TT_NULL) {
        micro_error_t err = {.msg = "Expected called functon name after destination variable name",
                             .line_ref = micro_toks[micro_pos].line_ref,
                             .chpos_ref = micro_toks[micro_pos].chpos_ref};
        __micro_push_err(err);
        goto err_exit;
    }

    micro_codegen_386_ident_info_t *fun_ident_info = sct_hashmap_get(micro_codegen_386_idents, tok_fun_name.val);
    if (!fun_ident_info) {
        micro_error_t err = {.msg = "Unknown function name",
                             .line_ref = tok_fun_name.line_ref,
                             .chpos_ref = tok_fun_name.chpos_ref};
        __micro_push_err(err);
        goto err_exit;
    }
    if (fun_ident_info->type != IT_FUN) {
        micro_error_t err = {.msg = "Expected function name",
                             .line_ref = tok_fun_name.line_ref,
                             .chpos_ref = tok_fun_name.chpos_ref};
        __micro_push_err(err);
        goto err_exit;
    }

    size_t arg_expr_starts[16] = {0};
    size_t cur_arg_num = 0;
    while (micro_toks[micro_pos].type != MICRO_TT_SEMICOLON) {
        arg_expr_starts[cur_arg_num++] = micro_pos;
        micro_pos += micro_expr_peek(micro_pos) - 1;

        if (++micro_pos >= micro_toks_size) {
            micro_error_t err = {.msg = "Expected ';'",
                                 .line_ref = micro_toks[micro_pos - 1].line_ref,
                                 .chpos_ref = micro_toks[micro_pos - 1].chpos_ref};
            __micro_push_err(err);
            goto err_exit;
        }
    }
    for (offset_t i = cur_arg_num - 1, arg_offset = 0; i >= 0; i--,arg_offset++) {
        micro_codegen_386_expr_parse(arg_expr_starts[i], (micro_codegen_386_storage_info_t){
            .type = MICRO_ST_STACK,
            .size = 4,
            .offset = micro_top_stack_offset - (arg_offset * 4),
            .is_unsigned = 0
        });
    }


    u8 addr[4];
    // вызов процедур по аддресу: адрес процедуры - (адрес текущей иструкции + размер иструкции call)
    micro_gen32imm_le(addr, fun_ident_info->fun_info.offset - (micro_outbuf->size + 5));
    u8 instruction[] = asm_call(addr);
    push_instruction(instruction);

    if (strcmp(tok_dst_var_name.val, "_")) {
        micro_codegen_386_ident_info_t *dst_var_ident_info = sct_hashmap_get(micro_codegen_386_idents, tok_dst_var_name.val);
        if (!dst_var_ident_info) {
            micro_error_t err = {.msg = "Undeclareted variable name",
                                .line_ref = tok_dst_var_name.line_ref,
                                .chpos_ref = tok_dst_var_name.chpos_ref};
            __micro_push_err(err);
            goto err_exit;
        }
        if (dst_var_ident_info->type != IT_VAR) {
            micro_error_t err = {.msg = "Expected variable name",
                                .line_ref = tok_dst_var_name.line_ref,
                                .chpos_ref = tok_dst_var_name.chpos_ref};
            __micro_push_err(err);
            goto err_exit;
        }

        if (dst_var_ident_info->var_info.type != fun_ident_info->fun_info.ret_type) {
            micro_error_t err = {.msg = "Wrong return type of function for this variable",
                                .line_ref = tok_dst_var_name.line_ref,
                                .chpos_ref = tok_dst_var_name.chpos_ref};
            __micro_push_err(err);
            goto err_exit;
        }

        if (dst_var_ident_info->var_info.storage_info.type == MICRO_ST_DATASEG) {
            u8 addr[4];
            micro_gen32imm_le(addr, dst_var_ident_info->var_info.storage_info.offset);

            if (dst_var_ident_info->var_info.storage_info.size == 4) {
                u8 instruction[] = asm_movM32R32(addr, REG32_EAX);
                push_instruction(instruction);
            } else
            if (dst_var_ident_info->var_info.storage_info.size == 2) {
                u8 instruction[] = asm_movM16R16(addr, REG16_AX);
                push_instruction(instruction);
            } else
            if (dst_var_ident_info->var_info.storage_info.size == 1) {
                u8 instruction[] = asm_movM8R8(addr, REG8_AL);
                push_instruction(instruction);
            }
        }
    }
    return;

err_exit:
    while (micro_pos < micro_toks_size && micro_toks[micro_pos].type != MICRO_TT_SEMICOLON) {
        micro_pos++;
    }
    return;
}

void micro_codegen_386__goto()
{
    micro_goto_ref_t *ref = malloc(sizeof(micro_goto_ref_t));
    ref->code_ref = micro_pos;
    ref->outbuf_ref = micro_outbuf->size;
    sct_list_push_back(micro_goto_refs, ref);
    for (size_t i = 0; i < 5; i++) {
        sct_string_push_back(micro_outbuf, 0);
    }
}

#endif