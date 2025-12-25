#ifndef CODEGEN_H
#define CODEGEN_H

#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include <SCT/hashmap.h>
#include <SCT/string.h>

#include "types.h"
#include "config.h"
#include "lexer.h"

#define tokislit(tok) (tok.type == TT_LIT_FLOAT || tok.type == TT_LIT_INT || tok.type == TT_LIT_STR)

typedef enum {
    MT_NULL = 0,
    MT_I8,
    MT_U8,
    MT_I16,
    MT_U16,
    MT_I32,
    MT_U32,
    MT_F32,
    MT_PTR,
} codegen_micro_type;

size_t mt_size[] = {
    [MT_NULL] = 0,
    [MT_I8]   = 8,
    [MT_U8]   = 8,
    [MT_I16]  = 16,
    [MT_U16]  = 16,
    [MT_I32]  = 32,
    [MT_U32]  = 32,
    [MT_F32]  = 32,
    [MT_PTR]  = 32
};

codegen_micro_type str2mt(char *str)
{
    if (!strcmp(str, "i8"))  return MT_I8;
    if (!strcmp(str, "u8"))  return MT_U8;
    if (!strcmp(str, "i16")) return MT_I16;
    if (!strcmp(str, "u16")) return MT_U16;
    if (!strcmp(str, "i32")) return MT_I32;
    if (!strcmp(str, "u32")) return MT_U32;
    if (!strcmp(str, "f32")) return MT_F32;
    if (!strcmp(str, "ptr")) return MT_PTR;
    return MT_NULL;
}

codegen_micro_type lit2mt(token_t lit, codegen_micro_type expected)
{
    if (!tokislit(lit)) {
        return MT_NULL;
    }
    if (lit.type == TT_LIT_FLOAT) {
        if (expected != MT_F32) return MT_NULL;
        return MT_F32;
    }
    if (lit.type == TT_LIT_INT) {
        if (expected != MT_I8
         && expected != MT_U8
         && expected != MT_I16
         && expected != MT_U16
         && expected != MT_I32
         && expected != MT_U32
        ) {
            return MT_NULL;
        }
        return expected;
    }
    if (lit.type == TT_LIT_STR) {
        if (expected != MT_PTR) return MT_NULL;
        return MT_PTR;
    }
    return MT_NULL;
}

typedef enum {
    ST_STACK,
    ST_DATASEC,
} codegen_storage_type;

typedef struct {
    char name[MAX_SYMBOL_SIZE];
    codegen_micro_type type;
    codegen_storage_type storage_type;
    size_t mem_offset;
} codegen_var_info_t;

genhashmap(codegen_var_info_t);

error_t *codegen_err_stk;
size_t   codegen_err_stk_size;
size_t __codegen_err_stk_real_size;

hashmap_codegen_var_info_t_t *codegen_vars;

string_t *outbuf;

size_t pos = 0;

token_t __peek(size_t offset)
{
    if (pos + offset >= toks_size) {
        return (token_t){.type = TT_NULL};
    }
    return toks[pos + offset];
}

token_t __get(size_t offset)
{
    if (pos + offset >= toks_size) {
        return (token_t){.type = TT_NULL};
    }
    return toks[pos += offset];
}

void codegen_init()
{
    codegen_err_stk = (error_t*)malloc(sizeof(error_t) * ERROR_STACK_EXTEND_SIZE);
    codegen_err_stk_size = 0;
    __codegen_err_stk_real_size = ERROR_STACK_EXTEND_SIZE;

    codegen_vars = hashmap_codegen_var_info_t_create();

    outbuf = string_create();
}

void codegen_delete()
{
    free(codegen_err_stk);
    hashmap_codegen_var_info_t_full_free(codegen_vars);
    string_free(outbuf);
}

void __codegen_err_stk_size_check(size_t offset)
{
    if (codegen_err_stk_size + offset >= __codegen_err_stk_real_size) {
        error_t *new_stk = (error_t*)malloc(sizeof(error_t) * (__codegen_err_stk_real_size += offset / ERROR_STACK_EXTEND_SIZE + 1));
        memcpy(new_stk, codegen_err_stk, sizeof(error_t) * codegen_err_stk_size);
        free(codegen_err_stk);
        codegen_err_stk = new_stk;
    }
}

void codegen()
{
    for (pos = 0; pos < toks_size; pos++) {
        // global var
        if (toks[pos].type == TT_KW_VAR) {
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

            codegen_var_info_t *var_info = (codegen_var_info_t*)malloc(sizeof(codegen_var_info_t));
            strcpy(var_info->name, ident_tok.val);
            var_info->type = str2mt(type_tok.val);
            var_info->storage_type = ST_DATASEC;
            var_info->mem_offset = outbuf->size; // смещение будет такое, как чисто записаных байт в outbuf

            if (__peek(3).type == TT_SEMICOLON) {
                for (size_t i = 0; i < mt_size[var_info->type] / 8; i++) {
                    string_push_back(outbuf, 0);
                }
                pos += 3;
                continue;
            }

            if (tokislit(__peek(4))) {
                // TODO
            }
        }
    }
}

#endif