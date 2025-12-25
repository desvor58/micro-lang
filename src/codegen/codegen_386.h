#ifndef CODEGEN_386_H
#define CODEGEN_386_H

#include "386/common.h"
#include "386/statements.h"

void codegen_368()
{
    for (pos = 0; pos < toks_size; pos++) {
        // global var
        if (toks[pos].type == TT_KW_VAR) {
            codegen_368__static_var();
        }
    }
}

#endif