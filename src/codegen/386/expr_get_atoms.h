#ifndef MICRO_CODEGEN_386_EXPR_GET_ATOMS_H
#define MICRO_CODEGEN_386_EXPR_GET_ATOMS_H

#include "common.h"

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

int __micro_codegen_386_expr_parse_get_ident_from_dataseg(u8 *ident_addr, micro_codegen_386_var_info_t *var_info, micro_codegen_386_storage_info_t dst)
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
            return 1;
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
            return 1;
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
            return 1;
        }
    }
    return 0;
}

int __micro_codegen_386_expr_parse_get_ident_from_stack(u8 *ident_addr, micro_codegen_386_var_info_t *var_info, micro_codegen_386_storage_info_t dst)
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
            return 1;
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
            return 1;
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
            return 1;
        }
    }
    return 0;
}

int __micro_codegen_386_expr_parse_get_ident_addr_from_dataseg(u8 *ident_addr, micro_codegen_386_var_info_t *var_info, micro_codegen_386_storage_info_t dst)
{
    if (dst.type == MICRO_ST_DATASEG) {
        u8 dst_addr[4];
        micro_gen32imm_le(dst_addr, dst.offset);

        if (dst.size == 4) {
            u8 instruction[] = asm_movM32I32(dst_addr, ident_addr);
            push_instruction(instruction);
        } else
        if (dst.size == 2) {
            u8 instruction[] = asm_movM16I16(dst_addr, ident_addr);
            push_instruction(instruction);
        } else
        if (dst.size == 1) {
            u8 instruction[] = asm_movM8I8(dst_addr, ident_addr);
            push_instruction(instruction);
        } else {
            return 1;
        }
    } else
    if (dst.type == MICRO_ST_REG) {
        int dst_reg = dst.offset;
        if (dst.size == 4) {
            u8 instruction[] = asm_movR32I32(dst_reg, ident_addr);
            push_instruction(instruction);
        } else
        if (dst.size == 2) {
            u8 instruction[] = asm_movR16I16(dst_reg, ident_addr);
            push_instruction(instruction);
        } else
        if (dst.size == 1) {
            u8 instruction[] = asm_movR8I8(dst_reg, ident_addr);
            push_instruction(instruction);
        } else {
            return 1;
        }
    } else
    if (dst.type == MICRO_ST_STACK) {
        u8 dst_offset[4];
        micro_gen32imm_le(dst_offset, dst.offset);
        if (dst.size == 4) {
            u8 instruction[] = asm_movS32I32(dst_offset, ident_addr);
            push_instruction(instruction);
        } else
        if (dst.size == 2) {
            u8 instruction[] = asm_movS32I16(dst_offset, ident_addr);
            push_instruction(instruction);
        } else
        if (dst.size == 1) {
            u8 instruction[] = asm_movS32I8(dst_offset, ident_addr);
            push_instruction(instruction);
        } else {
            return 1;
        }
    }
    return 0;
}

int __micro_codegen_386_expr_parse_get_ident_addr_from_stack(u8 *ident_addr, micro_codegen_386_var_info_t *var_info, micro_codegen_386_storage_info_t dst)
{
    if (dst.type == MICRO_ST_DATASEG) {
        u8 dst_addr[4];
        micro_gen32imm_le(dst_addr, dst.offset);

        if (dst.size == 4) {
            u8 instruction1[] = asm_leaR32S32(REG32_EAX, ident_addr);
            push_instruction(instruction1);
            u8 instruction2[] = asm_movM32R32(dst_addr, REG32_EAX);
            push_instruction(instruction2);
        } else
        if (dst.size == 2) {
            u8 instruction1[] = asm_leaR16S32(REG16_AX, ident_addr);
            push_instruction(instruction1);
            u8 instruction2[] = asm_movM16R16(dst_addr, REG16_AX);
            push_instruction(instruction2);
        } else {
            return 1;
        }
    } else
    if (dst.type == MICRO_ST_REG) {
        int dst_reg = dst.offset;
        if (dst.size == 4) {
            u8 instruction[] = asm_leaR32S32(dst_reg, ident_addr);
            push_instruction(instruction);
        } else
        if (dst.size == 2) {
            u8 instruction[] = asm_leaR16S32(dst_reg, ident_addr);
            push_instruction(instruction);
        } else {
            return 1;
        }
    } else
    if (dst.type == MICRO_ST_STACK) {
        u8 dst_offset[4];
        micro_gen32imm_le(dst_offset, dst.offset);
        if (dst.size == 4) {
            u8 instruction1[] = asm_leaR32S32(REG32_EAX, ident_addr);
            push_instruction(instruction1);
            u8 instruction2[] = asm_movS32R32(dst_offset, REG32_EAX);
            push_instruction(instruction2);
        } else
        if (dst.size == 2) {
            u8 instruction1[] = asm_leaR16S32(REG16_AX, ident_addr);
            push_instruction(instruction1);
            u8 instruction2[] = asm_movS32R16(dst_offset, REG16_AX);
            push_instruction(instruction2);
        } else {
            return 1;
        }
    }
    return 0;
}

int __micro_codegen_386_expr_parse_get_ident(size_t pos, micro_codegen_386_storage_info_t dst, int expected_addr)
{
    micro_codegen_386_var_info_t *var_info = sct_hashmap_get(micro_codegen_386_vars, micro_toks[pos].val);
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

    if (expected_addr) {
        if (var_info->storage_info.type == MICRO_ST_DATASEG) {
            int get_err = __micro_codegen_386_expr_parse_get_ident_addr_from_dataseg(ident_addr, var_info, dst);
            if (get_err) {
                goto err_wrong_dst_type_size;
            }
        } else
        if (var_info->storage_info.type == MICRO_ST_STACK) {
            int get_err = __micro_codegen_386_expr_parse_get_ident_addr_from_stack(ident_addr, var_info, dst);
            if (get_err) {
                goto err_wrong_dst_type_size;
            }
        }
    } else {
        if (var_info->storage_info.type == MICRO_ST_DATASEG) {
            int get_err = __micro_codegen_386_expr_parse_get_ident_from_dataseg(ident_addr, var_info, dst);
            if (get_err) {
                goto err_wrong_dst_type_size;
            }
        } else
        if (var_info->storage_info.type == MICRO_ST_STACK) {
            int get_err = __micro_codegen_386_expr_parse_get_ident_from_stack(ident_addr, var_info, dst);
            if (get_err) {
                goto err_wrong_dst_type_size;
            }
        }
    }
    return 1;

err_wrong_dst_type_size:
    micro_error_t err = {.msg = "Wrong destination type size", .line_ref = micro_toks[pos].line_ref, .chpos_ref = micro_toks[pos].chpos_ref};
    __micro_push_err(err);
    return 0;
}

#endif