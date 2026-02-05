#include "../../expr_get_atoms.h"

int __micro_codegen_386_expr_parse_get_ident_from_stack(u8 *ident_addr, micro_codegen_386_var_info_t *var_info, micro_codegen_386_storage_info_t dst)
{
    if (dst.type == MICRO_ST_DATASEG) {
        u8 dst_addr[4];
        micro_gen32imm_le(dst_addr, dst.offset);

        if (dst.size == 4) {
            u8 instruction1[] = asm_movR32S32(REG32_EAX, ident_addr);
            push_instruction(instruction1);
            u8 instruction2[] = asm_movM32R32(dst_addr, REG32_EAX);
            push_instruction(instruction2);
        } else
        if (dst.size == 2) {
            u8 instruction1[] = asm_movR16S32(REG16_AX, ident_addr);
            push_instruction(instruction1);
            u8 instruction2[] = asm_movM16R16(dst_addr, REG16_AX);
            push_instruction(instruction2);
        } else
        if (dst.size == 1) {
            u8 instruction1[] = asm_movR8S32(REG8_AL, ident_addr);
            push_instruction(instruction1);
            u8 instruction2[] = asm_movM8R8(dst_addr, REG8_AL);
            push_instruction(instruction2);
        } else {
            return 1;
        }
    } else
    if (dst.type == MICRO_ST_REG) {
        int dst_reg = dst.offset;
        if (dst.size == 4) {
            u8 instruction[] = asm_movR32S32(dst_reg, ident_addr);
            push_instruction(instruction);
        } else
        if (dst.size == 2) {
            u8 instruction[] = asm_movR16S32(dst_reg, ident_addr);
            push_instruction(instruction);
        } else
        if (dst.size == 1) {
            u8 instruction[] = asm_movR8S32(dst_reg, ident_addr);
            push_instruction(instruction);
        } else {
            return 1;
        }
    } else
    if (dst.type == MICRO_ST_STACK) {
        u8 dst_offset[4];
        micro_gen32imm_le(dst_offset, dst.offset);
        if (dst.size == 4) {
            u8 instruction1[] = asm_movR32S32(REG32_EAX, ident_addr);
            push_instruction(instruction1);
            u8 instruction2[] = asm_movS32R32(dst_offset, REG32_EAX);
            push_instruction(instruction2);
        } else
        if (dst.size == 2) {
            u8 instruction1[] = asm_movR16S32(REG16_AX, ident_addr);
            push_instruction(instruction1);
            u8 instruction2[] = asm_movS32R16(dst_offset, REG16_AX);
            push_instruction(instruction2);
        } else
        if (dst.size == 1) {
            u8 instruction1[] = asm_movR8S32(REG8_AL, ident_addr);
            push_instruction(instruction1);
            u8 instruction2[] = asm_movS32R8(dst_offset, REG8_AL);
            push_instruction(instruction2);
        } else {
            return 1;
        }
    }
    return 0;
}

int __micro_codegen_386_expr_parse_get_ident_addr_from_stack(u8 *ident_addr, micro_codegen_386_var_info_t *var_info, micro_codegen_386_storage_info_t dst)
{
    if (dst.type == MICRO_ST_DATASEG) {
        u8 dst_addr[4];
        micro_gen32imm_le(dst_addr, dst.offset);

        if (dst.size == 4) {
            u8 instruction1[] = asm_leaR32S32(REG32_EAX, ident_addr);
            push_instruction(instruction1);
            u8 instruction2[] = asm_movM32R32(dst_addr, REG32_EAX);
            push_instruction(instruction2);
        } else
        if (dst.size == 2) {
            u8 instruction1[] = asm_leaR16S32(REG16_AX, ident_addr);
            push_instruction(instruction1);
            u8 instruction2[] = asm_movM16R16(dst_addr, REG16_AX);
            push_instruction(instruction2);
        } else {
            return 1;
        }
    } else
    if (dst.type == MICRO_ST_REG) {
        int dst_reg = dst.offset;
        if (dst.size == 4) {
            u8 instruction[] = asm_leaR32S32(dst_reg, ident_addr);
            push_instruction(instruction);
        } else
        if (dst.size == 2) {
            u8 instruction[] = asm_leaR16S32(dst_reg, ident_addr);
            push_instruction(instruction);
        } else {
            return 1;
        }
    } else
    if (dst.type == MICRO_ST_STACK) {
        u8 dst_offset[4];
        micro_gen32imm_le(dst_offset, dst.offset);
        if (dst.size == 4) {
            u8 instruction1[] = asm_leaR32S32(REG32_EAX, ident_addr);
            push_instruction(instruction1);
            u8 instruction2[] = asm_movS32R32(dst_offset, REG32_EAX);
            push_instruction(instruction2);
        } else
        if (dst.size == 2) {
            u8 instruction1[] = asm_leaR16S32(REG16_AX, ident_addr);
            push_instruction(instruction1);
            u8 instruction2[] = asm_movS32R16(dst_offset, REG16_AX);
            push_instruction(instruction2);
        } else {
            return 1;
        }
    }
    return 0;
}