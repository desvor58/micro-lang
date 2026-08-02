#ifndef MICRO_CODEGEN386_EXPR_OPS_MINUS_H
#define MICRO_CODEGEN386_EXPR_OPS_MINUS_H

#include "common.h"

int op_minus_handler(micro_codegen_t *codegen, micro_codegen386_storage_t dst, micro_token_t *start)
{
    micro_codegen386_ext_t *ext = _micro_codegen386_ext(codegen);

    if (unlikely(start->type != MICRO_TOK_MINUS)) {
        return 0;
    }

    opMI_tbl[MICRO_SIZE_8]  = micro_asm386_subM8I8;
    opMI_tbl[MICRO_SIZE_16] = micro_asm386_subM16I16;
    opMI_tbl[MICRO_SIZE_32] = micro_asm386_subM32I32;
    
    opSI_fn = micro_asm386_subS32I32;

    opRI_tbl[MICRO_SIZE_8]  = micro_asm386_subR8I8;
    opRI_tbl[MICRO_SIZE_16] = micro_asm386_subR16I16;
    opRI_tbl[MICRO_SIZE_32] = micro_asm386_subR32I32;
    
    opMR_tbl[MICRO_SIZE_8]  = micro_asm386_subM8R8;
    opMR_tbl[MICRO_SIZE_16] = micro_asm386_subM16R16;
    opMR_tbl[MICRO_SIZE_32] = micro_asm386_subM32R32;

    opRR_tbl[MICRO_SIZE_8]  = micro_asm386_subR8R8;
    opRR_tbl[MICRO_SIZE_16] = micro_asm386_subR16R16;
    opRR_tbl[MICRO_SIZE_32] = micro_asm386_subR32R32;

    opRS_tbl[MICRO_SIZE_8]  = micro_asm386_subR8S32;
    opRS_tbl[MICRO_SIZE_16] = micro_asm386_subR16S32;
    opRS_tbl[MICRO_SIZE_32] = micro_asm386_subR32S32;

    opSR_fn = micro_asm386_subS32R32;

    micro_token_t *first_operand = start + 1;
    if (_micro_tok_is_lit(first_operand->type)) {
        char *end;
        errno = 0;
        i32 first_lit = strtol(first_operand->val, &end, 10);

        micro_token_t *second_operand = start + 2;
        if (_micro_tok_is_lit(second_operand->type)) {
            errno = 0;
            i32 second_lit = strtol(second_operand->val, &end, 10);

            expr_lit_parse(codegen, dst, first_lit);

            op_lit_to_dst(codegen, dst, second_lit);
            return 3;
        }
        if (_micro_tok_is_op(second_operand->type)) {
            expr_lit_parse(codegen, dst, first_lit);

            int expr_offset = op_expr_to_dst(codegen, dst, second_operand);
            if (!expr_offset) return 0;
            
            return 2 + expr_offset;
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

            op_lit_to_dst(codegen, dst, second_lit);
            return 2 + expr_offset;
        }
        if (_micro_tok_is_op(second_operand->type)) {
            int expr2_offset = op_expr_to_dst(codegen, dst, second_operand);
            if (!expr2_offset) return 0;
            return 1 + expr_offset + expr2_offset;
        }
    }
    return 0;
}

#endif