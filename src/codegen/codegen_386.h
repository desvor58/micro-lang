#ifndef MICRO_CODEGEN_386_H
#define MICRO_CODEGEN_386_H

#include "codegen.h"
#include "386/common.h"
#include "386/statements.h"

typedef struct {
    int       code_in_function;
    ptrdiff_t top_stack_offset;
    // Ty: micro_codegen_386_ident_info_t
    sct_hashmap_t *idents;
    // Ty: string_t
    sct_list_pair_t *local_var_list;
    sct_list_pair_t *goto_refs;
} micro_codegen_386_ext_t;

void micro_codegen_386_micro_instruction_parse(micro_codegen_t *codegen);

void micro_codegen_386(micro_codegen_t *codegen);

void micro_codegen_386_init(micro_codegen_t *codegen, micro_tok_vec_t *toks);

void micro_codegen_386_deinit(micro_codegen_t *codegen);

#endif