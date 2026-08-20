#ifndef TESTS_CODEGEN386_H
#define TESTS_CODEGEN386_H

#include "../include/munit.h"
#include <micro/micro.h>
#include <micro/asm/asm386.h>
#include <stdio.h>
#include <string.h>

static void cg_gen(const char *text, sct_vector_t *toks, micro_instrgen_t *ig, micro_codegen_t *cg)
{
    sct_vector_init(toks, sizeof(micro_token_t));
    micro_tokenize(text, strlen(text), toks);

    micro_instrgen_init(ig, toks);
    micro_instrgen_gen(ig);

    micro_codegen386_init(cg);
    cg->emit(cg, &ig->instructions);
}

static void cg_cleanup(sct_vector_t *toks, micro_instrgen_t *ig, micro_codegen_t *cg)
{
    micro_codegen386_deinit(cg);
    micro_instrgen_deinit(ig);
    sct_vector_deinit(toks);
}

static micro_asm386_instruction_t *cg_asm(micro_codegen_t *cg, size_t i)
{
    return sct_vector_get(&cg->asm_instrs, i);
}

static void cg_assert_asm_opcode(micro_codegen_t *cg, size_t i, micro_asm386_instruction_type_t opcode)
{
    micro_asm386_instruction_t *instr = cg_asm(cg, i);
    munit_assert_ptr_not_null(instr);
    munit_assert_int((int)instr->opcode, ==, (int)opcode);
}

static void cg_assert_asm_reg(micro_codegen_t *cg, size_t i, int operand, micro_asm386_reg_t reg)
{
    micro_asm386_instruction_t *instr = cg_asm(cg, i);
    munit_assert_ptr_not_null(instr);
    if (operand == 1) {
        munit_assert_int((int)instr->operand1.reg, ==, (int)reg);
    } else {
        munit_assert_int((int)instr->operand2.reg, ==, (int)reg);
    }
}

static void cg_assert_asm_imm(micro_codegen_t *cg, size_t i, int operand, int val)
{
    micro_asm386_instruction_t *instr = cg_asm(cg, i);
    munit_assert_ptr_not_null(instr);
    if (operand == 1) {
        munit_assert_int(instr->operand1.imm.val, ==, val);
    } else {
        munit_assert_int(instr->operand2.imm.val, ==, val);
    }
}

static void cg_assert_asm_lbl(micro_codegen_t *cg, size_t i, int operand, const char *name)
{
    micro_asm386_instruction_t *instr = cg_asm(cg, i);
    munit_assert_ptr_not_null(instr);
    if (operand == 1) {
        munit_assert_string_equal(instr->operand1.lbl_name, name);
    } else {
        munit_assert_string_equal(instr->operand2.lbl_name, name);
    }
}

MunitResult test_codegen_ret_no_val(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t toks;
    micro_instrgen_t ig;
    micro_codegen_t cg;
    cg_gen("fun empty\n"
           "start\n"
           "    ret;\n"
           "end\n", &toks, &ig, &cg);

    munit_assert_size(micro_err_stk_size, ==, 0);
    munit_assert_size(cg.asm_instrs.size, ==, 8);

    cg_assert_asm_opcode(&cg, 0, MICRO_ASM386_INSTR_LBL);
    cg_assert_asm_lbl(&cg, 0, 1, "empty");

    cg_assert_asm_opcode(&cg, 1, MICRO_ASM386_INSTR_PRELUDE);

    cg_assert_asm_opcode(&cg, 2, MICRO_ASM386_INSTR_SUB_R32I32);
    cg_assert_asm_reg(&cg, 2, 1, MICRO_ASM386_REG32_ESP);

    cg_assert_asm_opcode(&cg, 3, MICRO_ASM386_INSTR_LBL);
    cg_assert_asm_lbl(&cg, 3, 1, "empty.start");

    cg_assert_asm_opcode(&cg, 4, MICRO_ASM386_INSTR_JMP_L32);
    cg_assert_asm_lbl(&cg, 4, 1, "empty.end");

    cg_assert_asm_opcode(&cg, 5, MICRO_ASM386_INSTR_LBL);
    cg_assert_asm_lbl(&cg, 5, 1, "empty.end");

    cg_assert_asm_opcode(&cg, 6, MICRO_ASM386_INSTR_ADD_R32I32);
    cg_assert_asm_reg(&cg, 6, 1, MICRO_ASM386_REG32_ESP);

    cg_assert_asm_opcode(&cg, 7, MICRO_ASM386_INSTR_EPILOGUE);

    micro_asm386_emit(&cg.asm_instrs, &cg.outbuf);
    munit_assert_size(cg.outbuf.size, ==, 22);

    cg_cleanup(&toks, &ig, &cg);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_set_i32_lit(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t toks;
    micro_instrgen_t ig;
    micro_codegen_t cg;
    cg_gen("fun f\n"
           "start\n"
           "    set i32 x 5;\n"
           "end\n", &toks, &ig, &cg);

    munit_assert_size(micro_err_stk_size, ==, 0);
    munit_assert_size(cg.asm_instrs.size, ==, 8);

    cg_assert_asm_opcode(&cg, 4, MICRO_ASM386_INSTR_MOV_R32I32);
    cg_assert_asm_reg(&cg, 4, 1, MICRO_ASM386_REG32_EAX);
    cg_assert_asm_imm(&cg, 4, 2, 5);

    cg_cleanup(&toks, &ig, &cg);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_set_i8_lit(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t toks;
    micro_instrgen_t ig;
    micro_codegen_t cg;
    cg_gen("fun f\n"
           "start\n"
           "    set i8 x 5;\n"
           "end\n", &toks, &ig, &cg);

    munit_assert_size(micro_err_stk_size, ==, 0);

    cg_assert_asm_opcode(&cg, 4, MICRO_ASM386_INSTR_MOV_R8I8);
    cg_assert_asm_reg(&cg, 4, 1, MICRO_ASM386_REG32_EAX);
    cg_assert_asm_imm(&cg, 4, 2, 5);

    cg_cleanup(&toks, &ig, &cg);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_set_i16_lit(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t toks;
    micro_instrgen_t ig;
    micro_codegen_t cg;
    cg_gen("fun f\n"
           "start\n"
           "    set i16 x 1000;\n"
           "end\n", &toks, &ig, &cg);

    munit_assert_size(micro_err_stk_size, ==, 0);

    cg_assert_asm_opcode(&cg, 4, MICRO_ASM386_INSTR_MOV_R16I16);
    cg_assert_asm_reg(&cg, 4, 1, MICRO_ASM386_REG32_EAX);
    cg_assert_asm_imm(&cg, 4, 2, 1000);

    cg_cleanup(&toks, &ig, &cg);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_set_u32_lit(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t toks;
    micro_instrgen_t ig;
    micro_codegen_t cg;
    cg_gen("fun f\n"
           "start\n"
           "    set u32 x 7;\n"
           "end\n", &toks, &ig, &cg);

    munit_assert_size(micro_err_stk_size, ==, 0);

    cg_assert_asm_opcode(&cg, 4, MICRO_ASM386_INSTR_MOV_R32I32);
    cg_assert_asm_reg(&cg, 4, 1, MICRO_ASM386_REG32_EAX);
    cg_assert_asm_imm(&cg, 4, 2, 7);

    cg_cleanup(&toks, &ig, &cg);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_set_ptr_lit(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t toks;
    micro_instrgen_t ig;
    micro_codegen_t cg;
    cg_gen("fun f\n"
           "start\n"
           "    set ptr x 8;\n"
           "end\n", &toks, &ig, &cg);

    munit_assert_size(micro_err_stk_size, ==, 0);

    cg_assert_asm_opcode(&cg, 4, MICRO_ASM386_INSTR_MOV_R32I32);
    cg_assert_asm_reg(&cg, 4, 1, MICRO_ASM386_REG32_EAX);
    cg_assert_asm_imm(&cg, 4, 2, 8);

    cg_cleanup(&toks, &ig, &cg);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_set_two_regs(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t toks;
    micro_instrgen_t ig;
    micro_codegen_t cg;
    cg_gen("fun f\n"
           "start\n"
           "    set i32 x 5;\n"
           "    set i32 y 6;\n"
           "end\n", &toks, &ig, &cg);

    munit_assert_size(micro_err_stk_size, ==, 0);
    munit_assert_size(cg.asm_instrs.size, ==, 9);

    cg_assert_asm_opcode(&cg, 4, MICRO_ASM386_INSTR_MOV_R32I32);
    cg_assert_asm_reg(&cg, 4, 1, MICRO_ASM386_REG32_EAX);
    cg_assert_asm_imm(&cg, 4, 2, 5);

    cg_assert_asm_opcode(&cg, 5, MICRO_ASM386_INSTR_MOV_R32I32);
    cg_assert_asm_reg(&cg, 5, 1, MICRO_ASM386_REG32_ECX);
    cg_assert_asm_imm(&cg, 5, 2, 6);

    cg_cleanup(&toks, &ig, &cg);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_reassign_reg(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t toks;
    micro_instrgen_t ig;
    micro_codegen_t cg;
    cg_gen("fun f\n"
           "start\n"
           "    set i32 x 5;\n"
           "    set i32 x 6;\n"
           "end\n", &toks, &ig, &cg);

    munit_assert_size(micro_err_stk_size, ==, 0);
    munit_assert_size(cg.asm_instrs.size, ==, 9);

    // second set reuses the same register of x
    cg_assert_asm_opcode(&cg, 4, MICRO_ASM386_INSTR_MOV_R32I32);
    cg_assert_asm_reg(&cg, 4, 1, MICRO_ASM386_REG32_EAX);
    cg_assert_asm_imm(&cg, 4, 2, 5);

    cg_assert_asm_opcode(&cg, 5, MICRO_ASM386_INSTR_MOV_R32I32);
    cg_assert_asm_reg(&cg, 5, 1, MICRO_ASM386_REG32_EAX);
    cg_assert_asm_imm(&cg, 5, 2, 6);

    cg_cleanup(&toks, &ig, &cg);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_set_plus_lit(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t toks;
    micro_instrgen_t ig;
    micro_codegen_t cg;
    cg_gen("fun f\n"
           "start\n"
           "    set i32 x + 1 2;\n"
           "end\n", &toks, &ig, &cg);

    munit_assert_size(micro_err_stk_size, ==, 0);

    cg_assert_asm_opcode(&cg, 4, MICRO_ASM386_INSTR_MOV_R32I32);
    cg_assert_asm_imm(&cg, 4, 2, 1);
    cg_assert_asm_opcode(&cg, 5, MICRO_ASM386_INSTR_ADD_R32I32);
    cg_assert_asm_imm(&cg, 5, 2, 2);

    cg_cleanup(&toks, &ig, &cg);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_set_minus_lit(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t toks;
    micro_instrgen_t ig;
    micro_codegen_t cg;
    cg_gen("fun f\n"
           "start\n"
           "    set i32 x - 5 2;\n"
           "end\n", &toks, &ig, &cg);

    munit_assert_size(micro_err_stk_size, ==, 0);

    cg_assert_asm_opcode(&cg, 4, MICRO_ASM386_INSTR_MOV_R32I32);
    cg_assert_asm_imm(&cg, 4, 2, 5);
    cg_assert_asm_opcode(&cg, 5, MICRO_ASM386_INSTR_SUB_R32I32);
    cg_assert_asm_imm(&cg, 5, 2, 2);

    cg_cleanup(&toks, &ig, &cg);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_set_vreg_ref(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t toks;
    micro_instrgen_t ig;
    micro_codegen_t cg;
    cg_gen("fun f\n"
           "start\n"
           "    set i32 x 5;\n"
           "    set i32 y + x 1;\n"
           "end\n", &toks, &ig, &cg);

    munit_assert_size(micro_err_stk_size, ==, 0);

    // x -> eax, y -> ecx; y = x + 1
    cg_assert_asm_opcode(&cg, 4, MICRO_ASM386_INSTR_MOV_R32I32);
    cg_assert_asm_imm(&cg, 4, 2, 5);

    cg_assert_asm_opcode(&cg, 5, MICRO_ASM386_INSTR_MOV_R32R32);
    cg_assert_asm_reg(&cg, 5, 1, MICRO_ASM386_REG32_ECX);
    cg_assert_asm_reg(&cg, 5, 2, MICRO_ASM386_REG32_EAX);

    cg_assert_asm_opcode(&cg, 6, MICRO_ASM386_INSTR_ADD_R32I32);
    cg_assert_asm_imm(&cg, 6, 2, 1);

    cg_cleanup(&toks, &ig, &cg);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_set_minus_vreg(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t toks;
    micro_instrgen_t ig;
    micro_codegen_t cg;
    cg_gen("fun f\n"
           "start\n"
           "    set i32 x 5;\n"
           "    set i32 y - x 2;\n"
           "end\n", &toks, &ig, &cg);

    munit_assert_size(micro_err_stk_size, ==, 0);

    // x -> eax, y -> ecx; y = x - 2
    cg_assert_asm_opcode(&cg, 4, MICRO_ASM386_INSTR_MOV_R32I32);
    cg_assert_asm_imm(&cg, 4, 2, 5);

    cg_assert_asm_opcode(&cg, 5, MICRO_ASM386_INSTR_MOV_R32R32);
    cg_assert_asm_reg(&cg, 5, 1, MICRO_ASM386_REG32_ECX);
    cg_assert_asm_reg(&cg, 5, 2, MICRO_ASM386_REG32_EAX);

    cg_assert_asm_opcode(&cg, 6, MICRO_ASM386_INSTR_SUB_R32I32);
    cg_assert_asm_reg(&cg, 6, 1, MICRO_ASM386_REG32_ECX);
    cg_assert_asm_imm(&cg, 6, 2, 2);

    cg_cleanup(&toks, &ig, &cg);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_set_nested_plus(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t toks;
    micro_instrgen_t ig;
    micro_codegen_t cg;
    cg_gen("fun f\n"
           "start\n"
           "    set i32 x + 1 + 2 3;\n"
           "end\n", &toks, &ig, &cg);

    munit_assert_size(micro_err_stk_size, ==, 0);
    munit_assert_size(cg.asm_instrs.size, ==, 10);

    // inner expression + 2 3 is computed first, then 1 is added
    cg_assert_asm_opcode(&cg, 4, MICRO_ASM386_INSTR_MOV_R32I32);
    cg_assert_asm_imm(&cg, 4, 2, 2);
    cg_assert_asm_opcode(&cg, 5, MICRO_ASM386_INSTR_ADD_R32I32);
    cg_assert_asm_imm(&cg, 5, 2, 3);
    cg_assert_asm_opcode(&cg, 6, MICRO_ASM386_INSTR_ADD_R32I32);
    cg_assert_asm_imm(&cg, 6, 2, 1);

    cg_cleanup(&toks, &ig, &cg);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_stack_overflow(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t toks;
    micro_instrgen_t ig;
    micro_codegen_t cg;
    cg_gen("fun f\n"
           "start\n"
           "    set i32 a 1;\n"
           "    set i32 b 2;\n"
           "    set i32 c 3;\n"
           "    set i32 d 4;\n"
           "    set i32 e 5;\n"
           "end\n", &toks, &ig, &cg);

    munit_assert_size(micro_err_stk_size, ==, 0);
    munit_assert_size(cg.asm_instrs.size, ==, 18);

    // a..d use eax/ecx/edx/ebx, e uses esi
    cg_assert_asm_opcode(&cg, 7, MICRO_ASM386_INSTR_MOV_R32I32);
    cg_assert_asm_reg(&cg, 7, 1, MICRO_ASM386_REG32_EAX);
    cg_assert_asm_imm(&cg, 7, 2, 1);

    cg_assert_asm_opcode(&cg, 8, MICRO_ASM386_INSTR_MOV_R32I32);
    cg_assert_asm_reg(&cg, 8, 1, MICRO_ASM386_REG32_ECX);
    cg_assert_asm_imm(&cg, 8, 2, 2);

    cg_assert_asm_opcode(&cg, 9, MICRO_ASM386_INSTR_MOV_R32I32);
    cg_assert_asm_reg(&cg, 9, 1, MICRO_ASM386_REG32_EDX);
    cg_assert_asm_imm(&cg, 9, 2, 3);

    cg_assert_asm_opcode(&cg, 10, MICRO_ASM386_INSTR_MOV_R32I32);
    cg_assert_asm_reg(&cg, 10, 1, MICRO_ASM386_REG32_EBX);
    cg_assert_asm_imm(&cg, 10, 2, 4);

    cg_assert_asm_opcode(&cg, 11, MICRO_ASM386_INSTR_MOV_R32I32);
    cg_assert_asm_reg(&cg, 11, 1, MICRO_ASM386_REG32_ESI);
    cg_assert_asm_imm(&cg, 11, 2, 5);

    // callee-save regs are pushed/popped when a reg above edx was used
    cg_assert_asm_opcode(&cg, 2, MICRO_ASM386_INSTR_PUSH_R32);
    cg_assert_asm_opcode(&cg, 3, MICRO_ASM386_INSTR_PUSH_R32);
    cg_assert_asm_opcode(&cg, 4, MICRO_ASM386_INSTR_PUSH_R32);
    cg_assert_asm_opcode(&cg, 12, MICRO_ASM386_INSTR_POP_R32);
    cg_assert_asm_opcode(&cg, 13, MICRO_ASM386_INSTR_POP_R32);
    cg_assert_asm_opcode(&cg, 14, MICRO_ASM386_INSTR_POP_R32);

    cg_cleanup(&toks, &ig, &cg);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_ret_expr(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t toks;
    micro_instrgen_t ig;
    micro_codegen_t cg;
    cg_gen("fun f\n"
           "    ret i32\n"
           "start\n"
           "    ret 5;\n"
           "end\n", &toks, &ig, &cg);

    munit_assert_size(micro_err_stk_size, ==, 0);
    munit_assert_size(cg.asm_instrs.size, ==, 9);

    cg_assert_asm_opcode(&cg, 4, MICRO_ASM386_INSTR_MOV_R32I32);
    cg_assert_asm_reg(&cg, 4, 1, MICRO_ASM386_REG32_EAX);
    cg_assert_asm_imm(&cg, 4, 2, 5);

    cg_assert_asm_opcode(&cg, 5, MICRO_ASM386_INSTR_JMP_L32);
    cg_assert_asm_lbl(&cg, 5, 1, "f.end");

    cg_cleanup(&toks, &ig, &cg);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_ret_vreg(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t toks;
    micro_instrgen_t ig;
    micro_codegen_t cg;
    cg_gen("fun f\n"
           "    ret i32\n"
           "start\n"
           "    set i32 a 1;\n"
           "    ret a;\n"
           "end\n", &toks, &ig, &cg);

    munit_assert_size(micro_err_stk_size, ==, 0);
    munit_assert_size(cg.asm_instrs.size, ==, 11);

    cg_assert_asm_opcode(&cg, 4, MICRO_ASM386_INSTR_MOV_R32I32);
    cg_assert_asm_imm(&cg, 4, 2, 1);

    // value of a is moved into eax to return it
    cg_assert_asm_opcode(&cg, 6, MICRO_ASM386_INSTR_MOV_R32R32);
    cg_assert_asm_reg(&cg, 6, 1, MICRO_ASM386_REG32_EAX);

    cg_assert_asm_opcode(&cg, 7, MICRO_ASM386_INSTR_JMP_L32);
    cg_assert_asm_lbl(&cg, 7, 1, "f.end");

    cg_cleanup(&toks, &ig, &cg);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_call(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t toks;
    micro_instrgen_t ig;
    micro_codegen_t cg;
    cg_gen("fun add\n"
           "    i32 a\n"
           "    i32 b\n"
           "    ret i32\n"
           "start\n"
           "    ret + a b;\n"
           "end\n"
           "fun main\n"
           "    ret i32\n"
           "start\n"
           "    set i32 res;\n"
           "    call res add 10 5;\n"
           "    ret res;\n"
           "end\n", &toks, &ig, &cg);

    munit_assert_size(micro_err_stk_size, ==, 0);

    int found = 0;
    for (size_t i = 0; i < cg.asm_instrs.size; i++) {
        micro_asm386_instruction_t *instr = cg_asm(&cg, i);
        if (instr && instr->opcode == MICRO_ASM386_INSTR_CALL_L32) {
            munit_assert_string_equal(instr->operand1.lbl_name, "add");
            found = 1;
            break;
        }
    }
    munit_assert_int(found, ==, 1);

    cg_cleanup(&toks, &ig, &cg);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_two_funs(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t toks;
    micro_instrgen_t ig;
    micro_codegen_t cg;
    cg_gen("fun a\n"
           "    ret i32\n"
           "start\n"
           "    ret 1;\n"
           "end\n"
           "fun b\n"
           "start\n"
           "    set i32 y 3;\n"
           "    ret;\n"
           "end\n", &toks, &ig, &cg);

    munit_assert_size(micro_err_stk_size, ==, 0);

    int found_a = 0, found_b = 0;
    for (size_t i = 0; i < cg.asm_instrs.size; i++) {
        micro_asm386_instruction_t *instr = cg_asm(&cg, i);
        if (instr && instr->opcode == MICRO_ASM386_INSTR_LBL) {
            if (!strcmp(instr->operand1.lbl_name, "a")) found_a = 1;
            if (!strcmp(instr->operand1.lbl_name, "b")) found_b = 1;
        }
    }
    munit_assert_int(found_a, ==, 1);
    munit_assert_int(found_b, ==, 1);

    cg_cleanup(&toks, &ig, &cg);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_lbl(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t toks;
    micro_instrgen_t ig;
    micro_codegen_t cg;
    cg_gen("fun f\n"
           "start\n"
           "target:\n"
           "    ret;\n"
           "end\n", &toks, &ig, &cg);

    munit_assert_size(micro_err_stk_size, ==, 0);

    int found = 0;
    for (size_t i = 0; i < cg.asm_instrs.size; i++) {
        micro_asm386_instruction_t *instr = cg_asm(&cg, i);
        if (instr && instr->opcode == MICRO_ASM386_INSTR_LBL) {
            if (!strcmp(instr->operand1.lbl_name, "f.target")) {
                found = 1;
                break;
            }
        }
    }
    munit_assert_int(found, ==, 1);

    cg_cleanup(&toks, &ig, &cg);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_goto(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t toks;
    micro_instrgen_t ig;
    micro_codegen_t cg;
    cg_gen("fun f\n"
           "start\n"
           "target:\n"
           "    goto target;\n"
           "end\n", &toks, &ig, &cg);

    munit_assert_size(micro_err_stk_size, ==, 0);
    munit_assert_size(cg.asm_instrs.size, ==, 10);

    cg_assert_asm_opcode(&cg, 4, MICRO_ASM386_INSTR_LBL);
    cg_assert_asm_lbl(&cg, 4, 1, "f.target");

    cg_assert_asm_opcode(&cg, 5, MICRO_ASM386_INSTR_JMP_L32);
    cg_assert_asm_lbl(&cg, 5, 1, "f.target");

    cg_assert_asm_opcode(&cg, 6, MICRO_ASM386_INSTR_JMP_L32);
    cg_assert_asm_lbl(&cg, 6, 1, "f.end");

    cg_cleanup(&toks, &ig, &cg);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_goto_forward(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t toks;
    micro_instrgen_t ig;
    micro_codegen_t cg;
    cg_gen("fun f\n"
           "start\n"
           "    goto after;\n"
           "after:\n"
           "    ret;\n"
           "end\n", &toks, &ig, &cg);

    munit_assert_size(micro_err_stk_size, ==, 0);

    int found = 0;
    for (size_t i = 0; i < cg.asm_instrs.size; i++) {
        micro_asm386_instruction_t *instr = cg_asm(&cg, i);
        if (instr && instr->opcode == MICRO_ASM386_INSTR_JMP_L32 &&
            !strcmp(instr->operand1.lbl_name, "f.after")) {
            found = 1;
            break;
        }
    }
    munit_assert_int(found, ==, 1);

    cg_cleanup(&toks, &ig, &cg);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_err_goto_undefined(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t toks;
    micro_instrgen_t ig;
    micro_codegen_t cg;
    cg_gen("fun f\n"
           "start\n"
           "    goto nope;\n"
           "end\n", &toks, &ig, &cg);

    munit_assert_size(micro_err_stk_size, ==, 1);
    munit_assert_int((int)micro_err_stk[0].err, ==, (int)MICRO_ERROR_UNDEFINED_IDENT);

    cg_cleanup(&toks, &ig, &cg);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_err_goto_not_lbl(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t toks;
    micro_instrgen_t ig;
    micro_codegen_t cg;
    cg_gen("fun f\n"
           "start\n"
           "    set i32 x 5;\n"
           "    goto x;\n"
           "end\n", &toks, &ig, &cg);

    munit_assert_size(micro_err_stk_size, ==, 1);
    munit_assert_int((int)micro_err_stk[0].err, ==, (int)MICRO_ERROR_IDENT_NOT_LBL);

    cg_cleanup(&toks, &ig, &cg);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_err_goto_outside_scope(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t toks;
    micro_instrgen_t ig;
    micro_codegen_t cg;
    cg_gen("fun a\n"
           "start\n"
           "l1:\n"
           "    ret;\n"
           "end\n"
           "fun b\n"
           "start\n"
           "    goto l1;\n"
           "end\n", &toks, &ig, &cg);

    munit_assert_size(micro_err_stk_size, ==, 1);
    munit_assert_int((int)micro_err_stk[0].err, ==, (int)MICRO_ERROR_LBL_OUTSIDE_SCOPE);

    cg_cleanup(&toks, &ig, &cg);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_err_undefined_ident(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t toks;
    micro_instrgen_t ig;
    micro_codegen_t cg;
    cg_gen("fun f\n"
           "start\n"
           "    set i32 x undefined_name;\n"
           "end\n", &toks, &ig, &cg);

    munit_assert_size(micro_err_stk_size, ==, 1);
    munit_assert_int((int)micro_err_stk[0].err, ==, (int)MICRO_ERROR_UNDEFINED_IDENT);

    cg_cleanup(&toks, &ig, &cg);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_err_undefined_fun(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t toks;
    micro_instrgen_t ig;
    micro_codegen_t cg;
    cg_gen("fun f\n"
           "start\n"
           "    call _ nope 1;\n"
           "end\n", &toks, &ig, &cg);

    munit_assert_size(micro_err_stk_size, ==, 1);
    munit_assert_int((int)micro_err_stk[0].err, ==, (int)MICRO_ERROR_UNDEFINED_FUN);

    cg_cleanup(&toks, &ig, &cg);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_err_too_many_args(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t toks;
    micro_instrgen_t ig;
    micro_codegen_t cg;
    cg_gen("fun add\n"
           "    i32 a\n"
           "    ret i32\n"
           "start\n"
           "    ret a;\n"
           "end\n"
           "fun main\n"
           "start\n"
           "    call _ add 1 2 3;\n"
           "end\n", &toks, &ig, &cg);

    munit_assert_size(micro_err_stk_size, ==, 1);
    munit_assert_int((int)micro_err_stk[0].err, ==, (int)MICRO_ERROR_TOO_MANY_ARGS);

    cg_cleanup(&toks, &ig, &cg);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_err_too_few_args(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t toks;
    micro_instrgen_t ig;
    micro_codegen_t cg;
    cg_gen("fun add\n"
           "    i32 a\n"
           "    i32 b\n"
           "    ret i32\n"
           "start\n"
           "    ret a;\n"
           "end\n"
           "fun main\n"
           "start\n"
           "    call _ add 1;\n"
           "end\n", &toks, &ig, &cg);

    munit_assert_size(micro_err_stk_size, ==, 1);
    munit_assert_int((int)micro_err_stk[0].err, ==, (int)MICRO_ERROR_TOO_FEW_ARGS);

    cg_cleanup(&toks, &ig, &cg);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_err_vreg_type_mismatch(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t toks;
    micro_instrgen_t ig;
    micro_codegen_t cg;
    cg_gen("fun f\n"
           "start\n"
           "    set i32 x 5;\n"
           "    set u32 x 6;\n"
           "end\n", &toks, &ig, &cg);

    munit_assert_size(micro_err_stk_size, ==, 1);
    munit_assert_int((int)micro_err_stk[0].err, ==, (int)MICRO_ERROR_VREG_TYPE_MISMATCH);

    cg_cleanup(&toks, &ig, &cg);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_err_call_result_undef(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t toks;
    micro_instrgen_t ig;
    micro_codegen_t cg;
    cg_gen("fun add\n"
           "    ret i32\n"
           "start\n"
           "    ret 1;\n"
           "end\n"
           "fun f\n"
           "start\n"
           "    set i32 res;\n"
           "    call nope add;\n"
           "    ret;\n"
           "end\n", &toks, &ig, &cg);

    munit_assert_size(micro_err_stk_size, ==, 1);
    munit_assert_int((int)micro_err_stk[0].err, ==, (int)MICRO_ERROR_UNDEFINED_IDENT);

    cg_cleanup(&toks, &ig, &cg);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_err_call_result_type(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t toks;
    micro_instrgen_t ig;
    micro_codegen_t cg;
    cg_gen("fun add\n"
           "    ret i32\n"
           "start\n"
           "    ret 1;\n"
           "end\n"
           "fun f\n"
           "start\n"
           "    set u32 res;\n"
           "    call res add;\n"
           "    ret;\n"
           "end\n", &toks, &ig, &cg);

    munit_assert_size(micro_err_stk_size, ==, 1);
    munit_assert_int((int)micro_err_stk[0].err, ==, (int)MICRO_ERROR_RESULT_TYPE_MISMATCH);

    cg_cleanup(&toks, &ig, &cg);

    micro_deinit();

    return MUNIT_OK;
}

static MunitTest codegen386_tests[] = {
    { "/ret_no_val", test_codegen_ret_no_val, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/set_i32_lit", test_codegen_set_i32_lit, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/set_i8_lit", test_codegen_set_i8_lit, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/set_i16_lit", test_codegen_set_i16_lit, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/set_u32_lit", test_codegen_set_u32_lit, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/set_ptr_lit", test_codegen_set_ptr_lit, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/set_two_regs", test_codegen_set_two_regs, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/reassign_reg", test_codegen_reassign_reg, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/set_plus_lit", test_codegen_set_plus_lit, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/set_minus_lit", test_codegen_set_minus_lit, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/set_vreg_ref", test_codegen_set_vreg_ref, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/set_minus_vreg", test_codegen_set_minus_vreg, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/set_nested_plus", test_codegen_set_nested_plus, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/stack_overflow", test_codegen_stack_overflow, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/ret_expr", test_codegen_ret_expr, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/ret_vreg", test_codegen_ret_vreg, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/call", test_codegen_call, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/two_funs", test_codegen_two_funs, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/lbl", test_codegen_lbl, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/goto", test_codegen_goto, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/goto_forward", test_codegen_goto_forward, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/err_goto_undefined", test_codegen_err_goto_undefined, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/err_goto_not_lbl", test_codegen_err_goto_not_lbl, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/err_goto_outside_scope", test_codegen_err_goto_outside_scope, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/err_undefined_ident", test_codegen_err_undefined_ident, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/err_undefined_fun", test_codegen_err_undefined_fun, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/err_too_many_args", test_codegen_err_too_many_args, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/err_too_few_args", test_codegen_err_too_few_args, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/err_vreg_type_mismatch", test_codegen_err_vreg_type_mismatch, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/err_call_result_undef", test_codegen_err_call_result_undef, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/err_call_result_type", test_codegen_err_call_result_type, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite codegen386_suite = {
    "/codegen386", codegen386_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};

#endif