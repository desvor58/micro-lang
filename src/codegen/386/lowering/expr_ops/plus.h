#ifndef MICRO_CODEGEN386_EXPR_OPS_PLUS_H
#define MICRO_CODEGEN386_EXPR_OPS_PLUS_H

#include "common.h"

int op_plus_handler(micro_codegen_t *codegen, micro_codegen386_storage_t dst, micro_token_t *start)
{
    micro_codegen386_ext_t *ext = _micro_codegen386_ext(codegen);

    if (unlikely(start->type != MICRO_TOK_PLUS)) {
        return 0;
    }

    opMI_tbl[MICRO_SIZE_8]  = micro_asm386_addM8I8;
    opMI_tbl[MICRO_SIZE_16] = micro_asm386_addM16I16;
    opMI_tbl[MICRO_SIZE_32] = micro_asm386_addM32I32;
    
    opSI_fn = micro_asm386_addS32I32;

    opRI_tbl[MICRO_SIZE_8]  = micro_asm386_addR8I8;
    opRI_tbl[MICRO_SIZE_16] = micro_asm386_addR16I16;
    opRI_tbl[MICRO_SIZE_32] = micro_asm386_addR32I32;

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
            int expr_offset = expr_parse(codegen, dst, second_operand);
            if (!expr_offset) return 0;
            op_lit_to_dst(codegen, dst, first_lit);
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
            opMR_tbl[MICRO_SIZE_8]  = micro_asm386_addM8R8;
            opMR_tbl[MICRO_SIZE_16] = micro_asm386_addM16R16;
            opMR_tbl[MICRO_SIZE_32] = micro_asm386_addM32R32;

            opRR_tbl[MICRO_SIZE_8]  = micro_asm386_addR8R8;
            opRR_tbl[MICRO_SIZE_16] = micro_asm386_addR16R16;
            opRR_tbl[MICRO_SIZE_32] = micro_asm386_addR32R32;

            opRS_tbl[MICRO_SIZE_8]  = micro_asm386_addR8S32;
            opRS_tbl[MICRO_SIZE_16] = micro_asm386_addR16S32;
            opRS_tbl[MICRO_SIZE_32] = micro_asm386_addR32S32;

            opSR_fn = micro_asm386_addS32R32;

            int expr2_offset = op_expr_to_dst(codegen, dst, second_operand);
            if (!expr2_offset) return 0;
            return 1 + expr_offset + expr2_offset;
        }
    }
    return 0;
}

#endif