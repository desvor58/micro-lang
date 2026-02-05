#ifndef MICRO_CODEGEN_386_EXPR_OPERATORS_H
#define MICRO_CODEGEN_386_EXPR_OPERATORS_H

#include "common.h"

int micro_codegen_386__op_plus(size_t operand_offset, micro_codegen_386_storage_info_t dst);

int micro_codegen_386__op_minus(size_t operand_offset, micro_codegen_386_storage_info_t dst);

int micro_codegen_386__op_multiply(size_t operand_offset, micro_codegen_386_storage_info_t dst);

int micro_codegen_386__op_division(size_t operand_offset, micro_codegen_386_storage_info_t dst);

int micro_codegen_386__op_negate(size_t operand_offset, micro_codegen_386_storage_info_t dst);

int micro_codegen_386__op_deref(size_t operand_offset, micro_codegen_386_storage_info_t dst);

#endif