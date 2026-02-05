#ifndef MICRO_CODEGEN_386_EXPR_H
#define MICRO_CODEGEN_386_EXPR_H

#include "common.h"
#include "expr_get_atoms.h"
#include "operators.h"

typedef struct {
    u8 operands_num;
    int (*handler)(size_t, micro_codegen_386_storage_info_t);
} micro_codegen_386_operator_info_t;

extern micro_codegen_386_operator_info_t micro_codegen_386_ops_table[];

size_t micro_expr_peek(size_t pos);

int micro_codegen_386_expr_parse(size_t pos, micro_codegen_386_storage_info_t dst);

#endif