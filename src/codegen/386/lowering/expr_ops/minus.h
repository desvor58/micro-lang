#ifndef MICRO_CODEGEN386_EXPR_OPS_MINUS_H
#define MICRO_CODEGEN386_EXPR_OPS_MINUS_H

#include "common.h"

static const op_tbls_t minus_op_tbls = {
    .opMI_tbl[MICRO_SIZE_8]  = MICRO_ASM386_INSTR_SUB_M8I8,
    .opMI_tbl[MICRO_SIZE_16] = MICRO_ASM386_INSTR_SUB_M16I16,
    .opMI_tbl[MICRO_SIZE_32] = MICRO_ASM386_INSTR_SUB_M32I32,
    
    .opSI_fn = MICRO_ASM386_INSTR_SUB_S32I32,

    .opRI_tbl[MICRO_SIZE_8]  = MICRO_ASM386_INSTR_SUB_R8I8,
    .opRI_tbl[MICRO_SIZE_16] = MICRO_ASM386_INSTR_SUB_R16I16,
    .opRI_tbl[MICRO_SIZE_32] = MICRO_ASM386_INSTR_SUB_R32I32,
    
    .opMR_tbl[MICRO_SIZE_8]  = MICRO_ASM386_INSTR_SUB_M8R8,
    .opMR_tbl[MICRO_SIZE_16] = MICRO_ASM386_INSTR_SUB_M16R16,
    .opMR_tbl[MICRO_SIZE_32] = MICRO_ASM386_INSTR_SUB_M32R32,

    .opRR_tbl[MICRO_SIZE_8]  = MICRO_ASM386_INSTR_SUB_R8R8,
    .opRR_tbl[MICRO_SIZE_16] = MICRO_ASM386_INSTR_SUB_R16R16,
    .opRR_tbl[MICRO_SIZE_32] = MICRO_ASM386_INSTR_SUB_R32R32,

    .opRS_tbl[MICRO_SIZE_8]  = MICRO_ASM386_INSTR_SUB_R8S32,
    .opRS_tbl[MICRO_SIZE_16] = MICRO_ASM386_INSTR_SUB_R16S32,
    .opRS_tbl[MICRO_SIZE_32] = MICRO_ASM386_INSTR_SUB_R32S32,

    .opSR_fn = MICRO_ASM386_INSTR_SUB_S32R32,
};

int op_minus_handler(micro_codegen_t *codegen, micro_codegen386_storage_t dst, micro_expr_tok_t *start)
{
    micro_codegen386_ext_t *ext = _micro_codegen386_ext(codegen);

    if (unlikely(start->type != MICRO_EXPR_TOK_MINUS)) {
        return 0;
    }

    micro_expr_tok_t *first_operand = start + 1;
    if (first_operand->type == MICRO_EXPR_TOK_IDENT) {
        micro_codegen386_ident_t *ident = sct_hashmap_get(&ext->idents, first_operand->val);

        if (ident->type == MICRO_IDENT_VREG) {
            int ok = expr_vreg_parse(codegen, dst, &ident->vreg);
            if (!ok) return 0;
        }
        
        micro_expr_tok_t *second_operand = start + 2;
        if (_micro_expr_is_lit(second_operand->type)) {
            char *end;
            errno = 0;
            i32 lit = strtol(second_operand->val, &end, 10);

            op_lit_to_dst(codegen, &minus_op_tbls, dst, lit);
            return 3;
        }
        if (_micro_expr_is_op(second_operand->type)) {
            int expr_size = op_expr_to_dst(codegen, &minus_op_tbls, dst, second_operand);
            if (!expr_size) return 0;
            return 2 + expr_size;
        }
        if (second_operand->type == MICRO_EXPR_TOK_IDENT) {
            micro_codegen386_ident_t *ident2 = sct_hashmap_get(&ext->idents, second_operand->val);

            if (ident2->type == MICRO_IDENT_VREG) {
                op_vreg_to_dst(codegen, &minus_op_tbls, dst, ident2->vreg);
                return 3;
            }
            return 0;
        }
        return 0;
    }
    if (_micro_expr_is_lit(first_operand->type)) {
        char *end;
        errno = 0;
        i32 first_lit = strtol(first_operand->val, &end, 10);

        micro_expr_tok_t *second_operand = start + 2;
        if (_micro_expr_is_lit(second_operand->type)) {
            errno = 0;
            i32 second_lit = strtol(second_operand->val, &end, 10);

            int ok = expr_lit_parse(codegen, dst, first_lit);
            if (!ok) return 0;

            op_lit_to_dst(codegen, &minus_op_tbls, dst, second_lit);
            return 3;
        }
        if (_micro_expr_is_op(second_operand->type)) {
            expr_lit_parse(codegen, dst, first_lit);

            int expr_size = op_expr_to_dst(codegen, &minus_op_tbls, dst, second_operand);
            if (!expr_size) return 0;
            
            return 2 + expr_size;
        }
        if (second_operand->type == MICRO_EXPR_TOK_IDENT) {
            int ok = expr_lit_parse(codegen, dst, first_lit);
            if (!ok) return 0;

            micro_codegen386_ident_t *ident = sct_hashmap_get(&ext->idents, second_operand->val);
            if (ident->type == MICRO_IDENT_VREG) {
                op_vreg_to_dst(codegen, &minus_op_tbls, dst, ident->vreg);
            }
            return 3;
        }
        return 0;
    }
    if (_micro_expr_is_op(first_operand->type)) {
        int expr_size = expr_parse(codegen, dst, first_operand);
        if (!expr_size) return 0;

        if (dst.type == MICRO_STORAGE_REG) {
            ext->used_regs[dst.reg.reg] = 1;
        }

        micro_expr_tok_t *second_operand = start + expr_size + 1;
        if (_micro_expr_is_lit(second_operand->type)) {
            char *end;
            errno = 0;
            i32 second_lit = strtol(second_operand->val, &end, 10);

            op_lit_to_dst(codegen, &minus_op_tbls, dst, second_lit);
            return 2 + expr_size;
        }
        if (_micro_expr_is_op(second_operand->type)) {
            int expr2_size = op_expr_to_dst(codegen, &minus_op_tbls, dst, second_operand);
            if (!expr2_size) return 0;
            return 1 + expr_size + expr2_size;
        }
        if (second_operand->type == MICRO_EXPR_TOK_IDENT) {
            micro_codegen386_ident_t *ident = sct_hashmap_get(&ext->idents, second_operand->val);
            if (ident->type == MICRO_IDENT_VREG) {
                op_vreg_to_dst(codegen, &minus_op_tbls, dst, ident->vreg);
            }
            return 2 + expr_size;
        }
    }
    return 0;
}

#endif