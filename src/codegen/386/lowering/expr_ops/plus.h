#ifndef MICRO_CODEGEN386_EXPR_OPS_PLUS_H
#define MICRO_CODEGEN386_EXPR_OPS_PLUS_H

#include "common.h"

static const op_tbls_t plus_op_tbls = {
    .opMI_tbl[MICRO_SIZE_8] = MICRO_ASM386_INSTR_ADD_M8I8,
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

expr_info_t op_plus_handler(micro_codegen_t *codegen, micro_codegen386_storage_t dst, micro_expr_tok_t *start)
{
    micro_codegen386_ext_t *ext = _micro_codegen386_ext(codegen);

    if (unlikely(start->type != MICRO_EXPR_TOK_PLUS)) {
        return (expr_info_t){ 0, MICRO_TYPE_NULL };
    }

    {
        micro_codegen386_ident_t *i1 = 0, *i2 = 0;
        i32 n_add = 0, n_mul = 0;
        if (lea_match_add_scaled(codegen, start, &i1, &n_add, &i2, &n_mul) &&
            !code_selection_lea(codegen, dst, i1, i2, n_add, n_mul)) {
            return (expr_info_t){ 5, (i1 ? i1 : i2)->vreg.type };
        }
    }

    micro_expr_tok_t *first_operand = start + 1;
    if (first_operand->type == MICRO_EXPR_TOK_IDENT) {
        micro_codegen386_ident_t *ident = sct_hashmap_get(&ext->idents, first_operand->val);
        if (!ident) {
            micro_push_err((micro_error_t){
                .err = MICRO_ERROR_UNDEFINED_IDENT,
                .instr = ((micro_instruction_t*)sct_vector_get(codegen->instrs, codegen->pos))->type,
            });
            return (expr_info_t){ 0, MICRO_TYPE_NULL };
        }
        
        micro_expr_tok_t *second_operand = start + 2;
        if (_micro_expr_is_lit(second_operand->type)) {
            char *end;
            errno = 0;
            i32 lit = strtol(second_operand->val, &end, 10);

            if (ident->type == MICRO_IDENT_VREG) {
                expr_info_t expr_info = expr_vreg_parse(codegen, dst, &ident->vreg);
                if (!expr_info.size) return (expr_info_t){ 0, MICRO_TYPE_NULL };
            }

            op_lit_to_dst(codegen, &plus_op_tbls, dst, lit);
            return (expr_info_t){ 3, ident->vreg.type };
        }

        if (ident->type == MICRO_IDENT_VREG) {
            expr_info_t expr_info = expr_vreg_parse(codegen, dst, &ident->vreg);
            if (!expr_info.size) return (expr_info_t){ 0, MICRO_TYPE_NULL };
        }

        if (_micro_expr_is_op(second_operand->type)) {
            expr_info_t expr_info = op_expr_to_dst(codegen, &plus_op_tbls, dst, second_operand);
            if (!expr_info.size) return (expr_info_t){ 0, MICRO_TYPE_NULL };
            return (expr_info_t){ 2 + expr_info.size, ident->vreg.type };
        }
        if (second_operand->type == MICRO_EXPR_TOK_IDENT) {
            micro_codegen386_ident_t *ident2 = sct_hashmap_get(&ext->idents, second_operand->val);
            if (!ident2) {
                micro_push_err((micro_error_t){
                    .err = MICRO_ERROR_UNDEFINED_IDENT,
                    .instr = ((micro_instruction_t*)sct_vector_get(codegen->instrs, codegen->pos))->type,
                });
                return (expr_info_t){ 0, MICRO_TYPE_NULL };
            }

            if (ident2->type == MICRO_IDENT_VREG) {
                op_vreg_to_dst(codegen, &plus_op_tbls, dst, ident2->vreg);
                return (expr_info_t){ 3, ident->vreg.type };
            }
            return (expr_info_t){ 0, MICRO_TYPE_NULL };
        }
        return (expr_info_t){ 0, MICRO_TYPE_NULL };
    }
    if (_micro_expr_is_lit(first_operand->type)) {
        char *end;
        errno = 0;
        i32 first_lit = strtol(first_operand->val, &end, 10);

        micro_expr_tok_t *second_operand = start + 2;
        if (_micro_expr_is_lit(second_operand->type)) {
            errno = 0;
            i32 second_lit = strtol(second_operand->val, &end, 10);

            expr_info_t first_info = expr_lit_parse(codegen, dst, first_lit);
            if (!first_info.size) return (expr_info_t){ 0, MICRO_TYPE_NULL };

            op_lit_to_dst(codegen, &plus_op_tbls, dst, second_lit);
            return (expr_info_t){ 3, first_info.type };
        }
        if (_micro_expr_is_op(second_operand->type)) {
            expr_info_t expr_info = expr_parse(codegen, dst, second_operand);
            if (!expr_info.size) return (expr_info_t){ 0, MICRO_TYPE_NULL };

            op_lit_to_dst(codegen, &plus_op_tbls, dst, first_lit);
            return (expr_info_t){ 2 + expr_info.size, (first_lit < 0) ? MICRO_TYPE_I32 : MICRO_TYPE_U32 };
        }
        if (second_operand->type == MICRO_EXPR_TOK_IDENT) {
            micro_codegen386_ident_t *ident = sct_hashmap_get(&ext->idents, second_operand->val);
            if (!ident) {
                micro_push_err((micro_error_t){
                    .err = MICRO_ERROR_UNDEFINED_IDENT,
                    .instr = ((micro_instruction_t*)sct_vector_get(codegen->instrs, codegen->pos))->type,
                });
                return (expr_info_t){ 0, MICRO_TYPE_NULL };
            }

            expr_info_t expr_info = (expr_info_t){ 0, MICRO_TYPE_NULL };
            if (ident->type == MICRO_IDENT_VREG) {
                expr_info = expr_vreg_parse(codegen, dst, &ident->vreg);
                if (!expr_info.size) return (expr_info_t){ 0, MICRO_TYPE_NULL };
            }
            op_lit_to_dst(codegen, &plus_op_tbls, dst, first_lit);
            return (expr_info_t){ 3, expr_info.type };
        }
        return (expr_info_t){ 0, MICRO_TYPE_NULL };
    }
    if (_micro_expr_is_op(first_operand->type)) {
        if (first_operand->type == MICRO_EXPR_TOK_STAR) {
            micro_codegen386_ident_t *i2;
            i32 n_mul;
            if (lea_match_scale_mul(codegen, start + 2, start + 3, &i2, &n_mul)) {
                micro_expr_tok_t *addend = start + 4;
                i32 n_add = 0;
                micro_codegen386_ident_t *i1 = 0;
                if (addend->type == MICRO_EXPR_TOK_LIT_INT) {
                    n_add = strtol(addend->val, NULL, 10);
                } else
                if (addend->type == MICRO_EXPR_TOK_IDENT) {
                    i1 = sct_hashmap_get(&ext->idents, addend->val);
                    if (!i1 || i1->type != MICRO_IDENT_VREG) i1 = 0;
                }
                if ((i1 || n_add) && !code_selection_lea(codegen, dst, i1, i2, n_add, n_mul)) {
                    return (expr_info_t){ 5, (i1 ? i1 : i2)->vreg.type };
                }
            }
        } else
        if (first_operand->type == MICRO_EXPR_TOK_PLUS && (start + 6)->type == MICRO_EXPR_TOK_LIT_INT) {
            micro_codegen386_ident_t *i1 = 0, *i2 = 0;
            i32 n_add = 0, n_mul = 0;
            i32 disp = strtol((start + 6)->val, NULL, 10);
            if (lea_match_add_scaled(codegen, first_operand, &i1, &n_add, &i2, &n_mul) &&
                lea_disp_add_ok(n_add, disp) &&
                !code_selection_lea(codegen, dst, i1, i2, n_add + disp, n_mul)) {
                return (expr_info_t){ 7, (i1 ? i1 : i2)->vreg.type };
            }
        }

        expr_info_t expr_info = expr_parse(codegen, dst, first_operand);
        if (!expr_info.size) return (expr_info_t){ 0, MICRO_TYPE_NULL };

        if (dst.type == MICRO_STORAGE_REG) {
            ext->used_regs[dst.reg.reg] = 1;
        }

        micro_expr_tok_t *second_operand = start + expr_info.size + 1;
        if (_micro_expr_is_lit(second_operand->type)) {
            char *end;
            errno = 0;
            i32 second_lit = strtol(second_operand->val, &end, 10);

            op_lit_to_dst(codegen, &plus_op_tbls, dst, second_lit);
            return (expr_info_t){ 2 + expr_info.size, expr_info.type };
        }
        if (_micro_expr_is_op(second_operand->type)) {
            expr_info_t expr2_info = op_expr_to_dst(codegen, &plus_op_tbls, dst, second_operand);
            if (!expr2_info.size) return (expr_info_t){ 0, MICRO_TYPE_NULL };
            return (expr_info_t){ 1 + expr_info.size + expr2_info.size, expr_info.type };
        }
        if (second_operand->type == MICRO_EXPR_TOK_IDENT) {
            micro_codegen386_ident_t *ident = sct_hashmap_get(&ext->idents, second_operand->val);
            if (!ident) {
                micro_push_err((micro_error_t){
                    .err = MICRO_ERROR_UNDEFINED_IDENT,
                    .instr = ((micro_instruction_t*)sct_vector_get(codegen->instrs, codegen->pos))->type,
                });
                return (expr_info_t){ 0, MICRO_TYPE_NULL };
            }

            if (ident->type == MICRO_IDENT_VREG) {
                op_vreg_to_dst(codegen, &plus_op_tbls, dst, ident->vreg);
            }
            return (expr_info_t){ 2 + expr_info.size, expr_info.type };
        }
    }
    return (expr_info_t){ 0, MICRO_TYPE_NULL };
}

#endif