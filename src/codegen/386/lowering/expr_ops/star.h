#ifndef MICRO_CODEGEN386_EXPR_OPS_STAR_H
#define MICRO_CODEGEN386_EXPR_OPS_STAR_H

#include "common.h"

static const op_tbls_t star_op_tbls = {
    .opMI_tbl[MICRO_SIZE_8]  = MICRO_ASM386_INSTR_ADD_M8I8,
    .opMI_tbl[MICRO_SIZE_16] = MICRO_ASM386_INSTR_ADD_M16I16,
    .opMI_tbl[MICRO_SIZE_32] = MICRO_ASM386_INSTR_ADD_M32I32,

    .opSI_fn = MICRO_ASM386_INSTR_ADD_S32I32,

    .opRI_tbl[MICRO_SIZE_8]  = MICRO_ASM386_INSTR_ADD_R8I8,
    .opRI_tbl[MICRO_SIZE_16] = MICRO_ASM386_INSTR_ADD_R16I16,
    .opRI_tbl[MICRO_SIZE_32] = MICRO_ASM386_INSTR_ADD_R32I32,

    .opMR_tbl[MICRO_SIZE_8]  = MICRO_ASM386_INSTR_ADD_M8R8,
    .opMR_tbl[MICRO_SIZE_16] = MICRO_ASM386_INSTR_ADD_M16R16,
    .opMR_tbl[MICRO_SIZE_32] = MICRO_ASM386_INSTR_ADD_M32R32,

    .opRR_tbl[MICRO_SIZE_8]  = MICRO_ASM386_INSTR_ADD_R8R8,
    .opRR_tbl[MICRO_SIZE_16] = MICRO_ASM386_INSTR_ADD_R16R16,
    .opRR_tbl[MICRO_SIZE_32] = MICRO_ASM386_INSTR_ADD_R32R32,

    .opRS_tbl[MICRO_SIZE_8]  = MICRO_ASM386_INSTR_ADD_R8S32,
    .opRS_tbl[MICRO_SIZE_16] = MICRO_ASM386_INSTR_ADD_R16S32,
    .opRS_tbl[MICRO_SIZE_32] = MICRO_ASM386_INSTR_ADD_R32S32,

    .opSR_fn = MICRO_ASM386_INSTR_ADD_S32R32,
};

expr_info_t op_star_handler(micro_codegen_t *codegen, micro_codegen386_storage_t dst, micro_expr_tok_t *start)
{
    micro_codegen386_ext_t *ext = _micro_codegen386_ext(codegen);

    if (unlikely(start->type != MICRO_EXPR_TOK_STAR)) {
        return (expr_info_t){ 0, MICRO_TYPE_NULL };
    }

    expr_info_t res;

    int need_pop_eax = 0;
    int need_pop_edx = 0;
    if (ext->used_regs[MICRO_ASM386_REG32_EAX]) {
        push_asm_instr(MICRO_ASM386_INSTR_PUSH_R32, { .reg = MICRO_ASM386_REG32_EAX }, {});
        need_pop_eax = 1;
    }
    if (ext->used_regs[MICRO_ASM386_REG32_EDX]) {
        push_asm_instr(MICRO_ASM386_INSTR_PUSH_R32, { .reg = MICRO_ASM386_REG32_EDX }, {});
        need_pop_edx = 1;
    }

    micro_codegen386_storage_t eax_dst = {
        .type = MICRO_STORAGE_REG,
        .reg = {
            .reg = MICRO_ASM386_REG32_EAX,
            .size = MICRO_SIZE_32,
        },
    };

    micro_codegen386_storage_t edx_dst = {
        .type = MICRO_STORAGE_REG,
        .reg = {
            .reg = MICRO_ASM386_REG32_EDX,
            .size = MICRO_SIZE_32,
        },
    };

    micro_expr_tok_t *first_operand = start + 1;
    if (first_operand->type == MICRO_EXPR_TOK_IDENT) {
        micro_codegen386_ident_t *ident = sct_hashmap_get(&ext->idents, first_operand->val);
        if (!ident) {
            micro_push_err((micro_error_t){
                .err = MICRO_ERROR_UNDEFINED_IDENT,
                .instr = ((micro_instruction_t*)sct_vector_get(codegen->instrs, codegen->pos))->type,
            });
            res = (expr_info_t){ 0, MICRO_TYPE_NULL };
            goto exit;
        }

        if (ident->type == MICRO_IDENT_VREG) {
            expr_info_t expr_info = expr_vreg_parse(codegen, eax_dst, &ident->vreg);
            if (!expr_info.size) {
                res = (expr_info_t){ 0, MICRO_TYPE_NULL };
                goto exit;
            }
        }
        
        micro_expr_tok_t *second_operand = start + 2;
        if (_micro_expr_is_lit(second_operand->type)) {
            char *end;
            errno = 0;
            i32 lit = strtol(second_operand->val, &end, 10);

            push_asm_instr(MICRO_ASM386_INSTR_IMUL_R32I32, { .reg = MICRO_ASM386_REG32_EAX }, { .imm = micro_imm_le_gen(lit) });
            res = (expr_info_t){ 3, ident->vreg.type };
            goto exit;
        }
        if (_micro_expr_is_op(second_operand->type)) {
            expr_info_t expr_info = expr_parse(codegen, edx_dst, second_operand);
            if (!expr_info.size) {
                res = (expr_info_t){ 0, MICRO_TYPE_NULL };
                goto exit;
            }
            push_asm_instr(MICRO_ASM386_INSTR_IMUL_R32, { .reg = MICRO_ASM386_REG32_EDX }, {});
            res = (expr_info_t){ 2 + expr_info.size, ident->vreg.type };
            goto exit;
        }
        if (second_operand->type == MICRO_EXPR_TOK_IDENT) {
            micro_codegen386_ident_t *ident2 = sct_hashmap_get(&ext->idents, second_operand->val);
            if (!ident2) {
                micro_push_err((micro_error_t){
                    .err = MICRO_ERROR_UNDEFINED_IDENT,
                    .instr = ((micro_instruction_t*)sct_vector_get(codegen->instrs, codegen->pos))->type,
                });
                res = (expr_info_t){ 0, MICRO_TYPE_NULL };
                goto exit;
            }

            if (ident2->type == MICRO_IDENT_VREG) {
                expr_info_t expr_info = expr_vreg_parse(codegen, edx_dst, &ident2->vreg);
                if (!expr_info.size) {
                    res = (expr_info_t){ 0, MICRO_TYPE_NULL };
                    goto exit;
                }
                push_asm_instr(MICRO_ASM386_INSTR_IMUL_R32, { .reg = MICRO_ASM386_REG32_EDX }, {});
                res = (expr_info_t){ 3, ident->vreg.type };
                goto exit;
            }
        }
        res = (expr_info_t){ 0, MICRO_TYPE_NULL };
        goto exit;
    }
    if (_micro_expr_is_lit(first_operand->type)) {
        char *end;
        errno = 0;
        i32 first_lit = strtol(first_operand->val, &end, 10);

        micro_expr_tok_t *second_operand = start + 2;
        if (_micro_expr_is_lit(second_operand->type)) {
            errno = 0;
            i32 second_lit = strtol(second_operand->val, &end, 10);

            expr_info_t first_info = expr_lit_parse(codegen, eax_dst, first_lit);
            if (!first_info.size) {
                res = (expr_info_t){ 0, MICRO_TYPE_NULL };
                goto exit;
            }
            push_asm_instr(MICRO_ASM386_INSTR_IMUL_R32I32, { .reg = MICRO_ASM386_REG32_EAX }, { .imm = micro_imm_le_gen(second_lit) });
            res = (expr_info_t){ 3, first_info.type };
            goto exit;
        }
        if (_micro_expr_is_op(second_operand->type)) {
            expr_info_t expr_info = expr_parse(codegen, eax_dst, second_operand);
            if (!expr_info.size) {
                res = (expr_info_t){ 0, MICRO_TYPE_NULL };
                goto exit;
            }

            push_asm_instr(MICRO_ASM386_INSTR_IMUL_R32I32, { .reg = MICRO_ASM386_REG32_EAX }, { .imm = micro_imm_le_gen(first_lit) });
            res = (expr_info_t){ 2 + expr_info.size, (first_lit < 0) ? MICRO_TYPE_I32 : MICRO_TYPE_U32 };
            goto exit;
        }
        if (second_operand->type == MICRO_EXPR_TOK_IDENT) {
            micro_codegen386_ident_t *ident = sct_hashmap_get(&ext->idents, second_operand->val);
            if (!ident) {
                micro_push_err((micro_error_t){
                    .err = MICRO_ERROR_UNDEFINED_IDENT,
                    .instr = ((micro_instruction_t*)sct_vector_get(codegen->instrs, codegen->pos))->type,
                });
                res = (expr_info_t){ 0, MICRO_TYPE_NULL };
                goto exit;
            }

            expr_info_t expr_info = (expr_info_t){ 0, MICRO_TYPE_NULL };
            if (ident->type == MICRO_IDENT_VREG) {
                expr_info = expr_vreg_parse(codegen, eax_dst, &ident->vreg);
                if (!expr_info.size) {
                    res = (expr_info_t){ 0, MICRO_TYPE_NULL };
                    goto exit;
                }
            }
            push_asm_instr(MICRO_ASM386_INSTR_IMUL_R32I32, { .reg = MICRO_ASM386_REG32_EAX }, { .imm = micro_imm_le_gen(first_lit) });
            res = (expr_info_t){ 3, expr_info.type };
            goto exit;
        }
        res = (expr_info_t){ 0, MICRO_TYPE_NULL };
        goto exit;
    }
    if (_micro_expr_is_op(first_operand->type)) {
        expr_info_t expr_info = expr_parse(codegen, eax_dst, first_operand);
        if (!expr_info.size) {
            res = (expr_info_t){ 0, MICRO_TYPE_NULL };
            goto exit;
        }

        if (dst.type == MICRO_STORAGE_REG) {
            ext->used_regs[dst.reg.reg] = 1;
        }

        micro_expr_tok_t *second_operand = start + expr_info.size + 1;
        if (_micro_expr_is_lit(second_operand->type)) {
            char *end;
            errno = 0;
            i32 second_lit = strtol(second_operand->val, &end, 10);

            push_asm_instr(MICRO_ASM386_INSTR_IMUL_R32I32, { .reg = MICRO_ASM386_REG32_EAX }, { .imm = micro_imm_le_gen(second_lit) });
            res = (expr_info_t){ 2 + expr_info.size, expr_info.type };
            goto exit;
        }
        if (_micro_expr_is_op(second_operand->type)) {
            expr_info_t expr2_info = expr_parse(codegen, edx_dst, second_operand);
            if (!expr2_info.size) {
                res = (expr_info_t){ 0, MICRO_TYPE_NULL };
                goto exit;
            }
            push_asm_instr(MICRO_ASM386_INSTR_IMUL_R32, { .reg = MICRO_ASM386_REG32_EDX }, {});
            res = (expr_info_t){ 1 + expr_info.size + expr2_info.size, expr_info.type };
            goto exit;
        }
        if (second_operand->type == MICRO_EXPR_TOK_IDENT) {
            micro_codegen386_ident_t *ident = sct_hashmap_get(&ext->idents, second_operand->val);
            if (!ident) {
                micro_push_err((micro_error_t){
                    .err = MICRO_ERROR_UNDEFINED_IDENT,
                    .instr = ((micro_instruction_t*)sct_vector_get(codegen->instrs, codegen->pos))->type,
                });
                res = (expr_info_t){ 0, MICRO_TYPE_NULL };
                goto exit;
            }

            if (ident->type == MICRO_IDENT_VREG) {
                expr_vreg_parse(codegen, edx_dst, &ident->vreg);
            }
            push_asm_instr(MICRO_ASM386_INSTR_IMUL_R32, { .reg = MICRO_ASM386_REG32_EDX }, {});
            res = (expr_info_t){ 2 + expr_info.size, expr_info.type };
            goto exit;
        }
    }
    res = (expr_info_t){ 0, MICRO_TYPE_NULL };

exit:
    switch (dst.type) {
        case MICRO_STORAGE_DATASEC:
            push_asm_instr(movMR_tbl[dst.datasec.size], { .addr = micro_imm_le_gen(dst.datasec.address) }, { .reg = MICRO_ASM386_REG32_EAX });
            break;
            
        case MICRO_STORAGE_STACK:
            push_asm_instr(MICRO_ASM386_INSTR_MOV_S32R32, { .imm = micro_imm_le_gen(dst.stack.ebp_offset) }, { .reg = MICRO_ASM386_REG32_EAX });
            break;
            
        case MICRO_STORAGE_REG:
            push_asm_instr(movRR_tbl[dst.reg.size], { .reg = dst.reg.reg }, { .reg = MICRO_ASM386_REG32_EAX });
            break;
    }
    if (need_pop_eax) {
        push_asm_instr(MICRO_ASM386_INSTR_POP_R32, { .reg = MICRO_ASM386_REG32_EAX }, {});
    }
    if (need_pop_edx) {
        push_asm_instr(MICRO_ASM386_INSTR_POP_R32, { .reg = MICRO_ASM386_REG32_EDX }, {});
    }
    return res;
}

#endif