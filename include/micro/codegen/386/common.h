#ifndef MICRO_CODEGEN_386_COMMON_H
#define MICRO_CODEGEN_386_COMMON_H

#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#ifdef __linux__
# include <alloca.h>
#endif

#include <SCT/hashmap.h>
#include <SCT/string.h>
#include <SCT/vector.h>

#include "../codegen.h"
#include "../../common.h"
#include "../../lexer.h"
#include "../../asm/asm386.h"

typedef struct {
    int       code_in_function;
    ptrdiff_t top_stack_offset;
    // Ty: micro_codegen_386_ident_info_t
    sct_hashmap_t *idents;
    // Ty: string_t
    sct_list_pair_t *local_var_list;
    sct_list_pair_t *defer_addr_refs;
} micro_codegen_386_ext_t;

#define get_codegen_386_ext(cg) ((micro_codegen_386_ext_t*)(cg)->ext)

#define micro_tokislit(tok) ((tok).type == MICRO_TT_LIT_FLOAT  \
                          || (tok).type == MICRO_TT_LIT_INT    \
                          || (tok).type == MICRO_TT_LIT_STR)   \

#define micro_tokisop(tok) ((tok).type == MICRO_TT_PLUS         \
                         || (tok).type == MICRO_TT_MINUS        \
                         || (tok).type == MICRO_TT_STAR         \
                         || (tok).type == MICRO_TT_SLASH        \
                         || (tok).type == MICRO_TT_AMPERSAND    \
                         || (tok).type == MICRO_TT_HASH         \
                         || (tok).type == MICRO_TT_DOLLAR       \
                         || (tok).type == MICRO_TT_TILDE        \
                         || (tok).type == MICRO_TT_EQ           \
                         || (tok).type == MICRO_TT_NOT_EQ       \
                         || (tok).type == MICRO_TT_GREAT        \
                         || (tok).type == MICRO_TT_LESS         \
                         || (tok).type == MICRO_TT_GREAT_OR_EQ  \
                         || (tok).type == MICRO_TT_LESS_OR_EQ   \
                         || (tok).type == MICRO_TT_APOSTROPHE)  \

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

typedef enum {
    MICRO_SZ_8  = 0,
    MICRO_SZ_16 = 1,
    MICRO_SZ_32 = 2,
} micro_codegen_386_size;

#define micro_mtisunsigned(mt) (mt == MICRO_MT_U8 || mt == MICRO_MT_U16 || mt == MICRO_MT_U32 || mt == MICRO_MT_PTR)

extern micro_codegen_386_size micro_mt_size[];
extern size_t                 micro_sz_real_size[];

micro_codegen_386_micro_type micro_str2mt(char *str);

micro_codegen_386_micro_type micro_lit2mt(micro_token_t lit, micro_codegen_386_micro_type expected);

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
    ptrdiff_t offset;
    micro_codegen_386_size size;
    int is_unsigned;
} micro_codegen_386_storage_info_t;

typedef struct {
    char name[MICRO_MAX_SYMBOL_SIZE];
    micro_codegen_386_micro_type type;
    micro_codegen_386_storage_info_t storage_info;
} micro_codegen_386_var_info_t;

typedef struct {
    char name[MICRO_MAX_SYMBOL_SIZE];
    micro_codegen_386_micro_type ret_type;
    // Ty: micro_codegen_386_var_info_t
    sct_vector_t *args;
    size_t offset;
} micro_codegen_386_fun_info_t;

typedef struct {
    char name[MICRO_MAX_SYMBOL_SIZE];
    size_t offset;
} micro_codegen_386_lbl_info_t;

typedef enum {
    MICRO_IT_VAR,
    MICRO_IT_FUN,
    MICRO_IT_LBL,
} micro_codegen_386_ident_type;

typedef struct {
    micro_codegen_386_ident_type type;
    union {
        micro_codegen_386_var_info_t var_info;
        micro_codegen_386_fun_info_t fun_info;
        micro_codegen_386_lbl_info_t lbl_info;
    };
} micro_codegen_386_ident_info_t;

typedef struct {
    char  *lbl_name;
    size_t lbl_code_ref;
    size_t outbuf_ref;
} micro_defer_addr_ref_t;

micro_token_t __micro_peek(micro_codegen_t *codegen, size_t offset);

micro_token_t __micro_get(micro_codegen_t *codegen, size_t offset);

void __micro_dbg_print_idents(micro_codegen_t *codegen);

micro_codegen_386_micro_type micro_gettype(micro_codegen_t *codegen, micro_token_t tok, micro_codegen_386_micro_type expected);

void micro_codegen_386_micro_instruction_parse(micro_codegen_t *codegen);

#endif