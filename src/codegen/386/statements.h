#ifndef MICRO_CODEGEN_386_STATEMENTS_H
#define MICRO_CODEGEN_386_STATEMENTS_H

#include "common.h"
#include "expr.h"

void micro_codegen_386__static_var();

void micro_codegen_386__var();

void micro_codegen_386__set();

void micro_codegen_386__fun();

void micro_codegen_386__call();

void micro_codegen_386__goto();

#endif