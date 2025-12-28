#ifndef MICRO_CODEGEN_386_EXPR_H
#define MICRO_CODEGEN_386_EXPR_H

#include "common.h"

int micro_codegen_386_expr_parse(size_t pos, micro_codegen_386_storage_info_t dst)
{
    if (micro_tokislit(micro_toks[pos])) {
        if (dst.type == MICRO_ST_DATASEC) {
            if (dst.size == 4) {
                char dst_addr[4];
                micro_gen32imm_le(dst_addr, dst.offset);
                char src_val[4];
                micro_gen32imm_le(src_val, strtoull(micro_toks[pos].val, (char**)0, 10));

                char instruction[] = asm_movM32I32(dst_addr, src_val);
                for (size_t i = 0; i < sizeof(instruction)/sizeof(*instruction); i++) {
                    string_push_back(micro_outbuf, instruction[i]);
                }
            } else
            if (dst.size == 2) {
                char dst_addr[4];
                micro_gen32imm_le(dst_addr, dst.offset);
                char src_val[2];
                micro_gen16imm_le(src_val, strtoull(micro_toks[pos].val, (char**)0, 10));

                char instruction[] = asm_movM16I16(dst_addr, src_val);
                for (size_t i = 0; i < sizeof(instruction)/sizeof(*instruction); i++) {
                    string_push_back(micro_outbuf, instruction[i]);
                }
            } else
            if (dst.size == 1) {
                char dst_addr[4];
                micro_gen32imm_le(dst_addr, dst.offset);
                char src_val = strtoull(micro_toks[pos].val, (char**)0, 10);

                char instruction[] = asm_movM8I8(dst_addr, &src_val);
                for (size_t i = 0; i < sizeof(instruction)/sizeof(*instruction); i++) {
                    string_push_back(micro_outbuf, instruction[i]);
                }
            } else {
                micro_error_t err = {.msg = "Wrong destination type size", .line_ref = micro_toks[pos].line_ref, .chpos_ref = micro_toks[pos].chpos_ref};
                __micro_push_err(err);
                return 1;
            }
        }
        return 0;
    }
    if (micro_toks[pos].type == MICRO_TT_IDENT) {
        micro_codegen_386_var_info_t *var_info = hashmap_micro_codegen_386_var_info_t_get(micro_codegen_386_vars, micro_toks[pos].val);
        if (!var_info) {
            micro_error_t err = {.msg = "Unknown identifire", .line_ref = micro_toks[pos].line_ref, .chpos_ref = micro_toks[pos].chpos_ref};
            __micro_push_err(err);
            return 1;
        }
        if (micro_mt_size[var_info->type] != dst.size) {
            micro_error_t err = {.msg = "Variable has wrong type for this expression", .line_ref = micro_toks[pos].line_ref, .chpos_ref = micro_toks[pos].chpos_ref};
            __micro_push_err(err);
            return 1;
        }
        if (dst.type == MICRO_ST_DATASEC) {
            if (dst.size == 4) {
                reg32 acc_reg = REG32_EAX;
                char addr[4];
                // use as ident addr
                micro_gen32imm_le(addr, var_info->storage_info.offset);

                char instruction1[] = asm_movR32M32(acc_reg, addr);
                for (size_t i = 0; i < sizeof(instruction1)/sizeof(*instruction1); i++) {
                    string_push_back(micro_outbuf, instruction1[i]);
                }
                // use as dst addr
                micro_gen32imm_le(addr, dst.offset);

                char instruction2[] = asm_movM32R32(addr, acc_reg);
                for (size_t i = 0; i < sizeof(instruction2)/sizeof(*instruction2); i++) {
                    string_push_back(micro_outbuf, instruction2[i]);
                }
            } else
            if (dst.size == 2) {
                reg16 acc_reg = REG16_AX;
                char addr[4];
                // use as ident addr
                micro_gen32imm_le(addr, var_info->storage_info.offset);

                char instruction1[] = asm_movR16M16(acc_reg, addr);
                for (size_t i = 0; i < sizeof(instruction1)/sizeof(*instruction1); i++) {
                    string_push_back(micro_outbuf, instruction1[i]);
                }
                // use as dst addr
                micro_gen32imm_le(addr, dst.offset);

                char instruction2[] = asm_movM16R16(addr, acc_reg);
                for (size_t i = 0; i < sizeof(instruction2)/sizeof(*instruction2); i++) {
                    string_push_back(micro_outbuf, instruction2[i]);
                }
            } else
            if (dst.size == 1) {
                reg16 acc_reg = REG8_AL;
                char addr[4];
                // use as ident addr
                micro_gen32imm_le(addr, var_info->storage_info.offset);

                char instruction1[] = asm_movR8M8(acc_reg, addr);
                for (size_t i = 0; i < sizeof(instruction1)/sizeof(*instruction1); i++) {
                    string_push_back(micro_outbuf, instruction1[i]);
                }
                // use as dst addr
                micro_gen32imm_le(addr, dst.offset);

                char instruction2[] = asm_movM8R8(addr, acc_reg);
                for (size_t i = 0; i < sizeof(instruction2)/sizeof(*instruction2); i++) {
                    string_push_back(micro_outbuf, instruction2[i]);
                }
            } else {
                micro_error_t err = {.msg = "Wrong destination type size", .line_ref = micro_toks[pos].line_ref, .chpos_ref = micro_toks[pos].chpos_ref};
                __micro_push_err(err);
                return 1;
            }
        }
        return 0;
    }
    if (micro_tokisop(micro_toks[pos])) {
        
    }
}

#endif