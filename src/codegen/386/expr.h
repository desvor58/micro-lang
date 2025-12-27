#ifndef MICRO_CODEGEN_386_EXPR_H
#define MICRO_CODEGEN_386_EXPR_H

#include "common.h"

void micro_codegen_386_expr_parse(size_t pos, micro_codegen_386_storage_info_t dst)
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
            }
            if (dst.size == 2) {
                char addr[4];
                micro_gen32imm_le(addr, dst.offset);
                char val[2];
                micro_gen16imm_le(val, strtoull(micro_toks[pos].val, (char**)0, 10));

                char instruction[] = asm_movM16I16(addr, val);
                for (size_t i = 0; i < sizeof(instruction)/sizeof(*instruction); i++) {
                    string_push_back(micro_outbuf, instruction[i]);
                }
            }
            if (dst.size == 1) {
                char addr[4];
                micro_gen32imm_le(addr, dst.offset);
                char val = strtoull(micro_toks[pos].val, (char**)0, 10);

                char instruction[] = asm_movM8I8(addr, &val);
                for (size_t i = 0; i < sizeof(instruction)/sizeof(*instruction); i++) {
                    string_push_back(micro_outbuf, instruction[i]);
                }
            }
        }
    }

}

#endif