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
    var_info->storage_info.type = MICRO_ST_DATASEC;
    var_info->storage_info.offset = micro_outbuf->size; // смещение будет такое, как число записаных байт в outbuf
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
            micro_error_t err = {.msg = "Expected `;`", .line_ref = ident_tok.line_ref, .chpos_ref = ident_tok.chpos_ref};
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
    int error_skip = 0;

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
    int err_expr = micro_codegen_386_expr_parse(micro_pos + 2, var_info->storage_info);
    if (err_expr) {
        goto err_exit;
    }
    return;

err_exit:
    while (micro_toks[micro_pos].type == MICRO_TT_SEMICOLON && micro_pos != micro_toks_size) {
        micro_pos++;
    }
    return;
}

#endif