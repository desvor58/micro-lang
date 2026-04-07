#include "../../expr_get_atoms.h"

int __micro_codegen_386_expr_parse_get_ident_from_stack(micro_codegen_t *codegen, micro_addr_le_t ident_addr, micro_codegen_386_var_info_t *var_info, micro_codegen_386_storage_info_t dst)
{
    if (dst.type == MICRO_ST_DATASEG) {
        void (*instr_tbl1[])(asm386_reg, micro_addr_le_t) = {
            [MICRO_SZ_8]  = asm386_movR8S32,
            [MICRO_SZ_16] = asm386_movR16S32,
            [MICRO_SZ_32] = asm386_movR32M32,
        };
        instr_tbl1[dst.size](REG32_EAX, ident_addr);

        void (*instr_tbl2[])(micro_addr_le_t, asm386_reg) = {
            [MICRO_SZ_8]  = asm386_movM8R8,
            [MICRO_SZ_16] = asm386_movM16R16,
            [MICRO_SZ_32] = asm386_movM32R32,
        };
        instr_tbl2[dst.size](micro_imm_le_gen(dst.offset), REG32_EAX);
    } else
    if (dst.type == MICRO_ST_REG) {
        void (*instr_tbl[])(asm386_reg, micro_addr_le_t) = {
            [MICRO_SZ_8]  = asm386_movR8S32,
            [MICRO_SZ_16] = asm386_movR16S32,
            [MICRO_SZ_32] = asm386_movR32S32,
        };
        instr_tbl[dst.size](dst.offset, ident_addr);
    } else
    if (dst.type == MICRO_ST_STACK) {
        void (*instr_tbl1[])(asm386_reg, micro_addr_le_t) = {
            [MICRO_SZ_8]  = asm386_movR8S32,
            [MICRO_SZ_16] = asm386_movR16S32,
            [MICRO_SZ_32] = asm386_movR32S32,
        };
        instr_tbl1[dst.size](REG32_EAX, ident_addr);

        void (*instr_tbl2[])(micro_addr_le_t, asm386_reg) = {
            [MICRO_SZ_8]  = asm386_movS32R8,
            [MICRO_SZ_16] = asm386_movS32R16,
            [MICRO_SZ_32] = asm386_movS32R32,
        };
        instr_tbl2[dst.size](micro_imm_le_gen(dst.offset), REG32_EAX);
    }
    asm_put_instructions(codegen);
    return 0;
}

int __micro_codegen_386_expr_parse_get_ident_addr_from_stack(micro_codegen_t *codegen, micro_addr_le_t ident_addr, micro_codegen_386_var_info_t *var_info, micro_codegen_386_storage_info_t dst)
{
    if (dst.type == MICRO_ST_DATASEG) {
        if (dst.size == MICRO_SZ_8) {
            return 1;
        }
        void (*instr_tbl1[])(asm386_reg, micro_addr_le_t) = {
            [MICRO_SZ_16] = asm386_leaR16S32,
            [MICRO_SZ_32] = asm386_leaR32S32,
        };
        instr_tbl1[dst.size](REG32_EAX, ident_addr);

        void (*instr_tbl2[])(micro_addr_le_t, asm386_reg) = {
            [MICRO_SZ_16] = asm386_movM16R16,
            [MICRO_SZ_32] = asm386_movM32R32,
        };
        instr_tbl2[dst.size](micro_imm_le_gen(dst.offset), REG32_EAX);
    } else
    if (dst.type == MICRO_ST_REG) {
        if (dst.size == MICRO_SZ_8) {
            return 1;
        }
        void (*instr_tbl1[])(asm386_reg, micro_addr_le_t) = {
            [MICRO_SZ_16] = asm386_leaR16S32,
            [MICRO_SZ_32] = asm386_leaR32S32,
        };
        instr_tbl1[dst.size](dst.offset, ident_addr);
    } else
    if (dst.type == MICRO_ST_STACK) {
        if (dst.size == MICRO_SZ_8) {
            return 1;
        }
        void (*instr_tbl1[])(asm386_reg, micro_addr_le_t) = {
            [MICRO_SZ_16] = asm386_leaR16S32,
            [MICRO_SZ_32] = asm386_leaR32S32,
        };
        instr_tbl1[dst.size](REG32_EAX, ident_addr);

        void (*instr_tbl2[])(micro_addr_le_t, asm386_reg) = {
            [MICRO_SZ_16] = asm386_movS32R16,
            [MICRO_SZ_32] = asm386_movS32R32,
        };
        instr_tbl2[dst.size](micro_imm_le_gen(dst.offset), REG32_EAX);
    }
    asm_put_instructions(codegen);
    return 0;
}