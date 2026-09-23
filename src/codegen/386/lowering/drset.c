#include "../internal.h"

int lowering_drset(micro_codegen_t *codegen, micro_instruction_t *instr)
{
    micro_codegen386_ext_t *ext = _micro_codegen386_ext(codegen);

    if (!codegen->flags.no_err_outside_fun && !ext->in_function) {
        micro_push_err((micro_error_t){
            .err = MICRO_ERROR_SET_OUTSIDE_FUNCTION,
            .instr = MICRO_INSTR_DRSET
        });
        return 1;
    }

    micro_instruction_drset_t instr_drset = instr->drset;

    micro_codegen386_ident_t *ptr_ident = sct_hashmap_get(&ext->idents, instr_drset.reg_name);
    if (!ptr_ident || ptr_ident->type != MICRO_IDENT_VREG) {
        micro_push_err((micro_error_t){
            .err = MICRO_ERROR_UNDEFINED_IDENT,
            .instr = MICRO_INSTR_DRSET
        });
        return 1;
    }

    int need_pop_eax = 0;
    int prev_used_eax = ext->used_regs[MICRO_ASM386_REG32_EAX];
    if (prev_used_eax) {
        push_asm_instr(MICRO_ASM386_INSTR_PUSH_R32, operand_reg(MICRO_SIZE_32, MICRO_ASM386_REG32_EAX), operand_none());
        need_pop_eax = 1;
    }
    ext->used_regs[MICRO_ASM386_REG32_EAX] = 1;

    expr_info_t ptr_info = expr_vreg_parse(codegen,
        (micro_codegen386_storage_t){
            .type = MICRO_STORAGE_REG,
            .reg = { .reg = MICRO_ASM386_REG32_EAX, .size = MICRO_SIZE_32 }
        },
        &ptr_ident->vreg
    );

    if (!ptr_info.size) {
        if (need_pop_eax) {
            push_asm_instr(MICRO_ASM386_INSTR_POP_R32, operand_reg(MICRO_SIZE_32, MICRO_ASM386_REG32_EAX), operand_none());
        }
        ext->used_regs[MICRO_ASM386_REG32_EAX] = prev_used_eax;
        return 1;
    }

    int need_pop_edx = 0;
    int prev_used_edx = ext->used_regs[MICRO_ASM386_REG32_EDX];
    if (prev_used_edx) {
        push_asm_instr(MICRO_ASM386_INSTR_PUSH_R32, operand_reg(MICRO_SIZE_32, MICRO_ASM386_REG32_EDX), operand_none());
        need_pop_edx = 1;
    }
    ext->used_regs[MICRO_ASM386_REG32_EDX] = 1;

    micro_codegen386_storage_t edx_dst = {
        .type = MICRO_STORAGE_REG,
        .reg = { .reg = MICRO_ASM386_REG32_EDX, .size = micro_type_to_size[instr_drset.type] }
    };

    expr_info_t val_info = expr_parse(codegen, edx_dst, instr_drset.val_expr);

    if (!val_info.size) {
        if (need_pop_edx) push_asm_instr(MICRO_ASM386_INSTR_POP_R32, operand_reg(MICRO_SIZE_32, MICRO_ASM386_REG32_EDX), operand_none());
        if (need_pop_eax) push_asm_instr(MICRO_ASM386_INSTR_POP_R32, operand_reg(MICRO_SIZE_32, MICRO_ASM386_REG32_EAX), operand_none());
        ext->used_regs[MICRO_ASM386_REG32_EDX] = prev_used_edx;
        ext->used_regs[MICRO_ASM386_REG32_EAX] = prev_used_eax;
        return 1;
    }

    push_asm_instr(
        MICRO_ASM386_INSTR_MOV_MR32_R32,
        operand_reg(MICRO_SIZE_32, MICRO_ASM386_REG32_EAX),
        operand_reg(MICRO_SIZE_32, MICRO_ASM386_REG32_EDX)
    );

    ext->used_regs[MICRO_ASM386_REG32_EDX] = prev_used_edx;
    ext->used_regs[MICRO_ASM386_REG32_EAX] = prev_used_eax;
    if (need_pop_edx) push_asm_instr(MICRO_ASM386_INSTR_POP_R32, operand_reg(MICRO_SIZE_32, MICRO_ASM386_REG32_EDX), operand_none());
    if (need_pop_eax) push_asm_instr(MICRO_ASM386_INSTR_POP_R32, operand_reg(MICRO_SIZE_32, MICRO_ASM386_REG32_EAX), operand_none());

    return 0;
}
