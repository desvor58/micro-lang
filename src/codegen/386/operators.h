#ifndef MICRO_CODEGEN_386_EXPR_OPERATORS_H
#define MICRO_CODEGEN_386_EXPR_OPERATORS_H

#include "common.h"

int micro_codegen_386__op_plus(size_t start_offset, micro_codegen_386_storage_info_t dst)
{
    if (dst.size == 4) {
        u8 instruction[] = asm_addR32R32(start_offset, start_offset + 1);
        push_instruction(instruction);
    } else
    if (dst.size == 2) {
        u8 instruction[] = asm_addR16R16(start_offset, start_offset + 1);
        push_instruction(instruction);
    } else
    if (dst.size == 1) {
        u8 instruction[] = asm_addR8R8(start_offset, start_offset + 1);
        push_instruction(instruction);
    } else {
        return 1;
    }
    return 0;
}

int micro_codegen_386__op_minus(size_t start_offset, micro_codegen_386_storage_info_t dst)
{
    if (dst.size == 4) {
        u8 instruction[] = asm_subR32R32(start_offset, start_offset + 1);
        push_instruction(instruction);
    } else
    if (dst.size == 2) {
        u8 instruction[] = asm_subR16R16(start_offset, start_offset + 1);
        push_instruction(instruction);
    } else
    if (dst.size == 1) {
        u8 instruction[] = asm_subR8R8(start_offset, start_offset + 1);
        push_instruction(instruction);
    } else {
        return 1;
    }
    return 0;
}

int micro_codegen_386__op_multiply(size_t start_offset, micro_codegen_386_storage_info_t dst)
{
    if (dst.is_unsigned) {
        if (dst.size == 4) {
            if (start_offset) {
                u8 instruction[] = asm_xchgR32R32(0, start_offset);
                push_instruction(instruction);
            }
            u8 instruction[] = asm_mulR32(start_offset + 1);
            push_instruction(instruction);
            if (start_offset) {
                u8 instruction[] = asm_xchgR32R32(start_offset, 0);
                push_instruction(instruction);
            }
        } else
        if (dst.size == 2) {
            if (start_offset) {
                u8 instruction[] = asm_xchgR16R16(0, start_offset);
                push_instruction(instruction);
            }
            u8 instruction[] = asm_mulR16(start_offset + 1);
            push_instruction(instruction);
            if (start_offset) {
                u8 instruction[] = asm_xchgR16R16(start_offset, 0);
                push_instruction(instruction);
            }
        } else
        if (dst.size == 1) {
            if (start_offset) {
                u8 instruction[] = asm_xchgR8R8(0, start_offset);
                push_instruction(instruction);
            }
            u8 instruction[] = asm_mulR8(start_offset + 1);
            push_instruction(instruction);
            if (start_offset) {
                u8 instruction[] = asm_xchgR8R8(start_offset, 0);
                push_instruction(instruction);
            }
        } else {
            return 1;
        }
    } else {
        if (dst.size == 4) {
            u8 instruction[] = asm_imulR32R32(start_offset, start_offset + 1);
            push_instruction(instruction);
        } else
        if (dst.size == 2) {
            u8 instruction[] = asm_imulR16R16(start_offset, start_offset + 1);
            push_instruction(instruction);
        } else
        if (dst.size == 1) {
            u8 instruction[] = asm_imulR8R8(start_offset, start_offset + 1);
            push_instruction(instruction);
        } else {
            return 1;
        }
    }
    return 0;
}

int micro_codegen_386__op_devision(size_t start_offset, micro_codegen_386_storage_info_t dst)
{
    if (start_offset + 1 >= REG32_EDX) {
        u8 instruction[] = asm_pushR32(REG32_EDX);
        push_instruction(instruction);
    }
    if (dst.is_unsigned) {
        if (dst.size == 4) {
            if (start_offset) {
                u8 instruction[] = asm_xchgR32R32(0, start_offset);
                push_instruction(instruction);
            }
            u8 instruction[] = asm_divR32(start_offset + 1);
            push_instruction(instruction);
            if (start_offset) {
                u8 instruction[] = asm_xchgR32R32(start_offset, 0);
                push_instruction(instruction);
            }
        } else
        if (dst.size == 2) {
            if (start_offset) {
                u8 instruction[] = asm_xchgR16R16(0, start_offset);
                push_instruction(instruction);
            }
            u8 instruction[] = asm_divR16(start_offset + 1);
            push_instruction(instruction);
            if (start_offset) {
                u8 instruction[] = asm_xchgR16R16(start_offset, 0);
                push_instruction(instruction);
            }
        } else
        if (dst.size == 1) {
            if (start_offset) {
                u8 instruction[] = asm_xchgR8R8(0, start_offset);
                push_instruction(instruction);
            }
            u8 instruction[] = asm_divR8(start_offset + 1);
            push_instruction(instruction);
            if (start_offset) {
                u8 instruction[] = asm_xchgR8R8(start_offset, 0);
                push_instruction(instruction);
            }
        } else {
            return 1;
        }
    } else {
        if (dst.size == 4) {
            if (start_offset) {
                u8 instruction[] = asm_xchgR32R32(0, start_offset);
                push_instruction(instruction);
            }
            u8 instruction[] = asm_idivR32(start_offset + 1);
            push_instruction(instruction);
            if (start_offset) {
                u8 instruction[] = asm_xchgR32R32(0, start_offset);
                push_instruction(instruction);
            }
        } else
        if (dst.size == 2) {
            if (start_offset) {
                u8 instruction[] = asm_xchgR16R16(0, start_offset);
                push_instruction(instruction);
            }
            u8 instruction[] = asm_idivR16(start_offset + 1);
            push_instruction(instruction);
            if (start_offset) {
                u8 instruction[] = asm_xchgR16R16(start_offset, 0);
                push_instruction(instruction);
            }
        } else
        if (dst.size == 1) {
            if (start_offset) {
                u8 instruction[] = asm_xchgR8R8(0, start_offset);
                push_instruction(instruction);
            }
            u8 instruction[] = asm_idivR8(start_offset + 1);
            push_instruction(instruction);
            if (start_offset) {
                u8 instruction[] = asm_xchgR8R8(start_offset, 0);
                push_instruction(instruction);
            }
        } else {
            return 1;
        }
    }
    if (start_offset + 1 >= REG32_EDX) {
        u8 instruction[] = asm_popR32(REG32_EDX);
        push_instruction(instruction);
    }
    return 0;
}

#endif