#ifndef MICRO_CODEGEN_386_STATEMENTS_H
#define MICRO_CODEGEN_386_STATEMENTS_H

#include "common.h"
#include "expr.h"

void micro_codegen_386__static_var(micro_codegen_t *codegen);

void micro_codegen_386__var(micro_codegen_t *codegen);

void micro_codegen_386__set(micro_codegen_t *codegen);

void micro_codegen_386__fun(micro_codegen_t *codegen);

void micro_codegen_386__call(micro_codegen_t *codegen);

void micro_codegen_386__goto(micro_codegen_t *codegen);

void micro_codegen_386__if(micro_codegen_t *codegen);

#endif