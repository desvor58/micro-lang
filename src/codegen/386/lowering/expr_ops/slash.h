#ifndef MICRO_CODEGEN386_EXPR_OPS_SLASH_H
#define MICRO_CODEGEN386_EXPR_OPS_SLASH_H

#include "common.h"

expr_info_t op_slash_handler(micro_codegen_t *codegen, micro_codegen386_storage_t dst, micro_expr_tok_t *start)
{
    micro_codegen386_ext_t *ext = _micro_codegen386_ext(codegen);

    if (unlikely(start->type != MICRO_EXPR_TOK_SLASH)) {
        return (expr_info_t){ 0, MICRO_TYPE_NULL };
    }

    expr_info_t res;

    int need_pop_eax = 0;
    int need_pop_edx = 0;
    int need_pop_ecx = 0;

    int dst_is_eax = dst.type == MICRO_STORAGE_REG && dst.reg.reg == MICRO_ASM386_REG32_EAX;
    int dst_is_edx = dst.type == MICRO_STORAGE_REG && dst.reg.reg == MICRO_ASM386_REG32_EDX;
    int dst_is_ecx = dst.type == MICRO_STORAGE_REG && dst.reg.reg == MICRO_ASM386_REG32_ECX;
    int prev_used_eax = ext->used_regs[MICRO_ASM386_REG32_EAX];
    int prev_used_edx = ext->used_regs[MICRO_ASM386_REG32_EDX];
    int prev_used_ecx = ext->used_regs[MICRO_ASM386_REG32_ECX];

    if (prev_used_eax && !dst_is_eax) {
        push_asm_instr(MICRO_ASM386_INSTR_PUSH_R32, operand_reg(MICRO_SIZE_32, MICRO_ASM386_REG32_EAX), operand_none());
        need_pop_eax = 1;
    }
    if (prev_used_edx && !dst_is_edx) {
        push_asm_instr(MICRO_ASM386_INSTR_PUSH_R32, operand_reg(MICRO_SIZE_32, MICRO_ASM386_REG32_EDX), operand_none());
        need_pop_edx = 1;
    }
    if (prev_used_ecx && !dst_is_ecx) {
        push_asm_instr(MICRO_ASM386_INSTR_PUSH_R32, operand_reg(MICRO_SIZE_32, MICRO_ASM386_REG32_ECX), operand_none());
        need_pop_ecx = 1;
    }

    ext->used_regs[MICRO_ASM386_REG32_EAX] = 1;
    ext->used_regs[MICRO_ASM386_REG32_EDX] = 1;
    ext->used_regs[MICRO_ASM386_REG32_ECX] = 1;

    micro_codegen386_storage_t eax_dst = {
        .type = MICRO_STORAGE_REG,
        .reg = { .reg = MICRO_ASM386_REG32_EAX, .size = MICRO_SIZE_32 },
    };
    micro_codegen386_storage_t ecx_dst = {
        .type = MICRO_STORAGE_REG,
        .reg = { .reg = MICRO_ASM386_REG32_ECX, .size = MICRO_SIZE_32 },
    };

    micro_expr_tok_t *first_operand = start + 1;
    expr_info_t first_info = expr_parse(codegen, eax_dst, first_operand);
    if (!first_info.size) {
        res = (expr_info_t){ 0, MICRO_TYPE_NULL };
        goto exit;
    }

    micro_expr_tok_t *second_operand = start + first_info.size + 1;
    expr_info_t second_info = expr_parse(codegen, ecx_dst, second_operand);
    if (!second_info.size) {
        res = (expr_info_t){ 0, MICRO_TYPE_NULL };
        goto exit;
    }

    push_asm_instr(MICRO_ASM386_INSTR_CDQ, operand_none(), operand_none());

    push_asm_instr(MICRO_ASM386_INSTR_IDIV_R32, operand_reg(MICRO_SIZE_32, MICRO_ASM386_REG32_ECX), operand_none());

    res = (expr_info_t){ 1 + first_info.size + second_info.size, first_info.type };

exit:
    ext->used_regs[MICRO_ASM386_REG32_EAX] = prev_used_eax;
    ext->used_regs[MICRO_ASM386_REG32_EDX] = prev_used_edx;
    ext->used_regs[MICRO_ASM386_REG32_ECX] = prev_used_ecx;

    if (need_pop_ecx) push_asm_instr(MICRO_ASM386_INSTR_POP_R32, operand_reg(MICRO_SIZE_32, MICRO_ASM386_REG32_ECX), operand_none());
    if (need_pop_edx) push_asm_instr(MICRO_ASM386_INSTR_POP_R32, operand_reg(MICRO_SIZE_32, MICRO_ASM386_REG32_EDX), operand_none());

    switch (dst.type) {
        case MICRO_STORAGE_DATASEC:
            push_asm_instr(movMR_tbl[dst.datasec.size], operand_addr(micro_imm_le_gen(dst.datasec.address)), operand_reg(MICRO_SIZE_32, MICRO_ASM386_REG32_EAX));
            break;
        case MICRO_STORAGE_STACK:
            push_asm_instr(MICRO_ASM386_INSTR_MOV_S32R32, operand_imm(MICRO_SIZE_32, micro_imm_le_gen(dst.stack.ebp_offset)), operand_reg(MICRO_SIZE_32, MICRO_ASM386_REG32_EAX));
            break;
        case MICRO_STORAGE_REG:
            push_asm_instr(movRR_tbl[dst.reg.size], operand_reg(dst.reg.size, dst.reg.reg), operand_reg(MICRO_SIZE_32, MICRO_ASM386_REG32_EAX));
            break;
    }

    if (need_pop_eax) push_asm_instr(MICRO_ASM386_INSTR_POP_R32, operand_reg(MICRO_SIZE_32, MICRO_ASM386_REG32_EAX), operand_none());

    return res;
}

#endif
