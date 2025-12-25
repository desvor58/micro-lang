#ifndef CODEGEN_386_STATEMENTS_H
#define CODEGEN_386_STATEMENTS_H

#include "common.h"

void codegen_368__static_var()
{
    int error_skip = 0;

    token_t type_tok = __peek(1);
    token_t ident_tok = __peek(2);

    if (type_tok.type != TT_TYPE_NAME || type_tok.type == TT_NULL) {
        error_t err = {.msg = "Expected type name after 'var' keyword", .line_ref = type_tok.line_ref, .chpos_ref = type_tok.chpos_ref};
        push_err(err);
        error_skip = 1;
    }
    if (ident_tok.type != TT_IDENT || ident_tok.type == TT_NULL) {
        error_t err = {.msg = "Expected identifier after type name", .line_ref = ident_tok.line_ref, .chpos_ref = ident_tok.chpos_ref};
        push_err(err);
        error_skip = 1;
    }

    codegen_368_var_info_t *var_info = (codegen_368_var_info_t*)malloc(sizeof(codegen_368_var_info_t));
    strcpy(var_info->name, ident_tok.val);
    var_info->type = str2mt(type_tok.val);
    var_info->storage_type = ST_DATASEC;
    var_info->mem_offset = outbuf->size; // смещение будет такое, как число записаных байт в outbuf

    if (error_skip) {
        while (toks[pos].type != TT_SEMICOLON && pos < toks_size) {
            pos++;
        }
        return;
    }

    if (__peek(3).type == TT_SEMICOLON) {
        for (size_t i = 0; i < mt_size[var_info->type]; i++) {
            string_push_back(outbuf, 0);
        }
        pos += 3;
        return;
    }
    if (tokislit(__peek(3))) {
        codegen_368_micro_type lit_type = lit2mt(__peek(3), var_info->type);
        if (lit_type == MT_NULL) {
            error_t err = {.msg = "Wrong literal type", .line_ref = ident_tok.line_ref, .chpos_ref = ident_tok.chpos_ref};
            push_err(err);
            while (toks[pos].type != TT_SEMICOLON && pos < toks_size) {
                pos++;
            }
            return;
        }
        
        char *imm_val = (char*)malloc(sizeof(char) * mt_size[lit_type]);
        imm_from_mt(imm_val, lit_type, strtoull(__peek(3).val, (char**)0, 10));
        for (size_t i = 0; i < mt_size[lit_type]; i++) {
            string_push_back(outbuf, imm_val[i]);
        }
        pos += 4;
    }
}

#endif