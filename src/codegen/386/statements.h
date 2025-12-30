#ifndef MICRO_CODEGEN_386_STATEMENTS_H
#define MICRO_CODEGEN_386_STATEMENTS_H

#include "common.h"
#include "expr.h"

void micro_codegen_386__static_var()
{
    micro_token_t type_tok = __micro_peek(1);
    micro_token_t ident_tok = __micro_peek(2);

    if (type_tok.type != MICRO_TT_TYPE_NAME || type_tok.type == MICRO_TT_NULL) {
        micro_error_t err = {.msg = "Expected type name after 'var' keyword", .line_ref = type_tok.line_ref, .chpos_ref = type_tok.chpos_ref};
        __micro_push_err(err);
        goto err_exit;
    }
    if (ident_tok.type != MICRO_TT_IDENT || ident_tok.type == MICRO_TT_NULL) {
        micro_error_t err = {.msg = "Expected identifier after type name", .line_ref = ident_tok.line_ref, .chpos_ref = ident_tok.chpos_ref};
        __micro_push_err(err);
        goto err_exit;
    }

    micro_codegen_386_var_info_t *var_info = (micro_codegen_386_var_info_t*)malloc(sizeof(micro_codegen_386_var_info_t));
    strcpy(var_info->name, ident_tok.val);
    var_info->type = micro_str2mt(type_tok.val);
    var_info->storage_info.size = micro_mt_size[var_info->type];
    var_info->storage_info.type = MICRO_ST_DATASEG;
    var_info->storage_info.offset = micro_outbuf->size; // смещение будет такое, как число записаных байт в outbuf
    var_info->storage_info.is_unsigned = micro_mtisunsigned(var_info->type);
    hashmap_micro_codegen_386_var_info_t_set(micro_codegen_386_vars, var_info->name, var_info);

    if (__micro_peek(3).type == MICRO_TT_SEMICOLON) {
        for (size_t i = 0; i < micro_mt_size[var_info->type]; i++) {
            string_push_back(micro_outbuf, 0);
        }
        micro_pos += 3;
        return;
    }
    if (micro_tokislit(__micro_peek(3))) {
        micro_codegen_386_micro_type lit_type = micro_lit2mt(__micro_peek(3), var_info->type);
        if (__micro_peek(4).type != MICRO_TT_SEMICOLON) {
            micro_error_t err = {.msg = "Expected ';'", .line_ref = ident_tok.line_ref, .chpos_ref = ident_tok.chpos_ref};
            __micro_push_err(err);
            micro_pos += 3;
            return;
        }
        if (lit_type == MICRO_MT_NULL) {
            micro_error_t err = {.msg = "Wrong literal type", .line_ref = ident_tok.line_ref, .chpos_ref = ident_tok.chpos_ref};
            __micro_push_err(err);
            while (micro_toks[micro_pos].type != MICRO_TT_SEMICOLON && micro_pos < micro_toks_size) {
                micro_pos++;
            }
            return;
        }
        
        char *imm_val = (char*)malloc(sizeof(char) * micro_mt_size[lit_type]);
        micro_imm_from_mt(imm_val, lit_type, strtoull(__micro_peek(3).val, (char**)0, 10));
        for (size_t i = 0; i < micro_mt_size[lit_type]; i++) {
            string_push_back(micro_outbuf, imm_val[i]);
        }
        micro_pos += 4;
    }
    return;

err_exit:
    while (micro_toks[micro_pos].type != MICRO_TT_SEMICOLON && micro_pos < micro_toks_size) {
        micro_pos++;
    }
    return;
}

void micro_codegen_386__set()
{
    micro_token_t dst_tok = __micro_peek(1);
    micro_token_t src_tok = __micro_peek(2);

    if (dst_tok.type != MICRO_TT_IDENT || dst_tok.type == MICRO_TT_NULL) {
        micro_error_t err = {.msg = "Expected destination variable name after 'set' keyword", .line_ref = dst_tok.line_ref, .chpos_ref = dst_tok.chpos_ref};
        __micro_push_err(err);
        goto err_exit;
    }
    if (src_tok.type != MICRO_TT_IDENT && !micro_tokislit(src_tok) && !micro_tokisop(src_tok) || src_tok.type == MICRO_TT_NULL) {
        printf("%u\n", src_tok.type);
        micro_error_t err = {.msg = "Expected source expression after destination variable name", .line_ref = src_tok.line_ref, .chpos_ref = src_tok.chpos_ref};
        __micro_push_err(err);
        goto err_exit;
    }
    
    micro_codegen_386_var_info_t *var_info = hashmap_micro_codegen_386_var_info_t_get(micro_codegen_386_vars, dst_tok.val);
    if (!var_info) {
        micro_error_t err = {.msg = "Undeclareted variable name", .line_ref = src_tok.line_ref, .chpos_ref = src_tok.chpos_ref};
        __micro_push_err(err);
        return;
    }

    if (micro_gettype(dst_tok, var_info->type) == MICRO_MT_NULL) {
        micro_error_t err = {.msg = "Wrong type of sourse expression", .line_ref = src_tok.line_ref, .chpos_ref = src_tok.chpos_ref};
        __micro_push_err(err);
        return;
    }
    int expr_end_offset = micro_codegen_386_expr_parse(micro_pos + 2, var_info->storage_info);
    if (!expr_end_offset) {
        goto err_exit;
    }
    if (micro_toks[micro_pos + 2 + expr_end_offset].type != MICRO_TT_SEMICOLON) {
        micro_error_t err = {.msg = "Expected ';'", .line_ref = src_tok.line_ref, .chpos_ref = src_tok.chpos_ref};
        __micro_push_err(err);
    }
    return;

err_exit:
    while (micro_toks[micro_pos].type != MICRO_TT_SEMICOLON && micro_pos < micro_toks_size) {
        micro_pos++;
    }
    return;
}

void micro_codegen_386__fun()
{
    micro_token_t tok_ident = __micro_get(1);
    micro_pos++;
    if (tok_ident.type != MICRO_TT_IDENT || tok_ident.type == MICRO_TT_NULL) {
        micro_error_t err = {.msg = "Expected function name after 'fun' keyword", .line_ref = tok_ident.line_ref, .chpos_ref = tok_ident.chpos_ref};
        __micro_push_err(err);
        goto err_exit;
    }

    micro_codegen_386_fun_info_t *fun_info = (micro_codegen_386_fun_info_t*)malloc(sizeof(micro_codegen_386_fun_info_t));
    strcpy(fun_info->name, tok_ident.val);
    fun_info->args = list_micro_codegen_386_var_info_t_create();
    fun_info->ret_type = MICRO_MT_NULL;
    fun_info->offset = micro_outbuf->size;
    printf("fun:%u\n", fun_info->offset);
    
    if (micro_toks[micro_pos].type == MICRO_TT_KW_RET) {
        if (micro_toks[++micro_pos].type != MICRO_TT_TYPE_NAME) {
            micro_error_t err = {.msg = "Expected return type of function after 'ret' keyword",
                                 .line_ref = micro_toks[micro_pos].line_ref,
                                 .chpos_ref = micro_toks[micro_pos].chpos_ref};
            __micro_push_err(err);
            goto err_exit;
        }
        fun_info->ret_type = micro_str2mt(micro_toks[micro_pos].val);
        micro_pos++;
    }
    if (micro_toks[micro_pos].type != MICRO_TT_KW_START) {
        micro_error_t err = {.msg = "Expected 'start' keyword",
                             .line_ref = micro_toks[micro_pos].line_ref,
                             .chpos_ref = micro_toks[micro_pos].chpos_ref};
        __micro_push_err(err);
        goto err_exit;
    }
    while (micro_toks[++micro_pos].type != MICRO_TT_KW_END) {
        micro_codegen_386_micro_instruction_parse();
    }
    char ret_instruction[] = asm_ret;
    push_instruction(ret_instruction);

    hashmap_micro_codegen_386_fun_info_t_set(micro_codegen_386_funs, tok_ident.val, fun_info);

    return;

err_exit:
    while (micro_toks[micro_pos].type != MICRO_TT_KW_END && micro_pos < micro_toks_size) {
        micro_pos++;
    }
    return;
}

void micro_codegen_386__call()
{
    micro_token_t tok_fun_name = __micro_get(1);
    micro_pos++;
    if (tok_fun_name.type != MICRO_TT_IDENT || tok_fun_name.type == MICRO_TT_NULL) {
        micro_error_t err = {.msg = "Expected called functon name after 'call' keyword",
                             .line_ref = micro_toks[micro_pos].line_ref,
                             .chpos_ref = micro_toks[micro_pos].chpos_ref};
        __micro_push_err(err);
        goto err_exit;
    }
    if (micro_toks[micro_pos].type != MICRO_TT_SEMICOLON) {
        micro_error_t err = {.msg = "Expected ';'",
                             .line_ref = micro_toks[micro_pos].line_ref,
                             .chpos_ref = micro_toks[micro_pos].chpos_ref};
        __micro_push_err(err);
        goto err_exit;
    }

    micro_codegen_386_fun_info_t *fun_info = hashmap_micro_codegen_386_fun_info_t_get(micro_codegen_386_funs, tok_fun_name.val);

    if (!fun_info) {
        micro_error_t err = {.msg = "Unknown function name",
                             .line_ref = tok_fun_name.line_ref,
                             .chpos_ref = tok_fun_name.chpos_ref};
        __micro_push_err(err);
        goto err_exit;
    }

    printf("call:%u\n", fun_info->offset);

    char addr[4];
    // вызов процедур по аддресу: адрес процедуры - (адрес текущей иструкции + размер иструкции call)
    micro_gen32imm_le(addr, fun_info->offset - (micro_outbuf->size + 5));
    char instruction[] = asm_call(addr);
    push_instruction(instruction);

    return;

err_exit:
    while (micro_toks[micro_pos].type != MICRO_TT_SEMICOLON && micro_pos < micro_toks_size) {
        micro_pos++;
    }
    return;
}

#endif