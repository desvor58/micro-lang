#ifndef MICRO_CODEGEN_386_EXPR_GET_ATOMS_H
#define MICRO_CODEGEN_386_EXPR_GET_ATOMS_H

#include "common.h"

int __micro_codegen_386_expr_parse_get_lit(size_t pos, micro_codegen_386_storage_info_t dst);

int __micro_codegen_386_expr_parse_get_ident_from_dataseg(u8 *ident_addr, micro_codegen_386_var_info_t *var_info, micro_codegen_386_storage_info_t dst);

int __micro_codegen_386_expr_parse_get_ident_from_stack(u8 *ident_addr, micro_codegen_386_var_info_t *var_info, micro_codegen_386_storage_info_t dst);

int __micro_codegen_386_expr_parse_get_ident_addr_from_dataseg(u8 *ident_addr, micro_codegen_386_var_info_t *var_info, micro_codegen_386_storage_info_t dst);

int __micro_codegen_386_expr_parse_get_ident_addr_from_stack(u8 *ident_addr, micro_codegen_386_var_info_t *var_info, micro_codegen_386_storage_info_t dst);

int __micro_codegen_386_expr_parse_get_ident(size_t pos, micro_codegen_386_storage_info_t dst, int expected_addr);

#endif