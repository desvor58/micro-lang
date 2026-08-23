#ifndef MICRO_CODEGEN386_INTERNAL_H
#define MICRO_CODEGEN386_INTERNAL_H

#include <micro/codegen/386/codegen386.h>
#include <micro/asm/asm386.h>

#define push_asm_instr(instr, op1, op2)  \
    sct_vector_push(&codegen->asm_instrs, &(micro_asm386_instruction_t){  \
        .opcode = (instr),  \
        .operand1 = (micro_asm386_instruction_operand_t)op1,  \
        .operand2 = (micro_asm386_instruction_operand_t)op2,  \
    })

static const micro_asm386_instruction_type_t movRS_tbl[3] = {
    [MICRO_SIZE_8]  = MICRO_ASM386_INSTR_MOV_R8S32,
    [MICRO_SIZE_16] = MICRO_ASM386_INSTR_MOV_R16S32,
    [MICRO_SIZE_32] = MICRO_ASM386_INSTR_MOV_R32S32,
};
static const micro_asm386_instruction_type_t movMR_tbl[3] = {
    [MICRO_SIZE_8]  = MICRO_ASM386_INSTR_MOV_M8R8,
    [MICRO_SIZE_16] = MICRO_ASM386_INSTR_MOV_M16R16,
    [MICRO_SIZE_32] = MICRO_ASM386_INSTR_MOV_M32R32,
};
static const micro_asm386_instruction_type_t movSR_tbl[3] = {
    [MICRO_SIZE_8]  = MICRO_ASM386_INSTR_MOV_S32R8,
    [MICRO_SIZE_16] = MICRO_ASM386_INSTR_MOV_S32R16,
    [MICRO_SIZE_32] = MICRO_ASM386_INSTR_MOV_S32R32,
};
static const micro_asm386_instruction_type_t movRR_tbl[3] = {
    [MICRO_SIZE_8]  = MICRO_ASM386_INSTR_MOV_R8R8,
    [MICRO_SIZE_16] = MICRO_ASM386_INSTR_MOV_R16R16,
    [MICRO_SIZE_32] = MICRO_ASM386_INSTR_MOV_R32R32,
};

static const micro_asm386_instruction_type_t testRR_tbl[3] = {
    [MICRO_SIZE_8]  = MICRO_ASM386_INSTR_TEST_R8R8,
    [MICRO_SIZE_16] = MICRO_ASM386_INSTR_TEST_R16R16,
    [MICRO_SIZE_32] = MICRO_ASM386_INSTR_TEST_R32R32,
};

// return 1 if err, else return 0

int lowering(micro_codegen_t *codegen);
int lowering_fun(micro_codegen_t *codegen, micro_instruction_t *instr);
int lowering_ret(micro_codegen_t *codegen, micro_instruction_t *instr);
int lowering_set(micro_codegen_t *codegen, micro_instruction_t *instr);
int lowering_call(micro_codegen_t *codegen, micro_instruction_t *instr);
int lowering_lbl(micro_codegen_t *codegen, micro_instruction_t *instr);
int lowering_goto(micro_codegen_t *codegen, micro_instruction_t *instr);
int lowering_if(micro_codegen_t *codegen, micro_instruction_t *instr);

int asmopting(micro_codegen_t *codegen);

int label_resulting(micro_codegen_t *codegen);

size_t expr_lit_parse(micro_codegen_t *codegen, micro_codegen386_storage_t dst, i32 imm);
size_t expr_lbl_parse(micro_codegen_t *codegen, micro_codegen386_storage_t dst, micro_codegen386_ident_lbl_t *lbl);
size_t expr_fun_parse(micro_codegen_t *codegen, micro_codegen386_storage_t dst, micro_codegen386_ident_fun_t *fun);
size_t expr_vreg_parse(micro_codegen_t *codegen, micro_codegen386_storage_t dst, micro_codegen386_ident_vreg_t *vreg);

// return offset to next token after expr or 0 if err
size_t expr_parse(micro_codegen_t *codegen, micro_codegen386_storage_t dst, micro_token_t *start);

// return number of register or ebp offset, do not change ebp_offset
int get_last_free_space(micro_codegen_t *codegen);

// sets only flags
size_t cond_expr_parse(micro_codegen_t *codegen, micro_token_t *start);

#endif