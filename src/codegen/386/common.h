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

#include "../../types.h"
#include "../../config.h"
#include "../../lexer.h"
#include "../../asm/asm386.h"

#define micro_tokislit(tok) ((tok).type == MICRO_TT_LIT_FLOAT  \
                          || (tok).type == MICRO_TT_LIT_INT    \
                          || (tok).type == MICRO_TT_LIT_STR)   \

#define micro_tokisop(tok) ((tok).type == MICRO_TT_PLUS        \
                         || (tok).type == MICRO_TT_MINUS       \
                         || (tok).type == MICRO_TT_STAR        \
                         || (tok).type == MICRO_TT_SLASH       \
                         || (tok).type == MICRO_TT_AMPERRSAND  \
                         || (tok).type == MICRO_TT_HASH        \
                         || (tok).type == MICRO_TT_DOLAR       \
                         || (tok).type == MICRO_TT_TILDE       \
                         || (tok).type == MICRO_TT_APOSTROPHE  \
                        )

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

#define micro_mtisunsigned(mt) (mt == MICRO_MT_U8 || mt == MICRO_MT_U16 || mt == MICRO_MT_U32 || mt == MICRO_MT_PTR)

// in bytes
extern size_t micro_mt_size[];

micro_codegen_386_micro_type micro_str2mt(char *str);

micro_codegen_386_micro_type micro_lit2mt(micro_token_t lit, micro_codegen_386_micro_type expected);

void micro_imm_from_mt(u8 *buf, micro_codegen_386_micro_type type, size_t val);

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
    IT_VAR,
    IT_FUN,
    IT_LBL,
} micro_codegen_386_ident_type;

typedef struct {
    micro_codegen_386_ident_type type;
    union {
        micro_codegen_386_var_info_t var_info;
        micro_codegen_386_fun_info_t fun_info;
        micro_codegen_386_lbl_info_t lbl_info;
    };
} micro_codegen_386_ident_info_t;

// Ty: micro_codegen_386_ident_info_t
extern sct_hashmap_t *micro_codegen_386_idents;

// Ty: string_t
extern sct_list_pair_t *micro_codegen_386_local_vars_list;

extern micro_error_t *micro_codegen_386_err_stk;
extern size_t         micro_codegen_386_err_stk_size;
extern size_t       __micro_codegen_386_err_stk_real_size;

#define __micro_push_err(err) __micro_codegen_386_err_stk_size_check(1); micro_codegen_386_err_stk[micro_codegen_386_err_stk_size++] = err

extern int      micro_code_in_function;
extern offset_t micro_top_stack_offset;

typedef struct {
    size_t code_ref;
    size_t outbuf_ref;
} micro_goto_ref_t;

extern sct_list_pair_t *micro_goto_refs;

extern sct_string_t *micro_outbuf;

extern size_t micro_pos;

void micro_codegen_386_init();

void micro_codegen_386_delete();

micro_token_t __micro_peek(size_t offset);

micro_token_t __micro_get(size_t offset);

void __micro_codegen_386_err_stk_size_check(size_t offset);

void __micro_dbg_print_idents();

micro_codegen_386_micro_type micro_gettype(micro_token_t tok, micro_codegen_386_micro_type expected);

#define push_instruction(instruction)                                        \
    for (size_t i = 0; i < sizeof(instruction)/sizeof(*instruction); i++) {  \
        sct_string_push_back(micro_outbuf, instruction[i]);                  \
    }

#define push_instruction2addr(instruction, addr)                                       \
    for (size_t i = 0; i < sizeof(instruction)/sizeof(*instruction); i++) {            \
        micro_outbuf->str[addr + i] = instruction[i];                                  \
    }                                                                                  \

void micro_codegen_386_micro_instruction_parse();

#endif