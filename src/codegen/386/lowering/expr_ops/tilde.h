#ifndef MICRO_CODEGEN386_EXPR_OPS_TILDE_H
#define MICRO_CODEGEN386_EXPR_OPS_TILDE_H

#include "common.h"

static const unary_op_tbls_t tilde_op_tbls = {
    .R_tbl[MICRO_SIZE_8]  = MICRO_ASM386_INSTR_NEG_R8,
    .R_tbl[MICRO_SIZE_16] = MICRO_ASM386_INSTR_NEG_R16,
    .R_tbl[MICRO_SIZE_32] = MICRO_ASM386_INSTR_NEG_R32,

    .M_tbl[MICRO_SIZE_8]  = MICRO_ASM386_INSTR_NEG_M8,
    .M_tbl[MICRO_SIZE_16] = MICRO_ASM386_INSTR_NEG_M16,
    .M_tbl[MICRO_SIZE_32] = MICRO_ASM386_INSTR_NEG_M32,

    .S_fn = MICRO_ASM386_INSTR_NEG_S32,
};

expr_info_t op_tilde_handler(micro_codegen_t *codegen, micro_codegen386_storage_t dst, micro_expr_tok_t *start)
{
    if (unlikely(start->type != MICRO_EXPR_TOK_TILDE)) {
        return (expr_info_t){ 0, MICRO_TYPE_NULL };
    }

    expr_info_t expr_info = op_unary_operand(codegen, dst, start);
    if (!expr_info.size) {
        return (expr_info_t){ 0, MICRO_TYPE_NULL };
    }

    op_unary_to_dst(codegen, &tilde_op_tbls, dst);
    return expr_info;
}

#endif
