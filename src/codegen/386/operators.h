#ifndef MICRO_CODEGEN_386_EXPR_OPERATORS_H
#define MICRO_CODEGEN_386_EXPR_OPERATORS_H

#include "common.h"

int micro_codegen_386__op_plus(micro_codegen_t *codegen, size_t operand_offset, micro_codegen_386_storage_info_t dst);

int micro_codegen_386__op_minus(micro_codegen_t *codegen, size_t operand_offset, micro_codegen_386_storage_info_t dst);

int micro_codegen_386__op_multiply(micro_codegen_t *codegen, size_t operand_offset, micro_codegen_386_storage_info_t dst);

int micro_codegen_386__op_division(micro_codegen_t *codegen, size_t operand_offset, micro_codegen_386_storage_info_t dst);

int micro_codegen_386__op_negate(micro_codegen_t *codegen, size_t operand_offset, micro_codegen_386_storage_info_t dst);

int micro_codegen_386__op_deref(micro_codegen_t *codegen, size_t operand_offset, micro_codegen_386_storage_info_t dst);

#endif