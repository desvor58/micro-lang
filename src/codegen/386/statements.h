#ifndef MICRO_CODEGEN_386_STATEMENTS_H
#define MICRO_CODEGEN_386_STATEMENTS_H

#include "common.h"

void micro_codegen_386__static_var()
{
    int error_skip = 0;

    micro_token_t type_tok = __micro_peek(1);
    micro_token_t ident_tok = __micro_peek(2);

    if (type_tok.type != MICRO_TT_TYPE_NAME || type_tok.type == MICRO_TT_NULL) {
        micro_error_t err = {.msg = "Expected type name after 'var' keyword", .line_ref = type_tok.line_ref, .chpos_ref = type_tok.chpos_ref};
        __micro_push_err(err);
        error_skip = 1;
    }
    if (ident_tok.type != MICRO_TT_IDENT || ident_tok.type == MICRO_TT_NULL) {
        micro_error_t err = {.msg = "Expected identifier after type name", .line_ref = ident_tok.line_ref, .chpos_ref = ident_tok.chpos_ref};
        __micro_push_err(err);
        error_skip = 1;
    }

    micro_codegen_386_var_info_t *var_info = (micro_codegen_386_var_info_t*)malloc(sizeof(micro_codegen_386_var_info_t));
    strcpy(var_info->name, ident_tok.val);
    var_info->type = micro_str2mt(type_tok.val);
    var_info->storage_type = MICRO_ST_DATASEC;
    var_info->mem_offset = micro_outbuf->size; // смещение будет такое, как число записаных байт в outbuf

    if (error_skip) {
        while (micro_toks[micro_pos].type != MICRO_TT_SEMICOLON && micro_pos < micro_toks_size) {
            micro_pos++;
        }
        return;
    }

    if (__micro_peek(3).type == MICRO_TT_SEMICOLON) {
        for (size_t i = 0; i < micro_mt_size[var_info->type]; i++) {
            string_push_back(micro_outbuf, 0);
        }
        micro_pos += 3;
        return;
    }
    if (micro_tokislit(__micro_peek(3))) {
        micro_codegen_386_micro_type lit_type = micro_lit2mt(__micro_peek(3), var_info->type);
        if (lit_type == MICRO_MT_NULL) {
            micro_error_t err = {.msg = "Wrong literal type", .line_ref = ident_tok.line_ref, .chpos_ref = ident_tok.chpos_ref};
            __micro_push_err(err);
            while (micro_toks[micro_pos].type != MICRO_TT_SEMICOLON && micro_pos < micro_toks_size) {
                micro_pos++;
            }
            return;
        }
        
        char *imm_val = (char*)malloc(sizeof(char) * micro_mt_size[lit_type]);
        imm_from_mt(imm_val, lit_type, strtoull(__micro_peek(3).val, (char**)0, 10));
        for (size_t i = 0; i < micro_mt_size[lit_type]; i++) {
            string_push_back(micro_outbuf, imm_val[i]);
        }
        micro_pos += 4;
    }
}

#endif