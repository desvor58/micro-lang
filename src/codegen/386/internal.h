#ifndef MICRO_CODEGEN386_INTERNAL_H
#define MICRO_CODEGEN386_INTERNAL_H

#include <micro/codegen/386/codegen386.h>
#include <micro/asm/asm386.h>

// return 1 if err, else return 0

int lowering(micro_codegen_t *codegen);
int lowering_fun(micro_codegen_t *codegen, micro_instruction_t *instr);
int lowering_ret(micro_codegen_t *codegen, micro_instruction_t *instr);
int lowering_set(micro_codegen_t *codegen, micro_instruction_t *instr);

int asmopting(micro_codegen_t *codegen);

int label_resulting(micro_codegen_t *codegen);


int expr_lit_parse(micro_codegen_t *codegen, micro_codegen386_storage_t dst, i32 imm);
int expr_vreg_parse(micro_codegen_t *codegen, micro_codegen386_storage_t dst, micro_codegen386_ident_vreg_t vreg);

// return offset to next token after expr or 0 if err
int expr_parse(micro_codegen_t *codegen, micro_codegen386_storage_t dst, micro_token_t *start);

// return number of register or ebp offset, do not change ebp_offset
int get_last_free_space(micro_codegen_t *codegen);

#endif