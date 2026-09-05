#include "../internal.h"
#include "expr_ops.h"

expr_info_t expr_lit_parse(micro_codegen_t *codegen, micro_codegen386_storage_t dst, i32 imm)
{
    micro_codegen386_ext_t *ext = _micro_codegen386_ext(codegen);

    switch (dst.type) {
        case MICRO_STORAGE_DATASEC: {
            static const micro_asm386_instruction_type_t tbl[] = {
                [MICRO_SIZE_8]  = MICRO_ASM386_INSTR_MOV_M8I8,
                [MICRO_SIZE_16] = MICRO_ASM386_INSTR_MOV_M16I16,
                [MICRO_SIZE_32] = MICRO_ASM386_INSTR_MOV_M32I32,
            };
            push_asm_instr(tbl[dst.datasec.size], operand_addr(micro_imm_le_gen(dst.datasec.address)), operand_imm(MICRO_SIZE_32, micro_imm_le_gen(imm)));
        } break;
        
        case MICRO_STORAGE_STACK:
            push_asm_instr(MICRO_ASM386_INSTR_MOV_S32I32, operand_imm(MICRO_SIZE_32, micro_imm_le_gen(dst.stack.ebp_offset)), operand_imm(MICRO_SIZE_32, micro_imm_le_gen(imm)));
            break;

        case MICRO_STORAGE_REG: {
            static const micro_asm386_instruction_type_t tbl[] = {
                [MICRO_SIZE_8]  = MICRO_ASM386_INSTR_MOV_R8I8,
                [MICRO_SIZE_16] = MICRO_ASM386_INSTR_MOV_R16I16,
                [MICRO_SIZE_32] = MICRO_ASM386_INSTR_MOV_R32I32,
            };
            push_asm_instr(tbl[dst.reg.size], operand_reg(dst.reg.size, dst.reg.reg), operand_imm(MICRO_SIZE_32, micro_imm_le_gen(imm)));
        } break;
    }
    if (imm < 0) {
        return (expr_info_t){
            .size = 1,
            .type = MICRO_TYPE_I32,
        };
    }
    return (expr_info_t){
        .size = 1,
        .type = MICRO_TYPE_U32,
    };
}

expr_info_t expr_lbl_parse(micro_codegen_t *codegen, micro_codegen386_storage_t dst, micro_codegen386_ident_lbl_t *lbl)
{
    micro_codegen386_ext_t *ext = _micro_codegen386_ext(codegen);

    switch (dst.type) {
        case MICRO_STORAGE_DATASEC:
            return (expr_info_t){ 0, MICRO_TYPE_NULL };

        case MICRO_STORAGE_STACK:
            return (expr_info_t){ 0, MICRO_TYPE_NULL };

        case MICRO_STORAGE_REG:
            push_asm_instr(MICRO_ASM386_INSTR_MOV_R32L32, operand_reg(MICRO_SIZE_32, dst.reg.reg), operand_lbl(MICRO_SIZE_32, lbl->name));
            return (expr_info_t){ 1, MICRO_TYPE_U32 };
    }
    return (expr_info_t){ 0, MICRO_TYPE_NULL };
}

expr_info_t expr_fun_parse(micro_codegen_t *codegen, micro_codegen386_storage_t dst, micro_codegen386_ident_fun_t *fun)
{
    micro_codegen386_ext_t *ext = _micro_codegen386_ext(codegen);

    switch (dst.type) {
        case MICRO_STORAGE_DATASEC:
            return (expr_info_t){ 0, MICRO_TYPE_NULL };

        case MICRO_STORAGE_STACK:
            return (expr_info_t){ 0, MICRO_TYPE_NULL };

        case MICRO_STORAGE_REG:
            push_asm_instr(MICRO_ASM386_INSTR_MOV_R32L32, operand_reg(MICRO_SIZE_32, dst.reg.reg), operand_lbl(MICRO_SIZE_32, fun->instr_info.name));
            return (expr_info_t){ 1, MICRO_TYPE_U32 };
    }
    return (expr_info_t){ 0, MICRO_TYPE_NULL };
}

expr_info_t expr_vreg_parse(micro_codegen_t *codegen, micro_codegen386_storage_t dst, micro_codegen386_ident_vreg_t *vreg)
{
    micro_codegen386_ext_t *ext = _micro_codegen386_ext(codegen);

    if (unlikely(vreg->storage.type == MICRO_STORAGE_DATASEC)) {
        return (expr_info_t){ 0, MICRO_TYPE_NULL };
    }

    int dst_size = 0;
    if (dst.type == MICRO_STORAGE_DATASEC) {
        dst_size = dst.datasec.size;
    } else if (dst.type == MICRO_STORAGE_REG) {
        dst_size = dst.reg.size;
    } else {
        dst_size = MICRO_SIZE_32;
    }

    switch (vreg->storage.type) {
        case MICRO_STORAGE_DATASEC:;
        case MICRO_STORAGE_STACK:
            switch (dst.type) {
                case MICRO_STORAGE_DATASEC:
                    if (ext->used_regs[0]) {
                        push_asm_instr(MICRO_ASM386_INSTR_MOV_S32R32, operand_imm(MICRO_SIZE_32, micro_imm_le_gen(ext->ebp_top_offset)), operand_reg(MICRO_SIZE_32, MICRO_ASM386_REG32_EAX));
                        ext->max_stack_offset -= 4;
                    }
                    push_asm_instr(movRS_tbl[dst_size], operand_reg(MICRO_SIZE_32, MICRO_ASM386_REG32_EAX), operand_imm(MICRO_SIZE_32, micro_imm_le_gen(vreg->storage.stack.ebp_offset)));
                    push_asm_instr(movMR_tbl[dst_size], operand_addr(micro_imm_le_gen(dst.datasec.address)), operand_reg(MICRO_SIZE_32, MICRO_ASM386_REG32_EAX));
                    if (ext->used_regs[0]) {
                        push_asm_instr(MICRO_ASM386_INSTR_MOV_R32S32, operand_reg(MICRO_SIZE_32, MICRO_ASM386_REG32_EAX), operand_imm(MICRO_SIZE_32, micro_imm_le_gen(ext->ebp_top_offset)));
                    }
                    return (expr_info_t){ 1, vreg->type };

                case MICRO_STORAGE_STACK:
                    if (ext->used_regs[0]) {
                        push_asm_instr(MICRO_ASM386_INSTR_MOV_S32R32, operand_imm(MICRO_SIZE_32, micro_imm_le_gen(ext->ebp_top_offset)), operand_reg(MICRO_SIZE_32, MICRO_ASM386_REG32_EAX));
                        ext->max_stack_offset -= 4;
                    }
                    push_asm_instr(movRS_tbl[dst_size], operand_reg(MICRO_SIZE_32, MICRO_ASM386_REG32_EAX), operand_imm(MICRO_SIZE_32, micro_imm_le_gen(vreg->storage.stack.ebp_offset)));
                    push_asm_instr(movSR_tbl[dst_size], operand_imm(MICRO_SIZE_32, micro_imm_le_gen(dst.stack.ebp_offset)), operand_reg(MICRO_SIZE_32, MICRO_ASM386_REG32_EAX));
                    if (ext->used_regs[0]) {
                        push_asm_instr(MICRO_ASM386_INSTR_MOV_R32S32, operand_reg(MICRO_SIZE_32, MICRO_ASM386_REG32_EAX), operand_imm(MICRO_SIZE_32, micro_imm_le_gen(ext->ebp_top_offset)));
                    }
                    return (expr_info_t){ 1, vreg->type };

                case MICRO_STORAGE_REG:
                    push_asm_instr(movRS_tbl[dst_size], operand_reg(dst.reg.size, dst.reg.reg), operand_imm(MICRO_SIZE_32, micro_imm_le_gen(vreg->storage.stack.ebp_offset)));
                    return (expr_info_t){ 1, vreg->type };
            }
            return (expr_info_t){ 0, MICRO_TYPE_NULL };

        case MICRO_STORAGE_REG:
            switch (dst.type) {
                case MICRO_STORAGE_DATASEC:
                    push_asm_instr(movMR_tbl[dst_size], operand_addr(micro_imm_le_gen(dst.datasec.address)), operand_reg(vreg->storage.reg.size, vreg->storage.reg.reg));
                    return (expr_info_t){ 1, vreg->type };

                case MICRO_STORAGE_STACK:
                    push_asm_instr(movSR_tbl[dst_size], operand_imm(MICRO_SIZE_32, micro_imm_le_gen(dst.stack.ebp_offset)), operand_reg(vreg->storage.reg.size, vreg->storage.reg.reg));
                    return (expr_info_t){ 1, vreg->type };

                case MICRO_STORAGE_REG:
                    push_asm_instr(movRR_tbl[dst_size], operand_reg(dst.reg.size, dst.reg.reg), operand_reg(vreg->storage.reg.size, vreg->storage.reg.reg));
                    return (expr_info_t){ 1, vreg->type };
            }
            return (expr_info_t){ 0, MICRO_TYPE_NULL };
    }
    return (expr_info_t){ 0, MICRO_TYPE_NULL };
}

static micro_asm386_instruction_type_t get_set_instr(micro_expr_tok_t *tok, micro_type_t expr_type)
{
    micro_asm386_instruction_type_t instr = MICRO_ASM386_INSTR_JNZ_L32;

    if (tok->type == MICRO_EXPR_TOK_EXCLAMATION) {
        instr = get_set_instr(tok + 1, expr_type);
        instr = (micro_asm386_instruction_type_t[]){
            [MICRO_ASM386_INSTR_SETZ_R8]   = MICRO_ASM386_INSTR_SETNZ_R8,
            [MICRO_ASM386_INSTR_SETNZ_R8]  = MICRO_ASM386_INSTR_SETZ_R8,
            [MICRO_ASM386_INSTR_SETL_R8]   = MICRO_ASM386_INSTR_SETNL_R8,
            [MICRO_ASM386_INSTR_SETNL_R8]  = MICRO_ASM386_INSTR_SETL_R8,
            [MICRO_ASM386_INSTR_SETG_R8]   = MICRO_ASM386_INSTR_SETNG_R8,
            [MICRO_ASM386_INSTR_SETNG_R8]  = MICRO_ASM386_INSTR_SETG_R8,
            [MICRO_ASM386_INSTR_SETB_R8]   = MICRO_ASM386_INSTR_SETNB_R8,
            [MICRO_ASM386_INSTR_SETNB_R8]  = MICRO_ASM386_INSTR_SETB_R8,
            [MICRO_ASM386_INSTR_SETA_R8]   = MICRO_ASM386_INSTR_SETNA_R8,
            [MICRO_ASM386_INSTR_SETNA_R8]  = MICRO_ASM386_INSTR_SETA_R8,
            [MICRO_ASM386_INSTR_SETLE_R8]  = MICRO_ASM386_INSTR_SETNLE_R8,
            [MICRO_ASM386_INSTR_SETNLE_R8] = MICRO_ASM386_INSTR_SETLE_R8,
            [MICRO_ASM386_INSTR_SETGE_R8]  = MICRO_ASM386_INSTR_SETNGE_R8,
            [MICRO_ASM386_INSTR_SETNGE_R8] = MICRO_ASM386_INSTR_SETGE_R8,
            [MICRO_ASM386_INSTR_SETBE_R8]  = MICRO_ASM386_INSTR_SETNBE_R8,
            [MICRO_ASM386_INSTR_SETNBE_R8] = MICRO_ASM386_INSTR_SETBE_R8,
            [MICRO_ASM386_INSTR_SETAE_R8]  = MICRO_ASM386_INSTR_SETNAE_R8,
            [MICRO_ASM386_INSTR_SETNAE_R8] = MICRO_ASM386_INSTR_SETAE_R8,
        }[instr];
        return instr;
    } else
    if (tok->type == MICRO_EXPR_TOK_EQ) {
        instr = MICRO_ASM386_INSTR_SETZ_R8;
    } else
    if (tok->type == MICRO_EXPR_TOK_LESS) {
        if (micro_type_is_unsigned(expr_type)) {
            instr = MICRO_ASM386_INSTR_SETB_R8;
        } else {
            instr = MICRO_ASM386_INSTR_SETL_R8;
        }
    } else
    if (tok->type == MICRO_EXPR_TOK_GREAT) {
        if (micro_type_is_unsigned(expr_type)) {
            instr = MICRO_ASM386_INSTR_SETA_R8;
        } else {
            instr = MICRO_ASM386_INSTR_SETG_R8;
        }
    } else
    if (tok->type == MICRO_EXPR_TOK_LESS_OR_EQ) {
        if (micro_type_is_unsigned(expr_type)) {
            instr = MICRO_ASM386_INSTR_SETBE_R8;
        } else {
            instr = MICRO_ASM386_INSTR_SETLE_R8;
        }
    } else
    if (tok->type == MICRO_EXPR_TOK_GREAT_OR_EQ) {
        if (micro_type_is_unsigned(expr_type)) {
            instr = MICRO_ASM386_INSTR_SETAE_R8;
        } else {
            instr = MICRO_ASM386_INSTR_SETGE_R8;
        }
    }
    return instr;
}

expr_info_t expr_parse(micro_codegen_t *codegen, micro_codegen386_storage_t dst, micro_expr_tok_t *start)
{
    micro_codegen386_ext_t *ext = _micro_codegen386_ext(codegen);

    u8 used_regs_save[8];
    memcpy(used_regs_save, ext->used_regs, sizeof(u8)*8);

    expr_info_t res = { 0, MICRO_TYPE_NULL };

    if (dst.type == MICRO_STORAGE_REG) {
        ext->used_regs[dst.reg.reg] = 1;
    }

    if (start->type == MICRO_EXPR_TOK_LIT_INT) {
        char *end;
        errno = 0;
        i32 lit = strtol(start->val, &end, 10);
        res = expr_lit_parse(codegen, dst, lit);
        goto exit;
    }
    if (start->type == MICRO_EXPR_TOK_IDENT) {
        micro_codegen386_ident_t *ident = sct_hashmap_get(&ext->idents, start->val);
        if (!ident) {
            micro_push_err((micro_error_t){
                .err = MICRO_ERROR_UNDEFINED_IDENT,
            });
            goto exit;
        }

        switch (ident->type) {
            case MICRO_IDENT_FUN:
                res = expr_fun_parse(codegen, dst, &ident->fun);
                goto exit;

            case MICRO_IDENT_VREG:
                res = expr_vreg_parse(codegen, dst, &ident->vreg);
                goto exit;

            case MICRO_IDENT_LBL:
                res = expr_lbl_parse(codegen, dst, &ident->lbl);
                goto exit;
        };
    }
    if (_micro_expr_is_op(start->type)) {
        op_info_t op_info = op_tbl[start->type];
        res = op_info.handler(codegen, dst, start);
        if (op_info.is_cond) {
            micro_asm386_instruction_type_t set_instr = get_set_instr(start, res.type);

            if (dst.type == MICRO_STORAGE_REG) {
                push_asm_instr(set_instr, operand_reg(dst.reg.size, dst.reg.reg), operand_none());
                goto exit;
            }

            int need_pop_eax = 0;
            int free_reg = get_last_free_space(codegen);
            if (free_reg < 0) {
                need_pop_eax = 1;
                push_asm_instr(MICRO_ASM386_INSTR_PUSH_R32, operand_reg(MICRO_SIZE_32, MICRO_ASM386_REG32_EAX), operand_none());
                free_reg = 0;
            }

            push_asm_instr(set_instr, operand_reg(MICRO_SIZE_32, free_reg), operand_none());

            if (dst.type == MICRO_STORAGE_DATASEC) {
                push_asm_instr(MICRO_ASM386_INSTR_MOV_M8R8, operand_imm(MICRO_SIZE_32, micro_imm_le_gen(dst.datasec.address)), operand_reg(MICRO_SIZE_32, free_reg));
            } else {
                push_asm_instr(MICRO_ASM386_INSTR_MOV_S32R8, operand_imm(MICRO_SIZE_32, micro_imm_le_gen(dst.stack.ebp_offset)), operand_reg(MICRO_SIZE_32, free_reg));
            }

            if (need_pop_eax) {
                push_asm_instr(MICRO_ASM386_INSTR_POP_R32, operand_reg(MICRO_SIZE_32, MICRO_ASM386_REG32_EAX), operand_none());
            }
        }
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

expr_info_t cond_expr_vreg_parse(micro_codegen_t *codegen, micro_codegen386_ident_vreg_t *vreg)
{
    micro_codegen386_ext_t *ext = _micro_codegen386_ext(codegen);

    if (unlikely(vreg->storage.type == MICRO_STORAGE_DATASEC)) {
        return (expr_info_t){ 0, MICRO_TYPE_NULL };
    }
    
    if (vreg->storage.type == MICRO_STORAGE_STACK) {
        push_asm_instr(MICRO_ASM386_INSTR_CMP_S32I32, operand_imm(MICRO_SIZE_32, micro_imm_le_gen(vreg->storage.stack.ebp_offset)), operand_imm(MICRO_SIZE_32, micro_imm_le_gen(0)));
    } else
    if (vreg->storage.type == MICRO_STORAGE_REG) {
        push_asm_instr(testRR_tbl[vreg->storage.reg.size], operand_reg(vreg->storage.reg.size, vreg->storage.reg.reg), operand_reg(vreg->storage.reg.size, vreg->storage.reg.reg));
    }
    return (expr_info_t){ 1, vreg->type };
}

expr_info_t cond_expr_parse(micro_codegen_t *codegen, micro_expr_tok_t *start)
{
    micro_codegen386_ext_t *ext = _micro_codegen386_ext(codegen);

    expr_info_t res = (expr_info_t){ 0, MICRO_TYPE_NULL };
    
    if (start->type == MICRO_EXPR_TOK_LIT_INT) {

    } else
    if (start->type == MICRO_EXPR_TOK_IDENT) {
        micro_codegen386_ident_t *ident = sct_hashmap_get(&ext->idents, start->val);
        if (!ident) {
            micro_push_err((micro_error_t){
                .err = MICRO_ERROR_UNDEFINED_IDENT,
            });
            goto exit;
        }

        if (ident->type == MICRO_IDENT_VREG) {
            res = cond_expr_vreg_parse(codegen, &ident->vreg);
            goto exit;
        }
    } else
    if (_micro_expr_is_op(start->type)) {
        int free_space = get_last_free_space(codegen);
        micro_codegen386_storage_t dst;
        if (free_space < 0) {
            dst.type = MICRO_STORAGE_STACK;
            dst.stack.ebp_offset = free_space;
        } else {
            dst.type = MICRO_STORAGE_REG;
            dst.reg.reg = free_space;
            dst.reg.size = MICRO_SIZE_32;
        }
        op_info_t op_info = op_tbl[start->type];
        res = op_info.handler(codegen, dst, start);
        if (!op_info.is_cond) {
            if (dst.type == MICRO_STORAGE_STACK) {
                push_asm_instr(MICRO_ASM386_INSTR_CMP_S32I32, operand_imm(MICRO_SIZE_32, micro_imm_le_gen(dst.stack.ebp_offset)), operand_imm(MICRO_SIZE_32, micro_imm_le_gen(0)));
            } else
            if (dst.type == MICRO_STORAGE_REG) {
                push_asm_instr(MICRO_ASM386_INSTR_TEST_R32R32, operand_reg(MICRO_SIZE_32, dst.reg.reg), operand_reg(MICRO_SIZE_32, dst.reg.reg));
            }
        }
        goto exit;
    }

exit:
    return res;
}
