#include "../internal.h"
#include "expr_ops.h"

size_t expr_lit_parse(micro_codegen_t *codegen, micro_codegen386_storage_t dst, i32 imm)
{
    micro_codegen386_ext_t *ext = _micro_codegen386_ext(codegen);

    switch (dst.type) {
        case MICRO_STORAGE_DATASEC: {
            static const micro_asm386_instruction_type_t tbl[] = {
                [MICRO_SIZE_8]  = MICRO_ASM386_INSTR_MOV_M8I8,
                [MICRO_SIZE_16] = MICRO_ASM386_INSTR_MOV_M16I16,
                [MICRO_SIZE_32] = MICRO_ASM386_INSTR_MOV_M32I32,
            };
            push_asm_instr(tbl[dst.datasec.size], { .addr = micro_imm_le_gen(dst.datasec.address) }, { .imm = micro_imm_le_gen(imm) });
        } return 1;
        
        case MICRO_STORAGE_STACK:
            push_asm_instr(MICRO_ASM386_INSTR_MOV_S32I32, { .imm = micro_imm_le_gen(dst.stack.ebp_offset) }, { .imm = micro_imm_le_gen(imm) });
            return 1;

        case MICRO_STORAGE_REG: {
            static const micro_asm386_instruction_type_t tbl[] = {
                [MICRO_SIZE_8]  = MICRO_ASM386_INSTR_MOV_R8I8,
                [MICRO_SIZE_16] = MICRO_ASM386_INSTR_MOV_R16I16,
                [MICRO_SIZE_32] = MICRO_ASM386_INSTR_MOV_R32I32,
            };
            push_asm_instr(tbl[dst.reg.size], { .reg = dst.reg.reg }, { .imm = micro_imm_le_gen(imm) });
        } return 1;
    }
    return 0;
}

size_t expr_lbl_parse(micro_codegen_t *codegen, micro_codegen386_storage_t dst, micro_codegen386_ident_lbl_t *lbl)
{
    micro_codegen386_ext_t *ext = _micro_codegen386_ext(codegen);

    switch (dst.type) {
        case MICRO_STORAGE_DATASEC:
            return 0;

        case MICRO_STORAGE_STACK:
            return 0;

        case MICRO_STORAGE_REG:
            push_asm_instr(MICRO_ASM386_INSTR_MOV_R32L32, { .reg = dst.reg.reg }, { .lbl_name = lbl->name });
            return 1;
    }
    return 0;
}

size_t expr_fun_parse(micro_codegen_t *codegen, micro_codegen386_storage_t dst, micro_codegen386_ident_fun_t *fun)
{
    micro_codegen386_ext_t *ext = _micro_codegen386_ext(codegen);

    switch (dst.type) {
        case MICRO_STORAGE_DATASEC:
            return 0;

        case MICRO_STORAGE_STACK:
            return 0;

        case MICRO_STORAGE_REG:
            push_asm_instr(MICRO_ASM386_INSTR_MOV_R32L32, { .reg = dst.reg.reg }, { .lbl_name = fun->instr_info.name });
            return 1;
    }
    return 0;
}

size_t expr_vreg_parse(micro_codegen_t *codegen, micro_codegen386_storage_t dst, micro_codegen386_ident_vreg_t vreg)
{
    micro_codegen386_ext_t *ext = _micro_codegen386_ext(codegen);

    if (unlikely(vreg.storage.type == MICRO_STORAGE_DATASEC)) {
        return 0;
    }

    int dst_size = 0;
    if (dst.type == MICRO_STORAGE_DATASEC) {
        dst_size = dst.datasec.size;
    } else if (dst.type == MICRO_STORAGE_REG) {
        dst_size = dst.reg.size;
    } else {
        dst_size = MICRO_SIZE_32;
    }

    switch (vreg.storage.type) {
        case MICRO_STORAGE_DATASEC:;
        case MICRO_STORAGE_STACK:
            switch (dst.type) {
                case MICRO_STORAGE_DATASEC:
                    if (ext->used_regs[0]) {
                        push_asm_instr(MICRO_ASM386_INSTR_MOV_S32R32, { .imm = micro_imm_le_gen(ext->ebp_top_offset) }, { .reg = MICRO_ASM386_REG32_EAX });
                        ext->max_stack_offset -= 4;
                    }
                    push_asm_instr(movRS_tbl[dst_size], { .reg = MICRO_ASM386_REG32_EAX }, { .imm = micro_imm_le_gen(vreg.storage.stack.ebp_offset) });
                    push_asm_instr(movMR_tbl[dst_size], { .addr = micro_imm_le_gen(dst.datasec.address) }, { .reg = MICRO_ASM386_REG32_EAX });
                    if (ext->used_regs[0]) {
                        push_asm_instr(MICRO_ASM386_INSTR_MOV_R32S32, { .reg = MICRO_ASM386_REG32_EAX }, { .imm = micro_imm_le_gen(ext->ebp_top_offset) });
                    }
                    return 1;

                case MICRO_STORAGE_STACK:
                    if (ext->used_regs[0]) {
                        push_asm_instr(MICRO_ASM386_INSTR_MOV_S32R32, { .imm = micro_imm_le_gen(ext->ebp_top_offset) }, { .reg = MICRO_ASM386_REG32_EAX });
                        ext->max_stack_offset -= 4;
                    }
                    push_asm_instr(movRS_tbl[dst_size], { .reg = MICRO_ASM386_REG32_EAX }, { .imm = micro_imm_le_gen(vreg.storage.stack.ebp_offset) });
                    push_asm_instr(movSR_tbl[dst_size], { .imm = micro_imm_le_gen(dst.stack.ebp_offset) }, { .reg = MICRO_ASM386_REG32_EAX });
                    if (ext->used_regs[0]) {
                        push_asm_instr(MICRO_ASM386_INSTR_MOV_R32S32, { .reg = MICRO_ASM386_REG32_EAX }, { .imm = micro_imm_le_gen(ext->ebp_top_offset) });
                    }
                    return 1;

                case MICRO_STORAGE_REG:
                    push_asm_instr(movRS_tbl[dst_size], { .reg = dst.reg.reg }, { .imm = micro_imm_le_gen(vreg.storage.stack.ebp_offset) });
                    return 1;
            }
            return 0;

        case MICRO_STORAGE_REG:
            switch (dst.type) {
                case MICRO_STORAGE_DATASEC:
                    push_asm_instr(movMR_tbl[dst_size], { .addr = micro_imm_le_gen(dst.datasec.address) }, { .reg = vreg.storage.reg.reg });
                    return 1;

                case MICRO_STORAGE_STACK:
                    push_asm_instr(movSR_tbl[dst_size], { .imm = micro_imm_le_gen(dst.stack.ebp_offset) }, { .reg = vreg.storage.reg.reg });
                    return 1;

                case MICRO_STORAGE_REG:
                    push_asm_instr(movRR_tbl[dst_size], { .reg = dst.reg.reg }, { .reg = vreg.storage.reg.reg });
                    return 1;
            }
            return 0;
    }
    return 0;
}

size_t expr_parse(micro_codegen_t *codegen, micro_codegen386_storage_t dst, micro_token_t *start)
{
    micro_codegen386_ext_t *ext = _micro_codegen386_ext(codegen);

    u8 used_regs_save[8];
    memcpy(used_regs_save, ext->used_regs, sizeof(u8)*8);

    int res = 0;

    if (dst.type == MICRO_STORAGE_REG) {
        ext->used_regs[dst.reg.reg] = 1;
    }

    if (_micro_tok_is_lit(start->type)) {
        char *end;
        errno = 0;
        i32 lit = strtol(start->val, &end, 10);
        res = expr_lit_parse(codegen, dst, lit);
        goto exit;
    }
    if (start->type == MICRO_TOK_IDENT) {
        micro_codegen386_ident_t *ident = sct_hashmap_get(&ext->idents, start->val);
        if (!ident) {
            micro_push_err((micro_error_t){
                .err = MICRO_ERROR_UNDEFINED_IDENT,
                .line_ref = start->line_ref,
                .chpos_ref = start->chpos_ref
            });
            goto exit;
        }

        switch (ident->type) {
            case MICRO_IDENT_FUN:
                res = expr_fun_parse(codegen, dst, &ident->fun);
                goto exit;

            case MICRO_IDENT_VREG:
                res = expr_vreg_parse(codegen, dst, ident->vreg);
                goto exit;

            case MICRO_IDENT_LBL:
                res = expr_lbl_parse(codegen, dst, &ident->lbl);
                goto exit;
        };
    }
    if (_micro_tok_is_op(start->type)) {
        op_info_t op_info = op_tbl[start->type];
        res = op_info.handler(codegen, dst, start);
        goto exit;
    }

exit:
    memcpy(ext->used_regs, used_regs_save, sizeof(u8)*8);
    return res;
}

int get_last_free_space(micro_codegen_t *codegen)
{
    micro_codegen386_ext_t *ext = _micro_codegen386_ext(codegen);

    for (int i = 0; i < 8; i++) {
        if (!ext->used_regs[i]) {
            if (i > 2) {
                ext->use_callee_save_regs = 1;
            }
            return i;
        }
    }
    return ext->ebp_top_offset;
}
