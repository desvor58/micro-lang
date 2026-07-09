#ifndef MICRO_CODEGEN386_INTERNAL_H
#define MICRO_CODEGEN386_INTERNAL_H

#include <micro/codegen/386/codegen386.h>
#include <micro/asm/asm386.h>

// return 1 if err, else return 0

int lowering(micro_codegen_t *codegen);
int lowering_set(micro_codegen_t *codegen, micro_instruction_t *instr);
int lowering_fun(micro_codegen_t *codegen, micro_instruction_t *instr);

int asmopting(micro_codegen_t *codegen);

int label_resulting(micro_codegen_t *codegen);

#endif