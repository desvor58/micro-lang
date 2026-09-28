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

    int prev_used_eax = ext->used_regs[MICRO_ASM386_REG32_EAX];
    int prev_used_edx = ext->used_regs[MICRO_ASM386_REG32_EDX];
    int need_pop_eax = 0;
    int need_pop_edx = 0;

    ext->used_regs[MICRO_ASM386_REG32_EAX] = 1;
    ext->used_regs[MICRO_ASM386_REG32_EDX] = 1;

    int val_space = get_last_free_space(codegen);
    micro_codegen386_storage_t val_dst;
    if (val_space < 0) {
        val_dst.type = MICRO_STORAGE_STACK;
        val_dst.stack.ebp_offset = val_space;
        ext->max_stack_offset -= 4;
        ext->ebp_top_offset -= 4;
    } else {
        val_dst.type = MICRO_STORAGE_REG;
        val_dst.reg.reg = val_space;
        val_dst.reg.size = micro_type_to_size[instr_drset.type];
        ext->used_regs[val_space] = 1;
    }

    if (!expr_parse(codegen, val_dst, instr_drset.val_expr).size) goto fail;

    if (prev_used_eax) {
        push_asm_instr(MICRO_ASM386_INSTR_PUSH_R32, operand_reg(MICRO_SIZE_32, MICRO_ASM386_REG32_EAX), operand_none());
        need_pop_eax = 1;
    }

    micro_codegen386_storage_t eax_dst = {
        .type = MICRO_STORAGE_REG,
        .reg = {
            .reg = MICRO_ASM386_REG32_EAX,
            .size = MICRO_SIZE_32,
        },
    };

    if (!expr_vreg_parse(codegen, eax_dst, &ptr_ident->vreg).size) goto fail;

    if (val_dst.type == MICRO_STORAGE_REG) {
        push_asm_instr(
            MICRO_ASM386_INSTR_MOV_MR32_R32,
            operand_reg(MICRO_SIZE_32, MICRO_ASM386_REG32_EAX),
            operand_reg(MICRO_SIZE_32, val_dst.reg.reg)
        );
        ext->used_regs[val_dst.reg.reg] = 0;
    } else {
        if (prev_used_edx) {
            push_asm_instr(MICRO_ASM386_INSTR_PUSH_R32, operand_reg(MICRO_SIZE_32, MICRO_ASM386_REG32_EDX), operand_none());
            need_pop_edx = 1;
        }
        push_asm_instr(MICRO_ASM386_INSTR_MOV_R32S32, operand_reg(MICRO_SIZE_32, MICRO_ASM386_REG32_EDX), operand_imm(MICRO_SIZE_32, micro_imm_le_gen(val_dst.stack.ebp_offset)));
        push_asm_instr(MICRO_ASM386_INSTR_MOV_MR32_R32, operand_reg(MICRO_SIZE_32, MICRO_ASM386_REG32_EAX), operand_reg(MICRO_SIZE_32, MICRO_ASM386_REG32_EDX));
    }

    ext->used_regs[MICRO_ASM386_REG32_EAX] = prev_used_eax;
    ext->used_regs[MICRO_ASM386_REG32_EDX] = prev_used_edx;

    if (need_pop_edx) {
        push_asm_instr(MICRO_ASM386_INSTR_POP_R32, operand_reg(MICRO_SIZE_32, MICRO_ASM386_REG32_EDX), operand_none());
    }
    if (need_pop_eax) {
        push_asm_instr(MICRO_ASM386_INSTR_POP_R32, operand_reg(MICRO_SIZE_32, MICRO_ASM386_REG32_EAX), operand_none());
    }
    return 0;

fail:
    if (val_dst.type == MICRO_STORAGE_REG) {
        ext->used_regs[val_dst.reg.reg] = 0;
    }
    ext->used_regs[MICRO_ASM386_REG32_EAX] = prev_used_eax;
    ext->used_regs[MICRO_ASM386_REG32_EDX] = prev_used_edx;

    if (need_pop_edx) {
        push_asm_instr(MICRO_ASM386_INSTR_POP_R32, operand_reg(MICRO_SIZE_32, MICRO_ASM386_REG32_EDX), operand_none());
    }
    if (need_pop_eax) {
        push_asm_instr(MICRO_ASM386_INSTR_POP_R32, operand_reg(MICRO_SIZE_32, MICRO_ASM386_REG32_EAX), operand_none());
    }
    return 1;
}
