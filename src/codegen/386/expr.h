#ifndef MICRO_CODEGEN_386_EXPR_H
#define MICRO_CODEGEN_386_EXPR_H

#include "common.h"
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

int __micro_codegen_386_expr_parse_get_lit(size_t pos, micro_codegen_386_storage_info_t dst)
{
    u8 imm_val[4];

    if (dst.type == MICRO_ST_DATASEG) {
        u8 dst_addr[4];
        micro_gen32imm_le(dst_addr, dst.offset);

        if (dst.size == 4) {
            micro_gen32imm_le(imm_val, strtoll(micro_toks[pos].val, (char**)0, 10));
            u8 instruction[] = asm_movM32I32(dst_addr, imm_val);
            push_instruction(instruction);
        } else
        if (dst.size == 2) {
            micro_gen16imm_le(imm_val, strtoll(micro_toks[pos].val, (char**)0, 10));
            u8 instruction[] = asm_movM16I16(dst_addr, imm_val);
            push_instruction(instruction);
        } else
        if (dst.size == 1) {
            *imm_val = strtoll(micro_toks[pos].val, (char**)0, 10);
            u8 instruction[] = asm_movM8I8(dst_addr, imm_val);
            push_instruction(instruction);
        } else {
            goto err_wrong_dst_type_size;
        }
    } else
    if (dst.type == MICRO_ST_REG) {
        int dst_reg = dst.offset;
        if (dst.size == 4) {
            micro_gen32imm_le(imm_val, strtoll(micro_toks[pos].val, (char**)0, 10));
            u8 instruction[] = asm_movR32I32(dst_reg, imm_val);
            push_instruction(instruction);
        } else
        if (dst.size == 2) {
            micro_gen16imm_le(imm_val, strtoll(micro_toks[pos].val, (char**)0, 10));
            u8 instruction[] = asm_movR16I16(dst_reg, imm_val);
            push_instruction(instruction);
        } else
        if (dst.size == 1) {
            *imm_val = strtoll(micro_toks[pos].val, (char**)0, 10);
            u8 instruction[] = asm_movR8I8(dst_reg, imm_val);
            push_instruction(instruction);
        } else {
            goto err_wrong_dst_type_size;
        }
    } else
    if (dst.type == MICRO_ST_STACK) {
        u8 dst_addr[4];
        micro_gen32imm_le(dst_addr, dst.offset);
        if (dst.size == 4) {
            micro_gen32imm_le(imm_val, strtoll(micro_toks[pos].val, (char**)0, 10));
            u8 instruction[] = asm_movS32I32(dst_addr, imm_val);
            push_instruction(instruction);
        } else
        if (dst.size == 2) {
            micro_gen16imm_le(imm_val, strtoll(micro_toks[pos].val, (char**)0, 10));
            u8 instruction[] = asm_movS32I16(dst_addr, imm_val);
            push_instruction(instruction);
        } else
        if (dst.size == 1) {
            *imm_val = strtoll(micro_toks[pos].val, (char**)0, 10);
            u8 instruction[] = asm_movS32I8(dst_addr, imm_val);
            push_instruction(instruction);
        } else {
            goto err_wrong_dst_type_size;
        }
    }
    return 1;

err_wrong_dst_type_size:
    micro_error_t err = {.msg = "Wrong destination type size", .line_ref = micro_toks[pos].line_ref, .chpos_ref = micro_toks[pos].chpos_ref};
    __micro_push_err(err);
    return 0;
}

int __micro_codegen_386_expr_parse_get_ident_from_dataseg(size_t pos, u8 *ident_addr, micro_codegen_386_var_info_t *var_info, micro_codegen_386_storage_info_t dst)
{
    if (dst.type == MICRO_ST_DATASEG) {
        u8 dst_addr[4];
        micro_gen32imm_le(dst_addr, dst.offset);

        if (dst.size == 4) {
            u8 instruction1[] = asm_movR32M32(REG32_EAX, ident_addr);
            push_instruction(instruction1);
            u8 instruction2[] = asm_movM32R32(dst_addr, REG32_EAX);
            push_instruction(instruction2);
        } else
        if (dst.size == 2) {
            u8 instruction1[] = asm_movR16M16(REG16_AX, ident_addr);
            push_instruction(instruction1);
            u8 instruction2[] = asm_movM16R16(dst_addr, REG16_AX);
            push_instruction(instruction2);
        } else
        if (dst.size == 1) {
            u8 instruction1[] = asm_movR8M8(REG8_AL, ident_addr);
            push_instruction(instruction1);
            u8 instruction2[] = asm_movM8R8(dst_addr, REG8_AL);
            push_instruction(instruction2);
        } else {
            goto err_wrong_dst_type_size;
        }
    } else
    if (dst.type == MICRO_ST_REG) {
        int dst_reg = dst.offset;
        if (dst.size == 4) {
            u8 instruction[] = asm_movR32M32(dst_reg, ident_addr);
            push_instruction(instruction);
        } else
        if (dst.size == 2) {
            u8 instruction[] = asm_movR16M16(dst_reg, ident_addr);
            push_instruction(instruction);
        } else
        if (dst.size == 1) {
            u8 instruction[] = asm_movR8M8(dst_reg, ident_addr);
            push_instruction(instruction);
        } else {
            goto err_wrong_dst_type_size;
        }
    } else
    if (dst.type == MICRO_ST_STACK) {
        u8 dst_offset[4];
        micro_gen32imm_le(dst_offset, dst.offset);
        if (dst.size == 4) {
            u8 instruction1[] = asm_movR32M32(REG32_EAX, ident_addr);
            push_instruction(instruction1);
            u8 instruction2[] = asm_movS32R32(dst_offset, REG32_EAX);
            push_instruction(instruction2);
        } else
        if (dst.size == 2) {
            u8 instruction1[] = asm_movR16M16(REG16_AX, ident_addr);
            push_instruction(instruction1);
            u8 instruction2[] = asm_movS32R16(dst_offset, REG16_AX);
            push_instruction(instruction2);
        } else
        if (dst.size == 1) {
            u8 instruction1[] = asm_movR8M8(REG8_AL, ident_addr);
            push_instruction(instruction1);
            u8 instruction2[] = asm_movS32R8(dst_offset, REG8_AL);
            push_instruction(instruction2);
        } else {
            goto err_wrong_dst_type_size;
        }
    }
    return 1;

err_wrong_dst_type_size:
    micro_error_t err = {.msg = "Wrong destination type size", .line_ref = micro_toks[pos].line_ref, .chpos_ref = micro_toks[pos].chpos_ref};
    __micro_push_err(err);
    return 0;
}

int __micro_codegen_386_expr_parse_get_ident_from_stack(size_t pos, u8 *ident_addr, micro_codegen_386_var_info_t *var_info, micro_codegen_386_storage_info_t dst)
{
    if (dst.type == MICRO_ST_DATASEG) {
        u8 dst_addr[4];
        micro_gen32imm_le(dst_addr, dst.offset);

        if (dst.size == 4) {
            u8 instruction1[] = asm_movR32S32(REG32_EAX, ident_addr);
            push_instruction(instruction1);
            u8 instruction2[] = asm_movM32R32(dst_addr, REG32_EAX);
            push_instruction(instruction2);
        } else
        if (dst.size == 2) {
            u8 instruction1[] = asm_movR16S32(REG16_AX, ident_addr);
            push_instruction(instruction1);
            u8 instruction2[] = asm_movM16R16(dst_addr, REG16_AX);
            push_instruction(instruction2);
        } else
        if (dst.size == 1) {
            u8 instruction1[] = asm_movR8S32(REG8_AL, ident_addr);
            push_instruction(instruction1);
            u8 instruction2[] = asm_movM8R8(dst_addr, REG8_AL);
            push_instruction(instruction2);
        } else {
            goto err_wrong_dst_type_size;
        }
    } else
    if (dst.type == MICRO_ST_REG) {
        int dst_reg = dst.offset;
        if (dst.size == 4) {
            u8 instruction[] = asm_movR32S32(dst_reg, ident_addr);
            push_instruction(instruction);
        } else
        if (dst.size == 2) {
            u8 instruction[] = asm_movR16S32(dst_reg, ident_addr);
            push_instruction(instruction);
        } else
        if (dst.size == 1) {
            u8 instruction[] = asm_movR8S32(dst_reg, ident_addr);
            push_instruction(instruction);
        } else {
            goto err_wrong_dst_type_size;
        }
    } else
    if (dst.type == MICRO_ST_STACK) {
        u8 dst_offset[4];
        micro_gen32imm_le(dst_offset, dst.offset);
        if (dst.size == 4) {
            u8 instruction1[] = asm_movR32S32(REG32_EAX, ident_addr);
            push_instruction(instruction1);
            u8 instruction2[] = asm_movS32R32(dst_offset, REG32_EAX);
            push_instruction(instruction2);
        } else
        if (dst.size == 2) {
            u8 instruction1[] = asm_movR16S32(REG16_AX, ident_addr);
            push_instruction(instruction1);
            u8 instruction2[] = asm_movS32R16(dst_offset, REG16_AX);
            push_instruction(instruction2);
        } else
        if (dst.size == 1) {
            u8 instruction1[] = asm_movR8S32(REG8_AL, ident_addr);
            push_instruction(instruction1);
            u8 instruction2[] = asm_movS32R8(dst_offset, REG8_AL);
            push_instruction(instruction2);
        } else {
            goto err_wrong_dst_type_size;
        }
    }
    return 1;

err_wrong_dst_type_size:
    micro_error_t err = {.msg = "Wrong destination type size", .line_ref = micro_toks[pos].line_ref, .chpos_ref = micro_toks[pos].chpos_ref};
    __micro_push_err(err);
    return 0;
}

int __micro_codegen_386_expr_parse_get_ident(size_t pos, micro_codegen_386_storage_info_t dst)
{
    micro_codegen_386_var_info_t *var_info = hashmap_micro_codegen_386_var_info_t_get(micro_codegen_386_vars, micro_toks[pos].val);
    if (!var_info) {
        micro_error_t err = {.msg = "Unknown identifire", .line_ref = micro_toks[pos].line_ref, .chpos_ref = micro_toks[pos].chpos_ref};
        __micro_push_err(err);
        return 0;
    }
    if (micro_mt_size[var_info->type] != dst.size) {
        micro_error_t err = {.msg = "Variable has wrong type for this expression", .line_ref = micro_toks[pos].line_ref, .chpos_ref = micro_toks[pos].chpos_ref};
        __micro_push_err(err);
        return 0;
    }

    u8 ident_addr[4];
    micro_gen32imm_le(ident_addr, var_info->storage_info.offset);
    if (var_info->storage_info.type == MICRO_ST_DATASEG) {
        int ok = __micro_codegen_386_expr_parse_get_ident_from_dataseg(pos, ident_addr, var_info, dst);
        if (!ok) {
            return 0;
        }
    } else
    if (var_info->storage_info.type == MICRO_ST_STACK) {
        int ok = __micro_codegen_386_expr_parse_get_ident_from_stack(pos, ident_addr, var_info, dst);
        if (!ok) {
            return 0;
        }
    }
    
    return 1;
}

typedef struct {
    u8 operands_num;
    int (*handler)(size_t, micro_codegen_386_storage_info_t);
} micro_codegen_386_operator_info_t;

micro_codegen_386_operator_info_t micro_codegen_386_ops_table[] = {
    [MICRO_TT_PLUS]  = (micro_codegen_386_operator_info_t){ .operands_num = 2, .handler = micro_codegen_386__op_plus     },
    [MICRO_TT_MINUS] = (micro_codegen_386_operator_info_t){ .operands_num = 2, .handler = micro_codegen_386__op_minus    },
    [MICRO_TT_STAR]  = (micro_codegen_386_operator_info_t){ .operands_num = 2, .handler = micro_codegen_386__op_multiply },
    [MICRO_TT_SLASH] = (micro_codegen_386_operator_info_t){ .operands_num = 2, .handler = micro_codegen_386__op_division },
    [MICRO_TT_HASH]  = (micro_codegen_386_operator_info_t){ .operands_num = 1, .handler = micro_codegen_386__op_getaddr  },
};

int micro_codegen_386_expr_parse(size_t pos, micro_codegen_386_storage_info_t dst)
{
    if (micro_tokislit(micro_toks[pos])) {
        return __micro_codegen_386_expr_parse_get_lit(pos, dst);
    }
    if (micro_toks[pos].type == MICRO_TT_IDENT) {
        return __micro_codegen_386_expr_parse_get_ident(pos, dst);
    }

    if (!micro_tokisop(micro_toks[pos])) {
        micro_error_t err = {.msg = "Expected operator", .line_ref = micro_toks[pos].line_ref, .chpos_ref = micro_toks[pos].chpos_ref};
        __micro_push_err(err);
        return 0;
    }

    size_t start_offset = dst.type == MICRO_ST_REG ? dst.offset : 0;    
    micro_codegen_386_operator_info_t op_info = micro_codegen_386_ops_table[micro_toks[pos].type];

    int global_expr_end_offset = 1;
    for (size_t i = 0; i < op_info.operands_num; i++) {
        int expr_end_offset = micro_codegen_386_expr_parse(
            pos + global_expr_end_offset,
            (micro_codegen_386_storage_info_t) {
                .type = MICRO_ST_REG,
                .size = dst.size,
                .offset = start_offset + i,
                .is_unsigned = dst.is_unsigned
            }
        );
        if (expr_end_offset == 0) {
            return 0;
        }
        global_expr_end_offset += expr_end_offset;
    }

    int op_handle_err = op_info.handler(start_offset, dst);
    if (op_handle_err) {
        goto err_wrong_dst_type_size;
    }

    if (dst.type == MICRO_ST_DATASEG) {
        u8 dst_addr[4];
        micro_gen32imm_le(dst_addr, dst.offset);

        if (dst.size == 4) {
            u8 instruction[] = asm_movM32R32(dst_addr, start_offset);
            push_instruction(instruction);
        } else
        if (dst.size == 2) {
            u8 instruction[] = asm_movM16R16(dst_addr, start_offset);
            push_instruction(instruction);
        } else
        if (dst.size == 1) {
            u8 instruction[] = asm_movM8R8(dst_addr, start_offset);
            push_instruction(instruction);
        } else {
            goto err_wrong_dst_type_size;
        }
    } else
    if (dst.type == MICRO_ST_REG && dst.offset != start_offset) {
        reg32 dst_reg = dst.offset;

        if (dst.size == 4) {
            u8 instruction[] = asm_movR32R32(dst_reg, start_offset);
            push_instruction(instruction);
        } else
        if (dst.size == 2) {
            u8 instruction[] = asm_movR16R16(dst_reg, start_offset);
            push_instruction(instruction);
        } else
        if (dst.size == 1) {
            u8 instruction[] = asm_movR8R8(dst_reg, start_offset);
            push_instruction(instruction);
        } else {
            goto err_wrong_dst_type_size;
        }
    } else
    if (dst.type == MICRO_ST_STACK) {
        u8 dst_addr[4];
        micro_gen32imm_le(dst_addr, dst.offset);
        if (dst.size == 4) {
            u8 instruction[] = asm_movS32R32(dst_addr, start_offset);
            push_instruction(instruction);
        } else
        if (dst.size == 2) {
            u8 instruction[] = asm_movS32R16(dst_addr, start_offset);
            push_instruction(instruction);
        } else
        if (dst.size == 1) {
            u8 instruction[] = asm_movS32R8(dst_addr, start_offset);
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