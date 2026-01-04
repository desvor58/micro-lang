#ifndef MICRO_CODEGEN_386_EXPR_H
#define MICRO_CODEGEN_386_EXPR_H

#include "common.h"

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
    if (dst.type == MICRO_ST_DATASEG) {
        if (dst.size == 4) {
            u8 dst_addr[4];
            micro_gen32imm_le(dst_addr, dst.offset);
            u8 src_val[4];
            micro_gen32imm_le(src_val, strtoll(micro_toks[pos].val, (char**)0, 10));

            u8 instruction[] = asm_movM32I32(dst_addr, src_val);
            push_instruction(instruction);
        } else
        if (dst.size == 2) {
            u8 dst_addr[4];
            micro_gen32imm_le(dst_addr, dst.offset);
            u8 src_val[2];
            micro_gen16imm_le(src_val, strtoll(micro_toks[pos].val, (char**)0, 10));

            u8 instruction[] = asm_movM16I16(dst_addr, src_val);
            push_instruction(instruction);
        } else
        if (dst.size == 1) {
            u8 dst_addr[4];
            micro_gen32imm_le(dst_addr, dst.offset);
            u8 src_val = strtoll(micro_toks[pos].val, (char**)0, 10);

            u8 instruction[] = asm_movM8I8(dst_addr, &src_val);
            push_instruction(instruction);
        } else {
            goto err_wrong_dst_type_size;
        }
    } else
    if (dst.type == MICRO_ST_REG) {
        if (dst.size == 4) {
            reg32 dst_reg = dst.offset;
            u8 src_val[4];
            micro_gen32imm_le(src_val, strtoll(micro_toks[pos].val, (char**)0, 10));
            u8 instruction[] = asm_movR32I32(dst_reg, src_val);
            push_instruction(instruction);
        } else
        if (dst.size == 2) {
            reg32 dst_reg = dst.offset;
            u8 src_val[2];
            micro_gen16imm_le(src_val, strtoll(micro_toks[pos].val, (char**)0, 10));
            u8 instruction[] = asm_movR16I16(dst_reg, src_val);
            push_instruction(instruction);
        } else
        if (dst.size == 1) {
            reg32 dst_reg = dst.offset;
            u8 src_val = strtoll(micro_toks[pos].val, (char**)0, 10);
            u8 instruction[] = asm_movR8I8(dst_reg, &src_val);
            push_instruction(instruction);
        } else {
            goto err_wrong_dst_type_size;
        }
    } else
    if (dst.type == MICRO_ST_STACK) {
        if (dst.size == 4) {
            u8 dst_addr[4];
            micro_gen32imm_le(dst_addr, dst.offset);
            u8 src_val[4];
            micro_gen32imm_le(src_val, strtoll(micro_toks[pos].val, (char**)0, 10));

            u8 instruction[] = asm_movS32I32(dst_addr, src_val);
            push_instruction(instruction);
        } else
        if (dst.size == 2) {
            u8 dst_addr[4];
            micro_gen32imm_le(dst_addr, dst.offset);
            u8 src_val[2];
            micro_gen16imm_le(src_val, strtoll(micro_toks[pos].val, (char**)0, 10));

            u8 instruction[] = asm_movS32I16(dst_addr, src_val);
            push_instruction(instruction);
        } else
        if (dst.size == 1) {
            u8 dst_addr[4];
            micro_gen32imm_le(dst_addr, dst.offset);
            u8 src_val = strtoll(micro_toks[pos].val, (char**)0, 10);

            u8 instruction[] = asm_movS32I8(dst_addr, &src_val);
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
    if (dst.type == MICRO_ST_DATASEG) {
        if (dst.size == 4) {
            reg32 acc_reg = REG32_EAX;
            u8 addr[4];
            // use as ident addr
            micro_gen32imm_le(addr, var_info->storage_info.offset);

            u8 instruction1[] = asm_movR32M32(acc_reg, addr);
            push_instruction(instruction1);
            // use as dst addr
            micro_gen32imm_le(addr, dst.offset);

            u8 instruction2[] = asm_movM32R32(addr, acc_reg);
            push_instruction(instruction2);
        } else
        if (dst.size == 2) {
            reg16 acc_reg = REG16_AX;
            u8 addr[4];
            // use as ident addr
            micro_gen32imm_le(addr, var_info->storage_info.offset);

            u8 instruction1[] = asm_movR16M16(acc_reg, addr);
            push_instruction(instruction1);
            // use as dst addr
            micro_gen32imm_le(addr, dst.offset);

            u8 instruction2[] = asm_movM16R16(addr, acc_reg);
            push_instruction(instruction2);
        } else
        if (dst.size == 1) {
            reg8 acc_reg = REG8_AL;
            u8 addr[4];
            // use as ident addr
            micro_gen32imm_le(addr, var_info->storage_info.offset);

            u8 instruction1[] = asm_movR8M8(acc_reg, addr);
            push_instruction(instruction1);
            // use as dst addr
            micro_gen32imm_le(addr, dst.offset);

            u8 instruction2[] = asm_movM8R8(addr, acc_reg);
            push_instruction(instruction2);
        } else {
            goto err_wrong_dst_type_size;
        }
    } else
    if (dst.type == MICRO_ST_REG) {
        if (dst.size == 4) {
            reg32 dst_reg = dst.offset;
            u8 ident_addr[4];
            micro_gen32imm_le(ident_addr, var_info->storage_info.offset);

            u8 instruction[] = asm_movR32M32(dst_reg, ident_addr);
            push_instruction(instruction);
        } else
        if (dst.size == 2) {
            reg16 dst_reg = dst.offset;
            u8 ident_addr[4];
            micro_gen32imm_le(ident_addr, var_info->storage_info.offset);

            u8 instruction[] = asm_movR16M16(dst_reg, ident_addr);
            push_instruction(instruction);
        } else
        if (dst.size == 1) {
            reg8 dst_reg = dst.offset;
            u8 ident_addr[4];
            micro_gen32imm_le(ident_addr, var_info->storage_info.offset);

            u8 instruction[] = asm_movR8M8(dst_reg, ident_addr);
            push_instruction(instruction);
        } else {
            goto err_wrong_dst_type_size;
        }
    } else
    if (dst.type == MICRO_ST_STACK) {
        if (dst.size == 4) {
            reg32 acc_reg = REG32_EAX;
            u8 addr[4];
            // use as ident addr
            micro_gen32imm_le(addr, var_info->storage_info.offset);

            u8 instruction1[] = asm_movR32M32(acc_reg, addr);
            push_instruction(instruction1);
            // use as dst addr
            micro_gen32imm_le(addr, dst.offset);

            u8 instruction2[] = asm_movS32R32(addr, acc_reg);
            push_instruction(instruction2);
        } else
        if (dst.size == 2) {
            reg16 acc_reg = REG16_AX;
            u8 addr[4];
            // use as ident addr
            micro_gen32imm_le(addr, var_info->storage_info.offset);

            u8 instruction1[] = asm_movR16M16(acc_reg, addr);
            push_instruction(instruction1);
            // use as dst addr
            micro_gen32imm_le(addr, dst.offset);

            u8 instruction2[] = asm_movS32R16(addr, acc_reg);
            push_instruction(instruction2);
        } else
        if (dst.size == 1) {
            reg8 acc_reg = REG8_AL;
            u8 addr[4];
            // use as ident addr
            micro_gen32imm_le(addr, var_info->storage_info.offset);

            u8 instruction1[] = asm_movR8M8(acc_reg, addr);
            push_instruction(instruction1);
            // use as dst addr
            micro_gen32imm_le(addr, dst.offset);

            u8 instruction2[] = asm_movS32R8(addr, acc_reg);
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

// return the offset to pos where expression has ending
// + 5 4
// p     r
// 5     3
//
// 5
// p r
// 8 1
int micro_codegen_386_expr_parse(size_t pos, micro_codegen_386_storage_info_t dst)
{
    if (micro_tokislit(micro_toks[pos])) {
        return __micro_codegen_386_expr_parse_get_lit(pos, dst);
    }
    if (micro_toks[pos].type == MICRO_TT_IDENT) {
        return __micro_codegen_386_expr_parse_get_ident(pos, dst);
    }
    size_t start_offset = dst.type == MICRO_ST_REG ? dst.offset : 0;    
    if (micro_tokisop(micro_toks[pos])) {
        int expr1_end_offset = micro_codegen_386_expr_parse(
            pos + 1,
            (micro_codegen_386_storage_info_t) {
                .type = MICRO_ST_REG,
                .size = dst.size,
                .offset = start_offset,
                .is_unsigned = dst.is_unsigned
            }
        );
        if (expr1_end_offset == 0) {
            return 0;
        }
        int expr2_end_offset = micro_codegen_386_expr_parse(
            pos + expr1_end_offset + 1,
            (micro_codegen_386_storage_info_t) {
                .type = MICRO_ST_REG,
                .size = dst.size,
                .offset = start_offset + 1,
                .is_unsigned = dst.is_unsigned
            }
        );
        if (expr2_end_offset == 0) {
            return 0;
        }
        if (micro_toks[pos].type == MICRO_TT_PLUS) {
            if (dst.size == 4) {
                u8 instruction[] = asm_addR32R32(start_offset, start_offset + 1);
                push_instruction(instruction);
            } else
            if (dst.size == 2) {
                u8 instruction[] = asm_addR16R16(start_offset, start_offset + 1);
                push_instruction(instruction);
            } else
            if (dst.size == 1) {
                u8 instruction[] = asm_addR8R8(start_offset, start_offset + 1);
                push_instruction(instruction);
            } else {
                goto err_wrong_dst_type_size;
            }
        } else
        if (micro_toks[pos].type == MICRO_TT_MINUS) {
            if (dst.size == 4) {
                u8 instruction[] = asm_subR32R32(start_offset, start_offset + 1);
                push_instruction(instruction);
            } else
            if (dst.size == 2) {
                u8 instruction[] = asm_subR16R16(start_offset, start_offset + 1);
                push_instruction(instruction);
            } else
            if (dst.size == 1) {
                u8 instruction[] = asm_subR8R8(start_offset, start_offset + 1);
                push_instruction(instruction);
            } else {
                goto err_wrong_dst_type_size;
            }
        } else
        if (micro_toks[pos].type == MICRO_TT_STAR) {
            if (dst.is_unsigned) {
                if (dst.size == 4) {
                    if (start_offset) {
                        u8 instruction[] = asm_xchgR32R32(0, start_offset);
                        push_instruction(instruction);
                    }
                    u8 instruction[] = asm_mulR32(start_offset + 1);
                    push_instruction(instruction);
                    if (start_offset) {
                        u8 instruction[] = asm_xchgR32R32(start_offset, 0);
                        push_instruction(instruction);
                    }
                } else
                if (dst.size == 2) {
                    if (start_offset) {
                        u8 instruction[] = asm_xchgR16R16(0, start_offset);
                        push_instruction(instruction);
                    }
                    u8 instruction[] = asm_mulR16(start_offset + 1);
                    push_instruction(instruction);
                    if (start_offset) {
                        u8 instruction[] = asm_xchgR16R16(start_offset, 0);
                        push_instruction(instruction);
                    }
                } else
                if (dst.size == 1) {
                    if (start_offset) {
                        u8 instruction[] = asm_xchgR8R8(0, start_offset);
                        push_instruction(instruction);
                    }
                    u8 instruction[] = asm_mulR8(start_offset + 1);
                    push_instruction(instruction);
                    if (start_offset) {
                        u8 instruction[] = asm_xchgR8R8(start_offset, 0);
                        push_instruction(instruction);
                    }
                } else {
                    goto err_wrong_dst_type_size;
                }
            } else {
                if (dst.size == 4) {
                    u8 instruction[] = asm_imulR32R32(start_offset, start_offset + 1);
                    push_instruction(instruction);
                } else
                if (dst.size == 2) {
                    u8 instruction[] = asm_imulR16R16(start_offset, start_offset + 1);
                    push_instruction(instruction);
                } else
                if (dst.size == 1) {
                    u8 instruction[] = asm_imulR8R8(start_offset, start_offset + 1);
                    push_instruction(instruction);
                } else {
                    goto err_wrong_dst_type_size;
                }
            }
        } else
        if (micro_toks[pos].type == MICRO_TT_SLASH) {
            if (start_offset + 1 >= REG32_EDX) {
                u8 instruction[] = asm_pushR32(REG32_EDX);
                push_instruction(instruction);
            }
            if (dst.is_unsigned) {
                if (dst.size == 4) {
                    if (start_offset) {
                        u8 instruction[] = asm_xchgR32R32(0, start_offset);
                        push_instruction(instruction);
                    }
                    u8 instruction[] = asm_divR32(start_offset + 1);
                    push_instruction(instruction);
                    if (start_offset) {
                        u8 instruction[] = asm_xchgR32R32(start_offset, 0);
                        push_instruction(instruction);
                    }
                } else
                if (dst.size == 2) {
                    if (start_offset) {
                        u8 instruction[] = asm_xchgR16R16(0, start_offset);
                        push_instruction(instruction);
                    }
                    u8 instruction[] = asm_divR16(start_offset + 1);
                    push_instruction(instruction);
                    if (start_offset) {
                        u8 instruction[] = asm_xchgR16R16(start_offset, 0);
                        push_instruction(instruction);
                    }
                } else
                if (dst.size == 1) {
                    if (start_offset) {
                        u8 instruction[] = asm_xchgR8R8(0, start_offset);
                        push_instruction(instruction);
                    }
                    u8 instruction[] = asm_divR8(start_offset + 1);
                    push_instruction(instruction);
                    if (start_offset) {
                        u8 instruction[] = asm_xchgR8R8(start_offset, 0);
                        push_instruction(instruction);
                    }
                } else {
                    goto err_wrong_dst_type_size;
                }
            } else {
                if (dst.size == 4) {
                    if (start_offset) {
                        u8 instruction[] = asm_xchgR32R32(0, start_offset);
                        push_instruction(instruction);
                    }
                    u8 instruction[] = asm_idivR32(start_offset + 1);
                    push_instruction(instruction);
                    if (start_offset) {
                        u8 instruction[] = asm_xchgR32R32(0, start_offset);
                        push_instruction(instruction);
                    }
                } else
                if (dst.size == 2) {
                    if (start_offset) {
                        u8 instruction[] = asm_xchgR16R16(0, start_offset);
                        push_instruction(instruction);
                    }
                    u8 instruction[] = asm_idivR16(start_offset + 1);
                    push_instruction(instruction);
                    if (start_offset) {
                        u8 instruction[] = asm_xchgR16R16(start_offset, 0);
                        push_instruction(instruction);
                    }
                } else
                if (dst.size == 1) {
                    if (start_offset) {
                        u8 instruction[] = asm_xchgR8R8(0, start_offset);
                        push_instruction(instruction);
                    }
                    u8 instruction[] = asm_idivR8(start_offset + 1);
                    push_instruction(instruction);
                    if (start_offset) {
                        u8 instruction[] = asm_xchgR8R8(start_offset, 0);
                        push_instruction(instruction);
                    }
                } else {
                    goto err_wrong_dst_type_size;
                }
            }
            if (start_offset + 1 >= REG32_EDX) {
                u8 instruction[] = asm_popR32(REG32_EDX);
                push_instruction(instruction);
            }
        } else {
            micro_error_t err = {.msg = "Unknow operator", .line_ref = micro_toks[pos].line_ref, .chpos_ref = micro_toks[pos].chpos_ref};
            __micro_push_err(err);
            return 0;
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
        return expr1_end_offset + expr2_end_offset + 1;
    }

err_wrong_dst_type_size:
    micro_error_t err = {.msg = "Wrong destination type size", .line_ref = micro_toks[pos].line_ref, .chpos_ref = micro_toks[pos].chpos_ref};
    __micro_push_err(err);
    return 0;
}

#endif