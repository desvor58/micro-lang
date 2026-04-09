#ifndef MICRO_CODEGEN_386_H
#define MICRO_CODEGEN_386_H

#include "codegen.h"
#include "386/common.h"
#include "386/statements.h"

void micro_codegen_386_micro_instruction_parse(micro_codegen_t *codegen);

void micro_codegen_386(micro_codegen_t *codegen);

void micro_codegen_386_init(micro_codegen_t *codegen, micro_tok_vec_t *toks);

void micro_codegen_386_deinit(micro_codegen_t *codegen);

#endif