#include <micro/codegen/386/operators.h>

int micro_codegen_386__op_plus(micro_codegen_t *codegen, size_t operand_offset, micro_codegen_386_storage_info_t dst)
{
    void (*instr_tbl1[])(asm386_reg, asm386_reg) = {
        [MICRO_SZ_8]  = asm386_addR8R8,
        [MICRO_SZ_16] = asm386_addR16R16,
        [MICRO_SZ_32] = asm386_addR32R32,
    };
    instr_tbl1[dst.size](operand_offset, operand_offset + 1);
    asm_put_instructions(codegen);
    return 0;
}

int micro_codegen_386__op_minus(micro_codegen_t *codegen, size_t operand_offset, micro_codegen_386_storage_info_t dst)
{
    void (*instr_tbl1[])(asm386_reg, asm386_reg) = {
        [MICRO_SZ_8]  = asm386_subR8R8,
        [MICRO_SZ_16] = asm386_subR16R16,
        [MICRO_SZ_32] = asm386_subR32R32,
    };
    instr_tbl1[dst.size](operand_offset, operand_offset + 1);
    asm_put_instructions(codegen);
    return 0;
}

int micro_codegen_386__op_multiply(micro_codegen_t *codegen, size_t operand_offset, micro_codegen_386_storage_info_t dst)
{
    if (dst.is_unsigned) {
        void (*xchg_tbl[])(asm386_reg, asm386_reg) = {
            [MICRO_SZ_8]  = asm386_xchgR8R8,
            [MICRO_SZ_16] = asm386_xchgR16R16,
            [MICRO_SZ_32] = asm386_xchgR32R32,
        };
        if (operand_offset != REG32_EAX) xchg_tbl[dst.size](REG32_EAX, operand_offset);

        void (*mul_tbl[])(asm386_reg) = {
            [MICRO_SZ_8]  = asm386_mulR8,
            [MICRO_SZ_16] = asm386_mulR16,
            [MICRO_SZ_32] = asm386_mulR32,
        };
        mul_tbl[dst.size](operand_offset + 1);

        if (operand_offset != REG32_EAX) xchg_tbl[dst.size](operand_offset, REG32_EAX);
    } else {
        void (*mul_tbl[])(asm386_reg, asm386_reg) = {
            [MICRO_SZ_8]  = asm386_imulR8R8,
            [MICRO_SZ_16] = asm386_imulR16R16,
            [MICRO_SZ_32] = asm386_imulR32R32,
        };
        mul_tbl[dst.size](operand_offset, operand_offset + 1);
    }
    asm_put_instructions(codegen);
    return 0;
}

int micro_codegen_386__op_division(micro_codegen_t *codegen, size_t operand_offset, micro_codegen_386_storage_info_t dst)
{
    if (operand_offset + 1 >= REG32_EDX) {
        asm386_pushR32(REG32_EDX);
    }
    void (*xchg_tbl[])(asm386_reg, asm386_reg) = {
        [MICRO_SZ_8]  = asm386_xchgR8R8,
        [MICRO_SZ_16] = asm386_xchgR16R16,
        [MICRO_SZ_32] = asm386_xchgR32R32,
    };
    if (operand_offset != REG32_EAX) xchg_tbl[dst.size](REG32_EAX, operand_offset);

    if (dst.is_unsigned) {
        void (*div_tbl[])(asm386_reg) = {
            [MICRO_SZ_8]  = asm386_divR8,
            [MICRO_SZ_16] = asm386_divR16,
            [MICRO_SZ_32] = asm386_divR32,
        };
        div_tbl[dst.size](operand_offset + 1);
    } else {
        void (*idiv_tbl[])(asm386_reg) = {
            [MICRO_SZ_8]  = asm386_idivR8,
            [MICRO_SZ_16] = asm386_idivR16,
            [MICRO_SZ_32] = asm386_idivR32,
        };
        idiv_tbl[dst.size](operand_offset + 1);
    }

    if (operand_offset != REG32_EAX) xchg_tbl[dst.size](operand_offset, REG32_EAX);
    if (operand_offset + 1 >= REG32_EDX) {
        asm386_popR32(REG32_EDX);
    }
    asm_put_instructions(codegen);
    return 0;
}

int micro_codegen_386__op_negate(micro_codegen_t *codegen, size_t operand_offset, micro_codegen_386_storage_info_t dst)
{
    void (*neg_tbl[])(asm386_reg) = {
        [MICRO_SZ_8]  = asm386_negR8,
        [MICRO_SZ_16] = asm386_negR16,
        [MICRO_SZ_32] = asm386_negR32,
    };
    neg_tbl[dst.size](operand_offset);
    asm_put_instructions(codegen);
    return 0;
}

int micro_codegen_386__op_deref(micro_codegen_t *codegen, size_t operand_offset, micro_codegen_386_storage_info_t dst)
{
    void (*mov_tbl[])(asm386_reg, asm386_reg) = {
        [MICRO_SZ_8]  = asm386_movR8MR8,
        [MICRO_SZ_16] = asm386_movR16MR16,
        [MICRO_SZ_32] = asm386_movR32MR32,
    };
    mov_tbl[dst.size](operand_offset, operand_offset);
    asm_put_instructions(codegen);
    return 0;
}