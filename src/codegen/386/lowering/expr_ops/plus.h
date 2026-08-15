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

int op_plus_handler(micro_codegen_t *codegen, micro_codegen386_storage_t dst, micro_token_t *start)
{
    micro_codegen386_ext_t *ext = _micro_codegen386_ext(codegen);

    if (unlikely(start->type != MICRO_TOK_PLUS)) {
        return 0;
    }

    micro_token_t *first_operand = start + 1;
    if (first_operand->type == MICRO_TOK_IDENT) {
        micro_codegen386_ident_t *ident = sct_hashmap_get(&ext->idents, first_operand->val);

        if (ident->type == MICRO_IDENT_VREG) {
            expr_vreg_parse(codegen, dst, ident->vreg);
        }
        
        micro_token_t *second_operand = start + 2;
        if (_micro_tok_is_lit(second_operand->type)) {
            char *end;
            errno = 0;
            i32 lit = strtol(second_operand->val, &end, 10);

            op_lit_to_dst(codegen, &plus_op_tbls, dst, lit);
            return 3;
        }
        if (_micro_tok_is_op(second_operand->type)) {
            int expr_offset = op_expr_to_dst(codegen, &plus_op_tbls, dst, second_operand);
            if (!expr_offset) return 0;
            return 2 + expr_offset;
        }
        if (second_operand->type == MICRO_TOK_IDENT) {
            micro_codegen386_ident_t *ident2 = sct_hashmap_get(&ext->idents, second_operand->val);

            if (ident2->type == MICRO_IDENT_VREG) {
                op_vreg_to_dst(codegen, &plus_op_tbls, dst, ident2->vreg);
                return 3;
            }
            return 0;
        }
        return 0;
    }
    if (_micro_tok_is_lit(first_operand->type)) {
        char *end;
        errno = 0;
        i32 first_lit = strtol(first_operand->val, &end, 10);

        micro_token_t *second_operand = start + 2;
        if (_micro_tok_is_lit(second_operand->type)) {
            errno = 0;
            i32 second_lit = strtol(second_operand->val, &end, 10);

            expr_lit_parse(codegen, dst, first_lit);

            op_lit_to_dst(codegen, &plus_op_tbls, dst, second_lit);
            return 3;
        }
        if (_micro_tok_is_op(second_operand->type)) {
            int expr_offset = expr_parse(codegen, dst, second_operand);
            if (!expr_offset) return 0;

            op_lit_to_dst(codegen, &plus_op_tbls, dst, first_lit);
            return 2 + expr_offset;
        }
        if (second_operand->type == MICRO_TOK_IDENT) {
            micro_codegen386_ident_t *ident = sct_hashmap_get(&ext->idents, second_operand->val);
            if (ident->type == MICRO_IDENT_VREG) {
                expr_vreg_parse(codegen, dst, ident->vreg);
            }
            op_lit_to_dst(codegen, &plus_op_tbls, dst, first_lit);
            return 3;
        }
        return 0;
    }
    if (_micro_tok_is_op(first_operand->type)) {
        int expr_offset = expr_parse(codegen, dst, first_operand);
        if (!expr_offset) return 0;

        if (dst.type == MICRO_STORAGE_REG) {
            ext->used_regs[dst.reg.reg] = 1;
        }

        micro_token_t *second_operand = start + expr_offset + 1;
        if (_micro_tok_is_lit(second_operand->type)) {
            char *end;
            errno = 0;
            i32 second_lit = strtol(second_operand->val, &end, 10);

            op_lit_to_dst(codegen, &plus_op_tbls, dst, second_lit);
            return 2 + expr_offset;
        }
        if (_micro_tok_is_op(second_operand->type)) {
            int expr2_offset = op_expr_to_dst(codegen, &plus_op_tbls, dst, second_operand);
            if (!expr2_offset) return 0;
            return 1 + expr_offset + expr2_offset;
        }
        if (second_operand->type == MICRO_TOK_IDENT) {
            micro_codegen386_ident_t *ident = sct_hashmap_get(&ext->idents, second_operand->val);
            if (ident->type == MICRO_IDENT_VREG) {
                op_vreg_to_dst(codegen, &plus_op_tbls, dst, ident->vreg);
            }
            return 2 + expr_offset;
        }
    }
    return 0;
}

#endif