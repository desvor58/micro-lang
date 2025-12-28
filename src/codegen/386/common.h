#ifndef MICRO_CODEGEN_386_COMMON_H
#define MICRO_CODEGEN_386_COMMON_H

#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include <SCT/hashmap.h>
#include <SCT/string.h>

#include "../../types.h"
#include "../../config.h"
#include "../../lexer.h"
#include "../../asm/asm386.h"

#define micro_tokislit(tok) ((tok).type == MICRO_TT_LIT_FLOAT  \
                          || (tok).type == MICRO_TT_LIT_INT    \
                          || (tok).type == MICRO_TT_LIT_STR)   \

#define micro_tokisop(tok) ((tok).type == MICRO_TT_PLUS    \
                         || (tok).type == MICRO_TT_MINUS   \
                         || (tok).type == MICRO_TT_STAR    \
                         || (tok).type == MICRO_TT_SLASH)  \

typedef enum {
    MICRO_MT_NULL = 0,
    MICRO_MT_I8,
    MICRO_MT_U8,
    MICRO_MT_I16,
    MICRO_MT_U16,
    MICRO_MT_I32,
    MICRO_MT_U32,
    MICRO_MT_F32,
    MICRO_MT_PTR,
} micro_codegen_386_micro_type;

// in bytes
size_t micro_mt_size[] = {
    [MICRO_MT_NULL] = 0,
    [MICRO_MT_I8]   = 1,
    [MICRO_MT_U8]   = 1,
    [MICRO_MT_I16]  = 2,
    [MICRO_MT_U16]  = 2,
    [MICRO_MT_I32]  = 4,
    [MICRO_MT_U32]  = 4,
    [MICRO_MT_F32]  = 4,
    [MICRO_MT_PTR]  = 4
};

micro_codegen_386_micro_type micro_str2mt(char *str)
{
    if (!strcmp(str, "i8"))  return MICRO_MT_I8;
    if (!strcmp(str, "u8"))  return MICRO_MT_U8;
    if (!strcmp(str, "i16")) return MICRO_MT_I16;
    if (!strcmp(str, "u16")) return MICRO_MT_U16;
    if (!strcmp(str, "i32")) return MICRO_MT_I32;
    if (!strcmp(str, "u32")) return MICRO_MT_U32;
    if (!strcmp(str, "f32")) return MICRO_MT_F32;
    if (!strcmp(str, "ptr")) return MICRO_MT_PTR;
    return MICRO_MT_NULL;
}

micro_codegen_386_micro_type micro_lit2mt(micro_token_t lit, micro_codegen_386_micro_type expected)
{
    if (!micro_tokislit(lit)) {
        return MICRO_MT_NULL;
    }
    if (lit.type == MICRO_TT_LIT_FLOAT) {
        if (expected != MICRO_MT_F32) return MICRO_MT_NULL;
        return MICRO_MT_F32;
    }
    if (lit.type == MICRO_TT_LIT_INT) {
        if (expected != MICRO_MT_I8
         && expected != MICRO_MT_U8
         && expected != MICRO_MT_I16
         && expected != MICRO_MT_U16
         && expected != MICRO_MT_I32
         && expected != MICRO_MT_U32
        ) {
            return MICRO_MT_NULL;
        }
        return expected;
    }
    if (lit.type == MICRO_TT_LIT_STR) {
        if (expected != MICRO_MT_PTR) return MICRO_MT_NULL;
        return MICRO_MT_PTR;
    }
    return MICRO_MT_NULL;
}

void micro_imm_from_mt(char *buf, micro_codegen_386_micro_type type, size_t val)
{
    if (micro_mt_size[type] == 1) {
        buf[0] = val;
    } else
    if (micro_mt_size[type] == 2) {
        micro_gen16imm_le(buf, val);
    } else
    if (micro_mt_size[type] == 4) {
        micro_gen32imm_le(buf, val);
    }
}

typedef enum {
    MICRO_ST_STACK,
    MICRO_ST_DATASEG,
    MICRO_ST_REG,
} micro_codegen_386_storage_type;

typedef struct {
    micro_codegen_386_storage_type type;
    // if type == MICRO_ST_STACK:    esp + offset
    // if type == MICRO_ST_DATASEG:  start_of_file + org + offset
    // if type == MICRO_ST_REGISTER: offset - num of register
    offset_t offset;
    size_t size;
} micro_codegen_386_storage_info_t;

typedef struct {
    char name[MICRO_MAX_SYMBOL_SIZE];
    micro_codegen_386_micro_type type;
    micro_codegen_386_storage_info_t storage_info;
} micro_codegen_386_var_info_t;

genhashmap(micro_codegen_386_var_info_t);

micro_error_t *micro_codegen_386_err_stk;
size_t         micro_codegen_386_err_stk_size;
size_t       __micro_codegen_386_err_stk_real_size;

#define __micro_push_err(err) __micro_codegen_386_err_stk_size_check(1); micro_codegen_386_err_stk[micro_codegen_386_err_stk_size++] = err

hashmap_micro_codegen_386_var_info_t_t *micro_codegen_386_vars;

string_t *micro_outbuf;

size_t micro_pos = 0;

micro_token_t __micro_peek(size_t offset)
{
    if (micro_pos + offset >= micro_toks_size) {
        return (micro_token_t){.type = MICRO_TT_NULL};
    }
    return micro_toks[micro_pos + offset];
}

micro_token_t __micro_get(size_t offset)
{
    if (micro_pos + offset >= micro_toks_size) {
        return (micro_token_t){.type = MICRO_TT_NULL};
    }
    return micro_toks[micro_pos += offset];
}

void micro_codegen_386_init()
{
    micro_codegen_386_err_stk = (micro_error_t*)malloc(sizeof(micro_error_t) * MICRO_ERROR_STACK_EXTEND_SIZE);
    micro_codegen_386_err_stk_size = 0;
    __micro_codegen_386_err_stk_real_size = MICRO_ERROR_STACK_EXTEND_SIZE;

    micro_codegen_386_vars = hashmap_micro_codegen_386_var_info_t_create();

    micro_outbuf = string_create();
}

void micro_codegen_386_delete()
{
    free(micro_codegen_386_err_stk);
    hashmap_micro_codegen_386_var_info_t_full_free(micro_codegen_386_vars);
    string_free(micro_outbuf);
}

void __micro_codegen_386_err_stk_size_check(size_t offset)
{
    if (micro_codegen_386_err_stk_size + offset >= __micro_codegen_386_err_stk_real_size) {
        micro_error_t *new_stk = (micro_error_t*)malloc(sizeof(micro_error_t) * (__micro_codegen_386_err_stk_real_size += offset / MICRO_ERROR_STACK_EXTEND_SIZE + 1));
        memcpy(new_stk, micro_codegen_386_err_stk, sizeof(micro_error_t) * micro_codegen_386_err_stk_size);
        free(micro_codegen_386_err_stk);
        micro_codegen_386_err_stk = new_stk;
    }
}

micro_codegen_386_micro_type micro_gettype(micro_token_t tok, micro_codegen_386_micro_type expected)
{
    if (micro_tokislit(tok)) {
        return micro_lit2mt(tok, expected);
    }
    if (tok.type == MICRO_TT_IDENT) {
        micro_codegen_386_var_info_t *var_info = hashmap_micro_codegen_386_var_info_t_get(micro_codegen_386_vars, tok.val);
        if (var_info->type == expected) {
            return expected;
        }
    }
    return MICRO_MT_NULL;
}

#define push_instruction(instruction)                                        \
    for (size_t i = 0; i < sizeof(instruction)/sizeof(*instruction); i++) {  \
        string_push_back(micro_outbuf, instruction[i]);                      \
    }

#endif