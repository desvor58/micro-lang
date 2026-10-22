#ifndef MICRO_CODEGEN386_EXPR_OPS_APOSTROPHE_H
#define MICRO_CODEGEN386_EXPR_OPS_APOSTROPHE_H

#include "common.h"

static const unary_op_tbls_t apostrophe_op_tbls = {
    .R_tbl[MICRO_SIZE_8]  = MICRO_ASM386_INSTR_NOT_R8,
    .R_tbl[MICRO_SIZE_16] = MICRO_ASM386_INSTR_NOT_R16,
    .R_tbl[MICRO_SIZE_32] = MICRO_ASM386_INSTR_NOT_R32,

    .M_tbl[MICRO_SIZE_8]  = MICRO_ASM386_INSTR_NOT_M8,
    .M_tbl[MICRO_SIZE_16] = MICRO_ASM386_INSTR_NOT_M16,
    .M_tbl[MICRO_SIZE_32] = MICRO_ASM386_INSTR_NOT_M32,

    .S_fn = MICRO_ASM386_INSTR_NOT_S32,
};

expr_info_t op_apostrophe_handler(micro_codegen_t *codegen, micro_codegen386_storage_t dst, micro_expr_tok_t *start)
{
    if (unlikely(start->type != MICRO_EXPR_TOK_APOSTROPHE)) {
        return (expr_info_t){ 0, MICRO_TYPE_NULL };
    }

    expr_info_t expr_info = op_unary_operand(codegen, dst, start);
    if (!expr_info.size) {
        return (expr_info_t){ 0, MICRO_TYPE_NULL };
    }

    op_unary_to_dst(codegen, &apostrophe_op_tbls, dst);
    return expr_info;
}

#endif
