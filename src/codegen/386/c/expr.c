#include "../expr.h"

micro_codegen_386_operator_info_t micro_codegen_386_ops_table[] = {
    [MICRO_TT_PLUS]   = { .operands_num = 2, .handler = micro_codegen_386__op_plus     },
    [MICRO_TT_MINUS]  = { .operands_num = 2, .handler = micro_codegen_386__op_minus    },
    [MICRO_TT_STAR]   = { .operands_num = 2, .handler = micro_codegen_386__op_multiply },
    [MICRO_TT_SLASH]  = { .operands_num = 2, .handler = micro_codegen_386__op_division },
    [MICRO_TT_TILDE]  = { .operands_num = 1, .handler = micro_codegen_386__op_negate   },
    [MICRO_TT_DOLLAR] = { .operands_num = 1, .handler = micro_codegen_386__op_deref    },
};

size_t micro_expr_peek(micro_codegen_t *codegen, size_t pos)
{
    if (micro_tokislit(codegen->toks->toks[pos]) || codegen->toks->toks[pos].type == MICRO_TT_IDENT) {
        return 1;
    } else
    if (micro_tokisop(codegen->toks->toks[pos])) {
        size_t expr1_end_offset = micro_expr_peek(codegen, pos + 1);
        size_t expr2_end_offset = micro_expr_peek(codegen, pos + expr1_end_offset);
        return expr1_end_offset + expr2_end_offset + 1;
    }
    return 0;
}

// returns offset to next instruction after expr
// if returns 0 - err
int micro_codegen_386_expr_parse(micro_codegen_t *codegen, size_t pos, micro_codegen_386_storage_info_t dst)
{
    if (codegen->toks->toks[pos].type == MICRO_TT_HASH) {
        if (codegen->toks->toks[pos + 1].type != MICRO_TT_IDENT) {
            micro_push_err((micro_error_t){
                .msg = "Expected identifier",
                .line_ref = codegen->toks->toks[pos].line_ref,
                .chpos_ref = codegen->toks->toks[pos].chpos_ref
            });
            return 0;
        }
        return __micro_codegen_386_expr_parse_get_ident(codegen, pos + 1, dst, 1) + 1;
    }

    if (micro_tokislit(codegen->toks->toks[pos])) {
        return __micro_codegen_386_expr_parse_get_lit(codegen, pos, dst);
    }
    if (codegen->toks->toks[pos].type == MICRO_TT_IDENT) {
        return __micro_codegen_386_expr_parse_get_ident(codegen, pos, dst, 0);
    }

    if (!micro_tokisop(codegen->toks->toks[pos])) {
        micro_push_err((micro_error_t){
            .msg = "Expected operator",
            .line_ref = codegen->toks->toks[pos].line_ref,
            .chpos_ref = codegen->toks->toks[pos].chpos_ref
        });
        return 0;
    }

    size_t operand_offset = dst.type == MICRO_ST_REG ? dst.offset : 0;    
    micro_codegen_386_operator_info_t op_info = micro_codegen_386_ops_table[codegen->toks->toks[pos].type];

    int global_expr_end_offset = 1;
    for (size_t i = 0; i < op_info.operands_num; i++) {
        int expr_end_offset = micro_codegen_386_expr_parse(
            codegen,
            pos + global_expr_end_offset,
            (micro_codegen_386_storage_info_t){
                .type = MICRO_ST_REG,
                .size = dst.size,
                .offset = operand_offset + i,
                .is_unsigned = dst.is_unsigned
            }
        );
        if (!expr_end_offset) {
            return 0;
        }
        global_expr_end_offset += expr_end_offset;
    }

    int op_handle_err = op_info.handler(codegen, operand_offset, dst);
    if (op_handle_err) {
        micro_push_err((micro_error_t){
            .msg = "Wrong destination type size",
            .line_ref = codegen->toks->toks[pos].line_ref,
            .chpos_ref = codegen->toks->toks[pos].chpos_ref
        });
    }

    if (dst.type == MICRO_ST_DATASEG) {
        micro_addr_le_t dst_addr = micro_imm_le_gen(dst.offset);

        void (*instr_tbl[])(micro_addr_le_t, asm386_reg) = {
            [MICRO_SZ_8]  = asm386_movM8R8,
            [MICRO_SZ_16] = asm386_movM16R16,
            [MICRO_SZ_32] = asm386_movM32R32,
        };
        instr_tbl[dst.size](dst_addr, operand_offset);
    } else
    if (dst.type == MICRO_ST_REG && dst.offset != operand_offset) {
        asm386_reg dst_reg = dst.offset;

        void (*instr_tbl[])(asm386_reg, asm386_reg) = {
            [MICRO_SZ_8]  = asm386_movR8R8,
            [MICRO_SZ_16] = asm386_movR16R16,
            [MICRO_SZ_32] = asm386_movR32R32,
        };
        instr_tbl[dst.size](dst_reg, operand_offset);
    } else
    if (dst.type == MICRO_ST_STACK) {
        micro_addr_le_t dst_addr = micro_imm_le_gen(dst.offset);

        void (*instr_tbl[])(micro_addr_le_t, asm386_reg) = {
            [MICRO_SZ_8]  = asm386_movS32R8,
            [MICRO_SZ_16] = asm386_movS32R16,
            [MICRO_SZ_32] = asm386_movS32R32,
        };
        instr_tbl[dst.size](dst_addr, operand_offset);
    }
    asm_put_instructions(codegen);
    return global_expr_end_offset;
}