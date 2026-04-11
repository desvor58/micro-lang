#ifndef MICRO_CODEGEN_386_EXPR_OPERATORS_H
#define MICRO_CODEGEN_386_EXPR_OPERATORS_H

#include "common.h"

static inline int __micro_cond_op_template(micro_codegen_t *                codegen,
                                           size_t                           operand_offset,
                                           micro_codegen_386_storage_info_t dst,
                                           void                           (*instr)(asm386_reg))
{
    void (*cmp_tbl[])(asm386_reg, asm386_reg) = {
        [MICRO_SZ_8]  = asm386_cmpR8R8,
        [MICRO_SZ_16] = asm386_cmpR16R16,
        [MICRO_SZ_32] = asm386_cmpR32R32,
    };
    cmp_tbl[dst.size](operand_offset, operand_offset + 1);
    if (operand_offset + 2 > REG8_BH) {
        asm386_pushR32(REG32_EBX);
    }
    instr(REG8_BH);
    void (*movres_tbl[])(asm386_reg, asm386_reg) = {
        [MICRO_SZ_8]  = asm386_movR8R8,
        [MICRO_SZ_16] = asm386_movzxR16R8,
        [MICRO_SZ_32] = asm386_movzxR32R8,
    };
    movres_tbl[dst.size](operand_offset, REG8_BH);
    if (operand_offset + 2 > REG8_BH) {
        asm386_popR32(REG32_EBX);
    }
    return 0;
}

MICRO_ATTRIBUTE_CONST
int micro_codegen_386__op_plus(micro_codegen_t *codegen, size_t operand_offset, micro_codegen_386_storage_info_t dst);

MICRO_ATTRIBUTE_CONST
int micro_codegen_386__op_minus(micro_codegen_t *codegen, size_t operand_offset, micro_codegen_386_storage_info_t dst);

MICRO_ATTRIBUTE_CONST
int micro_codegen_386__op_multiply(micro_codegen_t *codegen, size_t operand_offset, micro_codegen_386_storage_info_t dst);

MICRO_ATTRIBUTE_CONST
int micro_codegen_386__op_division(micro_codegen_t *codegen, size_t operand_offset, micro_codegen_386_storage_info_t dst);

MICRO_ATTRIBUTE_CONST
int micro_codegen_386__op_negate(micro_codegen_t *codegen, size_t operand_offset, micro_codegen_386_storage_info_t dst);

MICRO_ATTRIBUTE_CONST
int micro_codegen_386__op_deref(micro_codegen_t *codegen, size_t operand_offset, micro_codegen_386_storage_info_t dst);

MICRO_ATTRIBUTE_CONST
int micro_codegen_386__op_eq(micro_codegen_t *codegen, size_t operand_offset, micro_codegen_386_storage_info_t dst);

MICRO_ATTRIBUTE_CONST
int micro_codegen_386__op_not_eq(micro_codegen_t *codegen, size_t operand_offset, micro_codegen_386_storage_info_t dst);

MICRO_ATTRIBUTE_CONST
int micro_codegen_386__op_great(micro_codegen_t *codegen, size_t operand_offset, micro_codegen_386_storage_info_t dst);

MICRO_ATTRIBUTE_CONST
int micro_codegen_386__op_less(micro_codegen_t *codegen, size_t operand_offset, micro_codegen_386_storage_info_t dst);

MICRO_ATTRIBUTE_CONST
int micro_codegen_386__op_great_or_eq(micro_codegen_t *codegen, size_t operand_offset, micro_codegen_386_storage_info_t dst);

MICRO_ATTRIBUTE_CONST
int micro_codegen_386__op_less_or_eq(micro_codegen_t *codegen, size_t operand_offset, micro_codegen_386_storage_info_t dst);


#endif