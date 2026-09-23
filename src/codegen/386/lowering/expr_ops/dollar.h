#ifndef MICRO_CODEGEN386_EXPR_OPS_DOLLAR_H
#define MICRO_CODEGEN386_EXPR_OPS_DOLLAR_H

#include "common.h"

expr_info_t op_dollar_handler(micro_codegen_t *codegen, micro_codegen386_storage_t dst, micro_expr_tok_t *start)
{
    micro_codegen386_ext_t *ext = _micro_codegen386_ext(codegen);

    if (unlikely(start->type != MICRO_EXPR_TOK_DOLLAR)) {
        return (expr_info_t){ 0, MICRO_TYPE_NULL };
    }

    expr_info_t res;

    int need_pop_eax = 0;
    int need_pop_edx = 0;
    int dst_is_eax = dst.type == MICRO_STORAGE_REG && dst.reg.reg == MICRO_ASM386_REG32_EAX;
    int dst_is_edx = dst.type == MICRO_STORAGE_REG && dst.reg.reg == MICRO_ASM386_REG32_EDX;
    if (ext->used_regs[MICRO_ASM386_REG32_EAX] && !dst_is_eax) {
        push_asm_instr(MICRO_ASM386_INSTR_PUSH_R32, operand_reg(MICRO_SIZE_32, MICRO_ASM386_REG32_EAX), operand_none());
        need_pop_eax = 1;
    }
    if (ext->used_regs[MICRO_ASM386_REG32_EDX] && !dst_is_edx) {
        push_asm_instr(MICRO_ASM386_INSTR_PUSH_R32, operand_reg(MICRO_SIZE_32, MICRO_ASM386_REG32_EDX), operand_none());
        need_pop_edx = 1;
    }

    micro_codegen386_storage_t eax_dst = {
        .type = MICRO_STORAGE_REG,
        .reg = {
            .reg = MICRO_ASM386_REG32_EAX,
            .size = MICRO_SIZE_32,
        },
    };

    micro_expr_tok_t *operand = start + 1;

    if (operand->type == MICRO_EXPR_TOK_IDENT) {
        micro_codegen386_ident_t *ident = sct_hashmap_get(&ext->idents, operand->val);
        if (!ident || ident->type != MICRO_IDENT_VREG) {
            res = (expr_info_t){ 0, MICRO_TYPE_NULL };
            goto exit;
        }
        expr_info_t expr_info = expr_vreg_parse(codegen, eax_dst, &ident->vreg);
        if (!expr_info.size) {
            res = (expr_info_t){ 0, MICRO_TYPE_NULL };
            goto exit;
        }
    } else if (_micro_expr_is_lit(operand->type)) {
        char *end;
        errno = 0;
        i32 lit = strtol(operand->val, &end, 10);
        expr_lit_parse(codegen, eax_dst, lit);
    } else if (_micro_expr_is_op(operand->type)) {
        expr_info_t expr_info = expr_parse(codegen, eax_dst, operand);
        if (!expr_info.size) {
            res = (expr_info_t){ 0, MICRO_TYPE_NULL };
            goto exit;
        }
    } else {
        res = (expr_info_t){ 0, MICRO_TYPE_NULL };
        goto exit;
    }

    push_asm_instr(MICRO_ASM386_INSTR_MOV_R32MR32, operand_reg(MICRO_SIZE_32, MICRO_ASM386_REG32_EAX), operand_reg(MICRO_SIZE_32, MICRO_ASM386_REG32_EAX));

    res = (expr_info_t){ 2, MICRO_TYPE_I32 };

exit:
    if (need_pop_edx) {
        push_asm_instr(MICRO_ASM386_INSTR_POP_R32, operand_reg(MICRO_SIZE_32, MICRO_ASM386_REG32_EDX), operand_none());
    }

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

    if (need_pop_eax) {
        push_asm_instr(MICRO_ASM386_INSTR_POP_R32, operand_reg(MICRO_SIZE_32, MICRO_ASM386_REG32_EAX), operand_none());
    }
    return res;
}

#endif
