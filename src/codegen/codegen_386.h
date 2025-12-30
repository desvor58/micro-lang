#ifndef MICRO_CODEGEN_386_H
#define MICRO_CODEGEN_386_H

#include "386/common.h"
#include "386/statements.h"

void micro_codegen_386_micro_instruction_parse()
{
    // global var
    if (micro_toks[micro_pos].type == MICRO_TT_KW_VAR) {
        micro_codegen_386__static_var();
    } else
    // set value
    if (micro_toks[micro_pos].type == MICRO_TT_KW_SET) {
        micro_codegen_386__set();
    } else
    // function decl
    if (micro_toks[micro_pos].type == MICRO_TT_KW_FUN) {
        micro_codegen_386__fun();
    }
    // function decl
    if (micro_toks[micro_pos].type == MICRO_TT_KW_CALL) {
        micro_codegen_386__call();
    }
}

void micro_codegen_386()
{
    for (micro_pos = 0; micro_pos < micro_toks_size; micro_pos++) {
        micro_codegen_386_micro_instruction_parse();
    }
}

#endif