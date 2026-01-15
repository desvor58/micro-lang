#ifndef MICRO_CODEGEN_386_EXPR_H
#define MICRO_CODEGEN_386_EXPR_H

#include "common.h"
#include "expr_get_atoms.h"
#include "operators.h"

size_t micro_expr_peek(size_t pos)
{
    if (micro_tokislit(micro_toks[pos]) || micro_toks[pos].type == MICRO_TT_IDENT) {
        return 1;
    } else
    if (micro_tokisop(micro_toks[pos])) {
        size_t expr1_end_offset = micro_expr_peek(pos + 1);
        size_t expr2_end_offset = micro_expr_peek(pos + expr1_end_offset);
        return expr1_end_offset + expr2_end_offset + 1;
    }
    return 0;
}

typedef struct {
    u8 operands_num;
    int (*handler)(size_t, micro_codegen_386_storage_info_t);
} micro_codegen_386_operator_info_t;

micro_codegen_386_operator_info_t micro_codegen_386_ops_table[] = {
    [MICRO_TT_PLUS]  = { .operands_num = 2, .handler = micro_codegen_386__op_plus     },
    [MICRO_TT_MINUS] = { .operands_num = 2, .handler = micro_codegen_386__op_minus    },
    [MICRO_TT_STAR]  = { .operands_num = 2, .handler = micro_codegen_386__op_multiply },
    [MICRO_TT_SLASH] = { .operands_num = 2, .handler = micro_codegen_386__op_division },
    [MICRO_TT_TILDE] = { .operands_num = 1, .handler = micro_codegen_386__op_negate   },
    [MICRO_TT_DOLAR] = { .operands_num = 1, .handler = micro_codegen_386__op_deref    },
};

int micro_codegen_386_expr_parse(size_t pos, micro_codegen_386_storage_info_t dst)
{
    if (micro_toks[pos].type == MICRO_TT_HASH) {
        if (micro_toks[pos + 1].type != MICRO_TT_IDENT) {
            micro_error_t err = {.msg = "Expected identifire", .line_ref = micro_toks[pos].line_ref, .chpos_ref = micro_toks[pos].chpos_ref};
            __micro_push_err(err);
            return 0;
        }
        return __micro_codegen_386_expr_parse_get_ident(pos + 1, dst, 1) + 1;
    }

    if (micro_tokislit(micro_toks[pos])) {
        return __micro_codegen_386_expr_parse_get_lit(pos, dst);
    }
    if (micro_toks[pos].type == MICRO_TT_IDENT) {
        return __micro_codegen_386_expr_parse_get_ident(pos, dst, 0);
    }

    if (!micro_tokisop(micro_toks[pos])) {
        micro_error_t err = {.msg = "Expected operator", .line_ref = micro_toks[pos].line_ref, .chpos_ref = micro_toks[pos].chpos_ref};
        __micro_push_err(err);
        return 0;
    }

    size_t operand_offset = dst.type == MICRO_ST_REG ? dst.offset : 0;    
    micro_codegen_386_operator_info_t op_info = micro_codegen_386_ops_table[micro_toks[pos].type];

    int global_expr_end_offset = 1;
    for (size_t i = 0; i < op_info.operands_num; i++) {
        int expr_end_offset = micro_codegen_386_expr_parse(
            pos + global_expr_end_offset,
            (micro_codegen_386_storage_info_t) {
                .type = MICRO_ST_REG,
                .size = dst.size,
                .offset = operand_offset + i,
                .is_unsigned = dst.is_unsigned
            }
        );
        if (expr_end_offset == 0) {
            return 0;
        }
        global_expr_end_offset += expr_end_offset;
    }

    int op_handle_err = op_info.handler(operand_offset, dst);
    if (op_handle_err) {
        goto err_wrong_dst_type_size;
    }

    if (dst.type == MICRO_ST_DATASEG) {
        u8 dst_addr[4];
        micro_gen32imm_le(dst_addr, dst.offset);

        if (dst.size == 4) {
            u8 instruction[] = asm_movM32R32(dst_addr, operand_offset);
            push_instruction(instruction);
        } else
        if (dst.size == 2) {
            u8 instruction[] = asm_movM16R16(dst_addr, operand_offset);
            push_instruction(instruction);
        } else
        if (dst.size == 1) {
            u8 instruction[] = asm_movM8R8(dst_addr, operand_offset);
            push_instruction(instruction);
        } else {
            goto err_wrong_dst_type_size;
        }
    } else
    if (dst.type == MICRO_ST_REG && dst.offset != operand_offset) {
        reg32 dst_reg = dst.offset;

        if (dst.size == 4) {
            u8 instruction[] = asm_movR32R32(dst_reg, operand_offset);
            push_instruction(instruction);
        } else
        if (dst.size == 2) {
            u8 instruction[] = asm_movR16R16(dst_reg, operand_offset);
            push_instruction(instruction);
        } else
        if (dst.size == 1) {
            u8 instruction[] = asm_movR8R8(dst_reg, operand_offset);
            push_instruction(instruction);
        } else {
            goto err_wrong_dst_type_size;
        }
    } else
    if (dst.type == MICRO_ST_STACK) {
        u8 dst_addr[4];
        micro_gen32imm_le(dst_addr, dst.offset);
        if (dst.size == 4) {
            u8 instruction[] = asm_movS32R32(dst_addr, operand_offset);
            push_instruction(instruction);
        } else
        if (dst.size == 2) {
            u8 instruction[] = asm_movS32R16(dst_addr, operand_offset);
            push_instruction(instruction);
        } else
        if (dst.size == 1) {
            u8 instruction[] = asm_movS32R8(dst_addr, operand_offset);
            push_instruction(instruction);
        } else {
            goto err_wrong_dst_type_size;
        }
    }
    return global_expr_end_offset;

err_wrong_dst_type_size:
    micro_error_t err = {.msg = "Wrong destination type size", .line_ref = micro_toks[pos].line_ref, .chpos_ref = micro_toks[pos].chpos_ref};
    __micro_push_err(err);
    return 0;
}

#endif