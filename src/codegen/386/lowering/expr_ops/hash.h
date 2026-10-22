#ifndef MICRO_CODEGEN386_EXPR_OPS_HASH_H
#define MICRO_CODEGEN386_EXPR_OPS_HASH_H

#include "common.h"

expr_info_t op_hash_handler(micro_codegen_t *codegen, micro_codegen386_storage_t dst, micro_expr_tok_t *start)
{
    micro_codegen386_ext_t *ext = _micro_codegen386_ext(codegen);

    if (unlikely(start->type != MICRO_EXPR_TOK_HASH)) {
        return (expr_info_t){ 0, MICRO_TYPE_NULL };
    }

    micro_expr_tok_t *operand = start + 1;
    if (operand->type != MICRO_EXPR_TOK_IDENT) {
        micro_push_err((micro_error_t){
            .err = MICRO_ERROR_EXPR_PARSE,
            .instr = ((micro_instruction_t*)sct_vector_get(codegen->instrs, codegen->pos))->type,
        });
        return (expr_info_t){ 0, MICRO_TYPE_NULL };
    }

    micro_codegen386_ident_t *ident = sct_hashmap_get(&ext->idents, operand->val);
    if (!ident) {
        micro_push_err((micro_error_t){
            .err = MICRO_ERROR_UNDEFINED_IDENT,
            .instr = ((micro_instruction_t*)sct_vector_get(codegen->instrs, codegen->pos))->type,
        });
        return (expr_info_t){ 0, MICRO_TYPE_NULL };
    }
    if (ident->type != MICRO_IDENT_VREG) {
        micro_push_err((micro_error_t){
            .err = MICRO_ERROR_IDENT_NOT_VREG,
            .instr = ((micro_instruction_t*)sct_vector_get(codegen->instrs, codegen->pos))->type,
        });
        return (expr_info_t){ 0, MICRO_TYPE_NULL };
    }

    micro_codegen386_ident_vreg_t *vreg = &ident->vreg;

    if (vreg->storage.type == MICRO_STORAGE_REG) {
        ptrdiff_t slot = ext->ebp_top_offset;
        ext->max_stack_offset -= 4;
        ext->ebp_top_offset -= 4;

        push_asm_instr(MICRO_ASM386_INSTR_MOV_S32R32,
                       operand_imm(MICRO_SIZE_32, micro_imm_le_gen(slot)),
                       operand_reg(MICRO_SIZE_32, vreg->storage.reg.reg));

        if (dst.type != MICRO_STORAGE_REG || dst.reg.reg != vreg->storage.reg.reg) {
            ext->regs_to_free |= 1u << vreg->storage.reg.reg;
        }

        vreg->storage.type = MICRO_STORAGE_STACK;
        vreg->storage.stack.ebp_offset = slot;
    }

    if (vreg->storage.type == MICRO_STORAGE_DATASEC) {
        expr_lit_parse(codegen, dst, (i32)vreg->storage.datasec.address);
        return (expr_info_t){ 2, MICRO_TYPE_PTR };
    }

    ptrdiff_t disp = vreg->storage.stack.ebp_offset;
    micro_asm386_instruction_type_t lea = MICRO_ASM386_INSTR_LEA_R32SIBI32;
    if (disp >= -128 && disp <= 127) {
        lea = MICRO_ASM386_INSTR_LEA_R32SIBI8;
    }

    if (dst.type == MICRO_STORAGE_REG) {
        push_asm_instr_with_sib(lea,
                                operand_reg(MICRO_SIZE_32, dst.reg.reg),
                                operand_imm(MICRO_SIZE_32, micro_imm_le_gen(disp)),
                                1,
                                MICRO_ASM386_REG32_NO_INDEX,
                                MICRO_ASM386_REG32_EBP);
        return (expr_info_t){ 2, MICRO_TYPE_PTR };
    }

    int need_pop_eax = 0;
    if (ext->used_regs[MICRO_ASM386_REG32_EAX]) {
        push_asm_instr(MICRO_ASM386_INSTR_PUSH_R32, operand_reg(MICRO_SIZE_32, MICRO_ASM386_REG32_EAX), operand_none());
        need_pop_eax = 1;
    }

    push_asm_instr_with_sib(lea,
                            operand_reg(MICRO_SIZE_32, MICRO_ASM386_REG32_EAX),
                            operand_imm(MICRO_SIZE_32, micro_imm_le_gen(disp)),
                            1,
                            MICRO_ASM386_REG32_NO_INDEX,
                            MICRO_ASM386_REG32_EBP);

    switch (dst.type) {
        case MICRO_STORAGE_DATASEC:
            push_asm_instr(movMR_tbl[dst.datasec.size], operand_addr(micro_imm_le_gen(dst.datasec.address)), operand_reg(MICRO_SIZE_32, MICRO_ASM386_REG32_EAX));
            break;

        case MICRO_STORAGE_STACK:
            push_asm_instr(MICRO_ASM386_INSTR_MOV_S32R32, operand_imm(MICRO_SIZE_32, micro_imm_le_gen(dst.stack.ebp_offset)), operand_reg(MICRO_SIZE_32, MICRO_ASM386_REG32_EAX));
            break;

        case MICRO_STORAGE_REG:
            break;
    }

    if (need_pop_eax) {
        push_asm_instr(MICRO_ASM386_INSTR_POP_R32, operand_reg(MICRO_SIZE_32, MICRO_ASM386_REG32_EAX), operand_none());
    }

    return (expr_info_t){ 2, MICRO_TYPE_PTR };
}

#endif
