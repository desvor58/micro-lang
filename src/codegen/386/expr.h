#ifndef MICRO_CODEGEN_386_EXPR_H
#define MICRO_CODEGEN_386_EXPR_H

#include "common.h"

int micro_codegen_386_expr_parse(size_t pos, micro_codegen_386_storage_info_t dst)
{
    if (micro_tokislit(micro_toks[pos])) {
        if (dst.type == MICRO_ST_DATASEC) {
            if (dst.size == 4) {
                char addr[4];
                micro_gen32imm_le(addr, dst.offset);
                char val[4];
                micro_gen32imm_le(val, strtoull(micro_toks[pos].val, (char**)0, 10));

                char instruction[] = asm_movM32I32(addr, val);
                for (size_t i = 0; i < sizeof(instruction)/sizeof(*instruction); i++) {
                    string_push_back(micro_outbuf, instruction[i]);
                }
            } else
            if (dst.size == 2) {
                char addr[4];
                micro_gen32imm_le(addr, dst.offset);
                char val[2];
                micro_gen16imm_le(val, strtoull(micro_toks[pos].val, (char**)0, 10));

                char instruction[] = asm_movM16I16(addr, val);
                for (size_t i = 0; i < sizeof(instruction)/sizeof(*instruction); i++) {
                    string_push_back(micro_outbuf, instruction[i]);
                }
            } else
            if (dst.size == 1) {
                char addr[4];
                micro_gen32imm_le(addr, dst.offset);
                char val = strtoull(micro_toks[pos].val, (char**)0, 10);

                char instruction[] = asm_movM8I8(addr, &val);
                for (size_t i = 0; i < sizeof(instruction)/sizeof(*instruction); i++) {
                    string_push_back(micro_outbuf, instruction[i]);
                }
            } else {
                micro_error_t err = {.msg = "Wrong destination type size", .line_ref = micro_toks[pos].line_ref, .chpos_ref = micro_toks[pos].chpos_ref};
                __micro_push_err(err);
                return 1;
            }
        }
    }
    if (micro_toks[micro_pos].type == MICRO_TT_IDENT) {
        micro_codegen_386_var_info_t *var_info = hashmap_micro_codegen_386_var_info_t_get(micro_codegen_386_vars, micro_toks[micro_pos].val);
        
    }
}

#endif