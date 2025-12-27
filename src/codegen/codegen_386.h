#ifndef MICRO_CODEGEN_386_H
#define MICRO_CODEGEN_386_H

#include "386/common.h"
#include "386/statements.h"

void micro_codegen_386()
{
    for (micro_pos = 0; micro_pos < micro_toks_size; micro_pos++) {
        // global var
        if (micro_toks[micro_pos].type == MICRO_TT_KW_VAR) {
            micro_codegen_386__static_var();
        } else
        // set value
        if (micro_toks[micro_pos].type == MICRO_TT_KW_SET) {
            micro_codegen_386__set();
        }
    }
}

#endif