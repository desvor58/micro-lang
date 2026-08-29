#ifndef TESTS_CODEGEN386_H
#define TESTS_CODEGEN386_H

#include "../include/munit.h"
#include "errors.h"
#include <micro/micro.h>
#include <micro/asm/asm386.h>
#include <microc/lexer.h>
#include <microc/instrgen.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    sct_vector_t    toks;
    mc_instrgen_t   ig;
    micro_codegen_t cg;
    sct_vector_t    asm_instrs;
    sct_vector_t    outbuf;
    sct_arena_t     arena;
} cg_ctx_t;

static void cg_gen(cg_ctx_t *ctx, const char *text)
{
    sct_vector_init(&ctx->toks, sizeof(mc_token_t));
    mc_tokenize(text, strlen(text), &ctx->toks);

    mc_instrgen_init(&ctx->ig, &ctx->toks);
    mc_instrgen_gen(&ctx->ig);

    sct_vector_init(&ctx->asm_instrs, sizeof(micro_asm386_instruction_t));
    sct_vector_init(&ctx->outbuf, sizeof(u8));
    sct_arena_init(&ctx->arena);

    micro_codegen386_init(&ctx->cg, (micro_codegen_flags_t){}, &ctx->asm_instrs, &ctx->arena);
    ctx->cg.emit(&ctx->cg, &ctx->ig.instructions);

    test_put_errors("codegen");
}

static void cg_cleanup(cg_ctx_t *ctx)
{
    micro_codegen386_deinit(&ctx->cg);
    mc_instrgen_deinit(&ctx->ig);
    sct_vector_deinit(&ctx->toks);
    sct_vector_deinit(&ctx->asm_instrs);
    sct_arena_deinit(&ctx->arena);
}

static size_t cg_asm_size(cg_ctx_t *ctx)
{
    return ctx->cg.asm_instrs->size;
}

static micro_asm386_instruction_t *cg_asm(cg_ctx_t *ctx, size_t i)
{
    return sct_vector_get(ctx->cg.asm_instrs, i);
}

static void cg_assert_asm_opcode(cg_ctx_t *ctx, size_t i, micro_asm386_instruction_type_t opcode)
{
    micro_asm386_instruction_t *instr = cg_asm(ctx, i);
    munit_assert_ptr_not_null(instr);
    munit_assert_int((int)instr->opcode, ==, (int)opcode);
}

static void cg_assert_asm_reg(cg_ctx_t *ctx, size_t i, int operand, micro_asm386_reg_t reg)
{
    micro_asm386_instruction_t *instr = cg_asm(ctx, i);
    munit_assert_ptr_not_null(instr);
    if (operand == 1) {
        munit_assert_int((int)instr->operand1.reg, ==, (int)reg);
    } else {
        munit_assert_int((int)instr->operand2.reg, ==, (int)reg);
    }
}

static void cg_assert_asm_imm(cg_ctx_t *ctx, size_t i, int operand, int val)
{
    micro_asm386_instruction_t *instr = cg_asm(ctx, i);
    munit_assert_ptr_not_null(instr);
    if (operand == 1) {
        munit_assert_int(instr->operand1.imm.val, ==, val);
    } else {
        munit_assert_int(instr->operand2.imm.val, ==, val);
    }
}

static void cg_assert_asm_lbl(cg_ctx_t *ctx, size_t i, int operand, const char *name)
{
    micro_asm386_instruction_t *instr = cg_asm(ctx, i);
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

    cg_ctx_t ctx;
    cg_gen(&ctx, "fun empty\n"
                 "start\n"
                 "    ret;\n"
                 "end\n");

    munit_assert_size(micro_err_stk_size, ==, 0);
    munit_assert_size(cg_asm_size(&ctx), ==, 8);

    cg_assert_asm_opcode(&ctx, 0, MICRO_ASM386_INSTR_LBL);
    cg_assert_asm_lbl(&ctx, 0, 1, "empty");

    cg_assert_asm_opcode(&ctx, 1, MICRO_ASM386_INSTR_PRELUDE);

    cg_assert_asm_opcode(&ctx, 2, MICRO_ASM386_INSTR_SUB_R32I32);
    cg_assert_asm_reg(&ctx, 2, 1, MICRO_ASM386_REG32_ESP);

    cg_assert_asm_opcode(&ctx, 3, MICRO_ASM386_INSTR_LBL);
    cg_assert_asm_lbl(&ctx, 3, 1, "empty.start");

    cg_assert_asm_opcode(&ctx, 4, MICRO_ASM386_INSTR_JMP_L32);
    cg_assert_asm_lbl(&ctx, 4, 1, "empty.end");

    cg_assert_asm_opcode(&ctx, 5, MICRO_ASM386_INSTR_LBL);
    cg_assert_asm_lbl(&ctx, 5, 1, "empty.end");

    cg_assert_asm_opcode(&ctx, 6, MICRO_ASM386_INSTR_ADD_R32I32);
    cg_assert_asm_reg(&ctx, 6, 1, MICRO_ASM386_REG32_ESP);

    cg_assert_asm_opcode(&ctx, 7, MICRO_ASM386_INSTR_EPILOGUE);

    micro_asm386_emit(&ctx.asm_instrs, &ctx.outbuf);
    munit_assert_size(ctx.outbuf.size, ==, 22);

    cg_cleanup(&ctx);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_set_i32_lit(const MunitParameter params[], void *data)
{
    micro_init();

    cg_ctx_t ctx;
    cg_gen(&ctx, "fun f\n"
                 "start\n"
                 "    set i32 x 5;\n"
                 "end\n");

    munit_assert_size(micro_err_stk_size, ==, 0);
    munit_assert_size(cg_asm_size(&ctx), ==, 8);

    cg_assert_asm_opcode(&ctx, 4, MICRO_ASM386_INSTR_MOV_R32I32);
    cg_assert_asm_reg(&ctx, 4, 1, MICRO_ASM386_REG32_EAX);
    cg_assert_asm_imm(&ctx, 4, 2, 5);

    cg_cleanup(&ctx);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_set_i8_lit(const MunitParameter params[], void *data)
{
    micro_init();

    cg_ctx_t ctx;
    cg_gen(&ctx, "fun f\n"
                 "start\n"
                 "    set i8 x 5;\n"
                 "end\n");

    munit_assert_size(micro_err_stk_size, ==, 0);

    cg_assert_asm_opcode(&ctx, 4, MICRO_ASM386_INSTR_MOV_R8I8);
    cg_assert_asm_reg(&ctx, 4, 1, MICRO_ASM386_REG32_EAX);
    cg_assert_asm_imm(&ctx, 4, 2, 5);

    cg_cleanup(&ctx);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_set_i16_lit(const MunitParameter params[], void *data)
{
    micro_init();

    cg_ctx_t ctx;
    cg_gen(&ctx, "fun f\n"
                 "start\n"
                 "    set i16 x 1000;\n"
                 "end\n");

    munit_assert_size(micro_err_stk_size, ==, 0);

    cg_assert_asm_opcode(&ctx, 4, MICRO_ASM386_INSTR_MOV_R16I16);
    cg_assert_asm_reg(&ctx, 4, 1, MICRO_ASM386_REG32_EAX);
    cg_assert_asm_imm(&ctx, 4, 2, 1000);

    cg_cleanup(&ctx);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_set_u32_lit(const MunitParameter params[], void *data)
{
    micro_init();

    cg_ctx_t ctx;
    cg_gen(&ctx, "fun f\n"
                 "start\n"
                 "    set u32 x 7;\n"
                 "end\n");

    munit_assert_size(micro_err_stk_size, ==, 0);

    cg_assert_asm_opcode(&ctx, 4, MICRO_ASM386_INSTR_MOV_R32I32);
    cg_assert_asm_reg(&ctx, 4, 1, MICRO_ASM386_REG32_EAX);
    cg_assert_asm_imm(&ctx, 4, 2, 7);

    cg_cleanup(&ctx);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_set_ptr_lit(const MunitParameter params[], void *data)
{
    micro_init();

    cg_ctx_t ctx;
    cg_gen(&ctx, "fun f\n"
                 "start\n"
                 "    set ptr x 8;\n"
                 "end\n");

    munit_assert_size(micro_err_stk_size, ==, 0);

    cg_assert_asm_opcode(&ctx, 4, MICRO_ASM386_INSTR_MOV_R32I32);
    cg_assert_asm_reg(&ctx, 4, 1, MICRO_ASM386_REG32_EAX);
    cg_assert_asm_imm(&ctx, 4, 2, 8);

    cg_cleanup(&ctx);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_set_two_regs(const MunitParameter params[], void *data)
{
    micro_init();

    cg_ctx_t ctx;
    cg_gen(&ctx, "fun f\n"
                 "start\n"
                 "    set i32 x 5;\n"
                 "    set i32 y 6;\n"
                 "end\n");

    munit_assert_size(micro_err_stk_size, ==, 0);
    munit_assert_size(cg_asm_size(&ctx), ==, 9);

    cg_assert_asm_opcode(&ctx, 4, MICRO_ASM386_INSTR_MOV_R32I32);
    cg_assert_asm_reg(&ctx, 4, 1, MICRO_ASM386_REG32_EAX);
    cg_assert_asm_imm(&ctx, 4, 2, 5);

    cg_assert_asm_opcode(&ctx, 5, MICRO_ASM386_INSTR_MOV_R32I32);
    cg_assert_asm_reg(&ctx, 5, 1, MICRO_ASM386_REG32_ECX);
    cg_assert_asm_imm(&ctx, 5, 2, 6);

    cg_cleanup(&ctx);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_reassign_reg(const MunitParameter params[], void *data)
{
    micro_init();

    cg_ctx_t ctx;
    cg_gen(&ctx, "fun f\n"
                 "start\n"
                 "    set i32 x 5;\n"
                 "    set i32 x 6;\n"
                 "end\n");

    munit_assert_size(micro_err_stk_size, ==, 0);
    munit_assert_size(cg_asm_size(&ctx), ==, 9);

    // second set reuses the same register of x
    cg_assert_asm_opcode(&ctx, 4, MICRO_ASM386_INSTR_MOV_R32I32);
    cg_assert_asm_reg(&ctx, 4, 1, MICRO_ASM386_REG32_EAX);
    cg_assert_asm_imm(&ctx, 4, 2, 5);

    cg_assert_asm_opcode(&ctx, 5, MICRO_ASM386_INSTR_MOV_R32I32);
    cg_assert_asm_reg(&ctx, 5, 1, MICRO_ASM386_REG32_EAX);
    cg_assert_asm_imm(&ctx, 5, 2, 6);

    cg_cleanup(&ctx);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_set_plus_lit(const MunitParameter params[], void *data)
{
    micro_init();

    cg_ctx_t ctx;
    cg_gen(&ctx, "fun f\n"
                 "start\n"
                 "    set i32 x + 1 2;\n"
                 "end\n");

    munit_assert_size(micro_err_stk_size, ==, 0);

    cg_assert_asm_opcode(&ctx, 4, MICRO_ASM386_INSTR_MOV_R32I32);
    cg_assert_asm_imm(&ctx, 4, 2, 1);
    cg_assert_asm_opcode(&ctx, 5, MICRO_ASM386_INSTR_ADD_R32I32);
    cg_assert_asm_imm(&ctx, 5, 2, 2);

    cg_cleanup(&ctx);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_set_minus_lit(const MunitParameter params[], void *data)
{
    micro_init();

    cg_ctx_t ctx;
    cg_gen(&ctx, "fun f\n"
                 "start\n"
                 "    set i32 x - 5 2;\n"
                 "end\n");

    munit_assert_size(micro_err_stk_size, ==, 0);

    cg_assert_asm_opcode(&ctx, 4, MICRO_ASM386_INSTR_MOV_R32I32);
    cg_assert_asm_imm(&ctx, 4, 2, 5);
    cg_assert_asm_opcode(&ctx, 5, MICRO_ASM386_INSTR_SUB_R32I32);
    cg_assert_asm_imm(&ctx, 5, 2, 2);

    cg_cleanup(&ctx);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_set_vreg_ref(const MunitParameter params[], void *data)
{
    micro_init();

    cg_ctx_t ctx;
    cg_gen(&ctx, "fun f\n"
                 "start\n"
                 "    set i32 x 5;\n"
                 "    set i32 y + x 1;\n"
                 "end\n");

    munit_assert_size(micro_err_stk_size, ==, 0);

    // x -> eax, y -> ecx; y = x + 1
    cg_assert_asm_opcode(&ctx, 4, MICRO_ASM386_INSTR_MOV_R32I32);
    cg_assert_asm_imm(&ctx, 4, 2, 5);

    cg_assert_asm_opcode(&ctx, 5, MICRO_ASM386_INSTR_MOV_R32R32);
    cg_assert_asm_reg(&ctx, 5, 1, MICRO_ASM386_REG32_ECX);
    cg_assert_asm_reg(&ctx, 5, 2, MICRO_ASM386_REG32_EAX);

    cg_assert_asm_opcode(&ctx, 6, MICRO_ASM386_INSTR_ADD_R32I32);
    cg_assert_asm_imm(&ctx, 6, 2, 1);

    cg_cleanup(&ctx);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_set_minus_vreg(const MunitParameter params[], void *data)
{
    micro_init();

    cg_ctx_t ctx;
    cg_gen(&ctx, "fun f\n"
                 "start\n"
                 "    set i32 x 5;\n"
                 "    set i32 y - x 2;\n"
                 "end\n");

    munit_assert_size(micro_err_stk_size, ==, 0);

    // x -> eax, y -> ecx; y = x - 2
    cg_assert_asm_opcode(&ctx, 4, MICRO_ASM386_INSTR_MOV_R32I32);
    cg_assert_asm_imm(&ctx, 4, 2, 5);

    cg_assert_asm_opcode(&ctx, 5, MICRO_ASM386_INSTR_MOV_R32R32);
    cg_assert_asm_reg(&ctx, 5, 1, MICRO_ASM386_REG32_ECX);
    cg_assert_asm_reg(&ctx, 5, 2, MICRO_ASM386_REG32_EAX);

    cg_assert_asm_opcode(&ctx, 6, MICRO_ASM386_INSTR_SUB_R32I32);
    cg_assert_asm_reg(&ctx, 6, 1, MICRO_ASM386_REG32_ECX);
    cg_assert_asm_imm(&ctx, 6, 2, 2);

    cg_cleanup(&ctx);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_set_nested_plus(const MunitParameter params[], void *data)
{
    micro_init();

    cg_ctx_t ctx;
    cg_gen(&ctx, "fun f\n"
                 "start\n"
                 "    set i32 x + 1 + 2 3;\n"
                 "end\n");

    munit_assert_size(micro_err_stk_size, ==, 0);
    munit_assert_size(cg_asm_size(&ctx), ==, 10);

    // inner expression + 2 3 is computed first, then 1 is added
    cg_assert_asm_opcode(&ctx, 4, MICRO_ASM386_INSTR_MOV_R32I32);
    cg_assert_asm_imm(&ctx, 4, 2, 2);
    cg_assert_asm_opcode(&ctx, 5, MICRO_ASM386_INSTR_ADD_R32I32);
    cg_assert_asm_imm(&ctx, 5, 2, 3);
    cg_assert_asm_opcode(&ctx, 6, MICRO_ASM386_INSTR_ADD_R32I32);
    cg_assert_asm_imm(&ctx, 6, 2, 1);

    cg_cleanup(&ctx);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_stack_overflow(const MunitParameter params[], void *data)
{
    micro_init();

    cg_ctx_t ctx;
    cg_gen(&ctx, "fun f\n"
                 "start\n"
                 "    set i32 a 1;\n"
                 "    set i32 b 2;\n"
                 "    set i32 c 3;\n"
                 "    set i32 d 4;\n"
                 "    set i32 e 5;\n"
                 "end\n");

    munit_assert_size(micro_err_stk_size, ==, 0);
    munit_assert_size(cg_asm_size(&ctx), ==, 18);

    // a..d use eax/ecx/edx/ebx, e uses esi
    cg_assert_asm_opcode(&ctx, 7, MICRO_ASM386_INSTR_MOV_R32I32);
    cg_assert_asm_reg(&ctx, 7, 1, MICRO_ASM386_REG32_EAX);
    cg_assert_asm_imm(&ctx, 7, 2, 1);

    cg_assert_asm_opcode(&ctx, 8, MICRO_ASM386_INSTR_MOV_R32I32);
    cg_assert_asm_reg(&ctx, 8, 1, MICRO_ASM386_REG32_ECX);
    cg_assert_asm_imm(&ctx, 8, 2, 2);

    cg_assert_asm_opcode(&ctx, 9, MICRO_ASM386_INSTR_MOV_R32I32);
    cg_assert_asm_reg(&ctx, 9, 1, MICRO_ASM386_REG32_EDX);
    cg_assert_asm_imm(&ctx, 9, 2, 3);

    cg_assert_asm_opcode(&ctx, 10, MICRO_ASM386_INSTR_MOV_R32I32);
    cg_assert_asm_reg(&ctx, 10, 1, MICRO_ASM386_REG32_EBX);
    cg_assert_asm_imm(&ctx, 10, 2, 4);

    cg_assert_asm_opcode(&ctx, 11, MICRO_ASM386_INSTR_MOV_R32I32);
    cg_assert_asm_reg(&ctx, 11, 1, MICRO_ASM386_REG32_ESI);
    cg_assert_asm_imm(&ctx, 11, 2, 5);

    // callee-save regs are pushed/popped when a reg above edx was used
    cg_assert_asm_opcode(&ctx, 2, MICRO_ASM386_INSTR_PUSH_R32);
    cg_assert_asm_opcode(&ctx, 3, MICRO_ASM386_INSTR_PUSH_R32);
    cg_assert_asm_opcode(&ctx, 4, MICRO_ASM386_INSTR_PUSH_R32);
    cg_assert_asm_opcode(&ctx, 13, MICRO_ASM386_INSTR_POP_R32);
    cg_assert_asm_opcode(&ctx, 14, MICRO_ASM386_INSTR_POP_R32);
    cg_assert_asm_opcode(&ctx, 15, MICRO_ASM386_INSTR_POP_R32);

    cg_cleanup(&ctx);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_ret_expr(const MunitParameter params[], void *data)
{
    micro_init();

    cg_ctx_t ctx;
    cg_gen(&ctx, "fun f\n"
                 "    ret i32\n"
                 "start\n"
                 "    ret 5;\n"
                 "end\n");

    munit_assert_size(micro_err_stk_size, ==, 0);
    munit_assert_size(cg_asm_size(&ctx), ==, 9);

    cg_assert_asm_opcode(&ctx, 4, MICRO_ASM386_INSTR_MOV_R32I32);
    cg_assert_asm_reg(&ctx, 4, 1, MICRO_ASM386_REG32_EAX);
    cg_assert_asm_imm(&ctx, 4, 2, 5);

    cg_assert_asm_opcode(&ctx, 5, MICRO_ASM386_INSTR_JMP_L32);
    cg_assert_asm_lbl(&ctx, 5, 1, "f.end");

    cg_cleanup(&ctx);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_ret_vreg(const MunitParameter params[], void *data)
{
    micro_init();

    cg_ctx_t ctx;
    cg_gen(&ctx, "fun f\n"
                 "    ret i32\n"
                 "start\n"
                 "    set i32 a 1;\n"
                 "    ret a;\n"
                 "end\n");

    munit_assert_size(micro_err_stk_size, ==, 0);
    munit_assert_size(cg_asm_size(&ctx), ==, 11);

    cg_assert_asm_opcode(&ctx, 4, MICRO_ASM386_INSTR_MOV_R32I32);
    cg_assert_asm_imm(&ctx, 4, 2, 1);

    // value of a is moved into eax to return it
    cg_assert_asm_opcode(&ctx, 6, MICRO_ASM386_INSTR_MOV_R32R32);
    cg_assert_asm_reg(&ctx, 6, 1, MICRO_ASM386_REG32_EAX);

    cg_assert_asm_opcode(&ctx, 7, MICRO_ASM386_INSTR_JMP_L32);
    cg_assert_asm_lbl(&ctx, 7, 1, "f.end");

    cg_cleanup(&ctx);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_call(const MunitParameter params[], void *data)
{
    micro_init();

    cg_ctx_t ctx;
    cg_gen(&ctx, "fun add\n"
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
                 "end\n");

    munit_assert_size(micro_err_stk_size, ==, 0);

    int found = 0;
    for (size_t i = 0; i < cg_asm_size(&ctx); i++) {
        micro_asm386_instruction_t *instr = cg_asm(&ctx, i);
        if (instr && instr->opcode == MICRO_ASM386_INSTR_CALL_L32) {
            munit_assert_string_equal(instr->operand1.lbl_name, "add");
            found = 1;
            break;
        }
    }
    munit_assert_int(found, ==, 1);

    cg_cleanup(&ctx);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_two_funs(const MunitParameter params[], void *data)
{
    micro_init();

    cg_ctx_t ctx;
    cg_gen(&ctx, "fun a\n"
                 "    ret i32\n"
                 "start\n"
                 "    ret 1;\n"
                 "end\n"
                 "fun b\n"
                 "start\n"
                 "    set i32 y 3;\n"
                 "    ret;\n"
                 "end\n");

    munit_assert_size(micro_err_stk_size, ==, 0);

    int found_a = 0, found_b = 0;
    for (size_t i = 0; i < cg_asm_size(&ctx); i++) {
        micro_asm386_instruction_t *instr = cg_asm(&ctx, i);
        if (instr && instr->opcode == MICRO_ASM386_INSTR_LBL) {
            if (!strcmp(instr->operand1.lbl_name, "a")) found_a = 1;
            if (!strcmp(instr->operand1.lbl_name, "b")) found_b = 1;
        }
    }
    munit_assert_int(found_a, ==, 1);
    munit_assert_int(found_b, ==, 1);

    cg_cleanup(&ctx);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_lbl(const MunitParameter params[], void *data)
{
    micro_init();

    cg_ctx_t ctx;
    cg_gen(&ctx, "fun f\n"
                 "start\n"
                 "target:\n"
                 "    ret;\n"
                 "end\n");

    munit_assert_size(micro_err_stk_size, ==, 0);

    int found = 0;
    for (size_t i = 0; i < cg_asm_size(&ctx); i++) {
        micro_asm386_instruction_t *instr = cg_asm(&ctx, i);
        if (instr && instr->opcode == MICRO_ASM386_INSTR_LBL) {
            if (!strcmp(instr->operand1.lbl_name, "f.target")) {
                found = 1;
                break;
            }
        }
    }
    munit_assert_int(found, ==, 1);

    cg_cleanup(&ctx);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_goto(const MunitParameter params[], void *data)
{
    micro_init();

    cg_ctx_t ctx;
    cg_gen(&ctx, "fun f\n"
                 "start\n"
                 "target:\n"
                 "    goto target;\n"
                 "end\n");

    munit_assert_size(micro_err_stk_size, ==, 0);
    munit_assert_size(cg_asm_size(&ctx), ==, 9);

    cg_assert_asm_opcode(&ctx, 4, MICRO_ASM386_INSTR_LBL);
    cg_assert_asm_lbl(&ctx, 4, 1, "f.target");

    cg_assert_asm_opcode(&ctx, 5, MICRO_ASM386_INSTR_JMP_L32);
    cg_assert_asm_lbl(&ctx, 5, 1, "f.target");

    cg_assert_asm_opcode(&ctx, 6, MICRO_ASM386_INSTR_LBL);
    cg_assert_asm_lbl(&ctx, 6, 1, "f.end");

    cg_cleanup(&ctx);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_goto_forward(const MunitParameter params[], void *data)
{
    micro_init();

    cg_ctx_t ctx;
    cg_gen(&ctx, "fun f\n"
                 "start\n"
                 "    goto after;\n"
                 "after:\n"
                 "    ret;\n"
                 "end\n");

    munit_assert_size(micro_err_stk_size, ==, 0);

    int found = 0;
    for (size_t i = 0; i < cg_asm_size(&ctx); i++) {
        micro_asm386_instruction_t *instr = cg_asm(&ctx, i);
        if (instr && instr->opcode == MICRO_ASM386_INSTR_JMP_L32 &&
            !strcmp(instr->operand1.lbl_name, "f.after")) {
            found = 1;
            break;
        }
    }
    munit_assert_int(found, ==, 1);

    cg_cleanup(&ctx);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_if_vreg(const MunitParameter params[], void *data)
{
    micro_init();

    cg_ctx_t ctx;
    cg_gen(&ctx, "fun f\n"
                 "start\n"
                 "    set i32 n 1;\n"
                 "    if n : target;\n"
                 "    ret;\n"
                 "target:\n"
                 "    ret;\n"
                 "end\n");

    munit_assert_size(micro_err_stk_size, ==, 0);
    munit_assert_size(cg_asm_size(&ctx), ==, 14);

    // if n jumps to f.target when n is non-zero
    cg_assert_asm_opcode(&ctx, 5, MICRO_ASM386_INSTR_TEST_R32R32);
    cg_assert_asm_reg(&ctx, 5, 1, MICRO_ASM386_REG32_EAX);

    cg_assert_asm_opcode(&ctx, 6, MICRO_ASM386_INSTR_JNZ_L32);
    cg_assert_asm_lbl(&ctx, 6, 1, "f.target");

    cg_assert_asm_opcode(&ctx, 9, MICRO_ASM386_INSTR_LBL);
    cg_assert_asm_lbl(&ctx, 9, 1, "f.target");

    cg_cleanup(&ctx);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_if_not(const MunitParameter params[], void *data)
{
    micro_init();

    cg_ctx_t ctx;
    cg_gen(&ctx, "fun f\n"
                 "start\n"
                 "    set i32 n 0;\n"
                 "    if ! n : target;\n"
                 "    ret;\n"
                 "target:\n"
                 "    ret;\n"
                 "end\n");

    munit_assert_size(micro_err_stk_size, ==, 0);
    munit_assert_size(cg_asm_size(&ctx), ==, 14);

    // if !n jumps to f.target when n is zero (JZ instead of JNZ)
    cg_assert_asm_opcode(&ctx, 5, MICRO_ASM386_INSTR_TEST_R32R32);

    cg_assert_asm_opcode(&ctx, 6, MICRO_ASM386_INSTR_JZ_L32);
    cg_assert_asm_lbl(&ctx, 6, 1, "f.target");

    cg_assert_asm_opcode(&ctx, 9, MICRO_ASM386_INSTR_LBL);
    cg_assert_asm_lbl(&ctx, 9, 1, "f.target");

    cg_cleanup(&ctx);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_if_eq(const MunitParameter params[], void *data)
{
    micro_init();

    cg_ctx_t ctx;
    cg_gen(&ctx, "fun f\n"
                 "    i32 n\n"
                 "start\n"
                 "    if = n 1 : target;\n"
                 "    ret;\n"
                 "target:\n"
                 "    ret;\n"
                 "end\n");

    munit_assert_size(micro_err_stk_size, ==, 0);

    // comparison generates a CMP and a SET* ; then a TEST + JNZ jump
    int found_jnz = 0;
    for (size_t i = 0; i < cg_asm_size(&ctx); i++) {
        micro_asm386_instruction_t *instr = cg_asm(&ctx, i);
        if (instr && instr->opcode == MICRO_ASM386_INSTR_JNZ_L32 &&
            !strcmp(instr->operand1.lbl_name, "f.target")) {
            found_jnz = 1;
            break;
        }
    }
    munit_assert_int(found_jnz, ==, 1);

    cg_cleanup(&ctx);

    micro_deinit();

    return MUNIT_OK;
}

/* The docs (docs/micro-language-ref.md #Conditional jumps) allow any
 * comparison as an if condition (e.g. `if > n 1 : target;`). The 386
 * backend currently lowers only `=` (and vreg / !vreg). These tests are
 * marked TODO: they document the intended CMP + Jcc lowering and start
 * passing when the comparisons are implemented. They must not crash: an
 * unimplemented condition merely reports EXPECTED_EXPRESSION. */
static void codegen_if_cmp_todo(cg_ctx_t *ctx, const char *cond)
{
    char text[256];
    snprintf(text, sizeof(text),
             "fun f\n"
             "    i32 n\n"
             "start\n"
             "    if %s n 1 : target;\n"
             "    ret;\n"
             "target:\n"
             "    ret;\n"
             "end\n", cond);
    cg_gen(ctx, text);

    /* documented behaviour: no errors, a conditional jump to f.target */
    munit_assert_size(micro_err_stk_size, ==, 0);

    int found_jcc = 0;
    for (size_t i = 0; i < cg_asm_size(ctx); i++) {
        micro_asm386_instruction_t *instr = cg_asm(ctx, i);
        if (instr && !strcmp(instr->operand1.lbl_name, "f.target") &&
            (instr->opcode == MICRO_ASM386_INSTR_JZ_L32 ||
             instr->opcode == MICRO_ASM386_INSTR_JNZ_L32)) {
            found_jcc = 1;
            break;
        }
    }
    munit_assert_int(found_jcc, ==, 1);
}

MunitResult test_codegen_if_great(const MunitParameter params[], void *data)
{
    micro_init();

    cg_ctx_t ctx;
    codegen_if_cmp_todo(&ctx, ">");
    cg_cleanup(&ctx);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_if_less(const MunitParameter params[], void *data)
{
    micro_init();

    cg_ctx_t ctx;
    codegen_if_cmp_todo(&ctx, "<");
    cg_cleanup(&ctx);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_if_great_or_eq(const MunitParameter params[], void *data)
{
    micro_init();

    cg_ctx_t ctx;
    codegen_if_cmp_todo(&ctx, ">=");
    cg_cleanup(&ctx);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_if_less_or_eq(const MunitParameter params[], void *data)
{
    micro_init();

    cg_ctx_t ctx;
    codegen_if_cmp_todo(&ctx, "<=");
    cg_cleanup(&ctx);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_err_if_outside_function(const MunitParameter params[], void *data)
{
    micro_init();

    cg_ctx_t ctx;
    cg_gen(&ctx, "if n : target;\n");

    munit_assert_size(micro_err_stk_size, ==, 1);
    munit_assert_int((int)micro_err_stk[0].err, ==, (int)MICRO_ERROR_IF_OUTSIDE_FUNCTION);

    cg_cleanup(&ctx);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_err_if_undefined_ident(const MunitParameter params[], void *data)
{
    micro_init();

    cg_ctx_t ctx;
    cg_gen(&ctx, "fun f\n"
                 "start\n"
                 "    if nope : target;\n"
                 "target:\n"
                 "    ret;\n"
                 "end\n");

    munit_assert_size(micro_err_stk_size, >=, 1);
    munit_assert_int((int)micro_err_stk[0].err, ==, (int)MICRO_ERROR_UNDEFINED_IDENT);

    cg_cleanup(&ctx);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_err_if_undefined_label(const MunitParameter params[], void *data)
{
    micro_init();

    cg_ctx_t ctx;
    cg_gen(&ctx, "fun f\n"
                 "start\n"
                 "    set i32 n 1;\n"
                 "    if n : nope;\n"
                 "end\n");

    munit_assert_size(micro_err_stk_size, ==, 1);
    munit_assert_int((int)micro_err_stk[0].err, ==, (int)MICRO_ERROR_UNDEFINED_LBL);

    cg_cleanup(&ctx);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_err_if_not_lbl(const MunitParameter params[], void *data)
{
    micro_init();

    cg_ctx_t ctx;
    cg_gen(&ctx, "fun f\n"
                 "start\n"
                 "    set i32 n 1;\n"
                 "    if n : n;\n"
                 "end\n");

    munit_assert_size(micro_err_stk_size, ==, 1);
    munit_assert_int((int)micro_err_stk[0].err, ==, (int)MICRO_ERROR_IDENT_NOT_LBL);

    cg_cleanup(&ctx);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_err_goto_undefined(const MunitParameter params[], void *data)
{
    micro_init();

    cg_ctx_t ctx;
    cg_gen(&ctx, "fun f\n"
                 "start\n"
                 "    goto nope;\n"
                 "end\n");

    munit_assert_size(micro_err_stk_size, ==, 1);
    munit_assert_int((int)micro_err_stk[0].err, ==, (int)MICRO_ERROR_UNDEFINED_LBL);

    cg_cleanup(&ctx);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_err_goto_not_lbl(const MunitParameter params[], void *data)
{
    micro_init();

    cg_ctx_t ctx;
    cg_gen(&ctx, "fun f\n"
                 "start\n"
                 "    set i32 x 5;\n"
                 "    goto x;\n"
                 "end\n");

    munit_assert_size(micro_err_stk_size, ==, 1);
    munit_assert_int((int)micro_err_stk[0].err, ==, (int)MICRO_ERROR_IDENT_NOT_LBL);

    cg_cleanup(&ctx);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_err_goto_outside_scope(const MunitParameter params[], void *data)
{
    micro_init();

    cg_ctx_t ctx;
    cg_gen(&ctx, "fun a\n"
                 "start\n"
                 "l1:\n"
                 "    ret;\n"
                 "end\n"
                 "fun b\n"
                 "start\n"
                 "    goto l1;\n"
                 "end\n");

    munit_assert_size(micro_err_stk_size, ==, 1);
    munit_assert_int((int)micro_err_stk[0].err, ==, (int)MICRO_ERROR_UNDEFINED_LBL);

    cg_cleanup(&ctx);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_err_goto_outside_function(const MunitParameter params[], void *data)
{
    micro_init();

    cg_ctx_t ctx;
    cg_gen(&ctx, "goto l1;\n");

    munit_assert_size(micro_err_stk_size, ==, 1);
    munit_assert_int((int)micro_err_stk[0].err, ==, (int)MICRO_ERROR_GOTO_OUTSIDE_FUNCTION);

    cg_cleanup(&ctx);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_err_undefined_ident(const MunitParameter params[], void *data)
{
    micro_init();

    cg_ctx_t ctx;
    cg_gen(&ctx, "fun f\n"
                 "start\n"
                 "    set i32 x undefined_name;\n"
                 "end\n");

    munit_assert_size(micro_err_stk_size, ==, 1);
    munit_assert_int((int)micro_err_stk[0].err, ==, (int)MICRO_ERROR_UNDEFINED_IDENT);

    cg_cleanup(&ctx);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_err_undefined_fun(const MunitParameter params[], void *data)
{
    micro_init();

    cg_ctx_t ctx;
    cg_gen(&ctx, "fun f\n"
                 "start\n"
                 "    call _ nope 1;\n"
                 "end\n");

    munit_assert_size(micro_err_stk_size, ==, 1);
    munit_assert_int((int)micro_err_stk[0].err, ==, (int)MICRO_ERROR_UNDEFINED_FUN);

    cg_cleanup(&ctx);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_err_too_many_args(const MunitParameter params[], void *data)
{
    micro_init();

    cg_ctx_t ctx;
    cg_gen(&ctx, "fun add\n"
                 "    i32 a\n"
                 "    ret i32\n"
                 "start\n"
                 "    ret a;\n"
                 "end\n"
                 "fun main\n"
                 "start\n"
                 "    call _ add 1 2 3;\n"
                 "end\n");

    munit_assert_size(micro_err_stk_size, ==, 1);
    munit_assert_int((int)micro_err_stk[0].err, ==, (int)MICRO_ERROR_TOO_MANY_ARGS);

    cg_cleanup(&ctx);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_err_too_few_args(const MunitParameter params[], void *data)
{
    micro_init();

    cg_ctx_t ctx;
    cg_gen(&ctx, "fun add\n"
                 "    i32 a\n"
                 "    i32 b\n"
                 "    ret i32\n"
                 "start\n"
                 "    ret a;\n"
                 "end\n"
                 "fun main\n"
                 "start\n"
                 "    call _ add 1;\n"
                 "end\n");

    munit_assert_size(micro_err_stk_size, ==, 1);
    munit_assert_int((int)micro_err_stk[0].err, ==, (int)MICRO_ERROR_TOO_FEW_ARGS);

    cg_cleanup(&ctx);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_err_vreg_type_mismatch(const MunitParameter params[], void *data)
{
    micro_init();

    cg_ctx_t ctx;
    cg_gen(&ctx, "fun f\n"
                 "start\n"
                 "    set i32 x 5;\n"
                 "    set u32 x 6;\n"
                 "end\n");

    munit_assert_size(micro_err_stk_size, ==, 1);
    munit_assert_int((int)micro_err_stk[0].err, ==, (int)MICRO_ERROR_VREG_TYPE_MISMATCH);

    cg_cleanup(&ctx);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_err_call_result_undef(const MunitParameter params[], void *data)
{
    micro_init();

    cg_ctx_t ctx;
    cg_gen(&ctx, "fun add\n"
                 "    ret i32\n"
                 "start\n"
                 "    ret 1;\n"
                 "end\n"
                 "fun f\n"
                 "start\n"
                 "    set i32 res;\n"
                 "    call nope add;\n"
                 "    ret;\n"
                 "end\n");

    munit_assert_size(micro_err_stk_size, ==, 1);
    munit_assert_int((int)micro_err_stk[0].err, ==, (int)MICRO_ERROR_UNDEFINED_IDENT);

    cg_cleanup(&ctx);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_err_call_result_type(const MunitParameter params[], void *data)
{
    micro_init();

    cg_ctx_t ctx;
    cg_gen(&ctx, "fun add\n"
                 "    ret i32\n"
                 "start\n"
                 "    ret 1;\n"
                 "end\n"
                 "fun f\n"
                 "start\n"
                 "    set u32 res;\n"
                 "    call res add;\n"
                 "    ret;\n"
                 "end\n");

    munit_assert_size(micro_err_stk_size, ==, 1);
    munit_assert_int((int)micro_err_stk[0].err, ==, (int)MICRO_ERROR_RESULT_TYPE_MISMATCH);

    cg_cleanup(&ctx);

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
    { "/if_vreg", test_codegen_if_vreg, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/if_not", test_codegen_if_not, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/if_eq", test_codegen_if_eq, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/if_great", test_codegen_if_great, NULL, NULL, MUNIT_TEST_OPTION_TODO, NULL },
    { "/if_less", test_codegen_if_less, NULL, NULL, MUNIT_TEST_OPTION_TODO, NULL },
    { "/if_great_or_eq", test_codegen_if_great_or_eq, NULL, NULL, MUNIT_TEST_OPTION_TODO, NULL },
    { "/if_less_or_eq", test_codegen_if_less_or_eq, NULL, NULL, MUNIT_TEST_OPTION_TODO, NULL },
    { "/err_if_outside_function", test_codegen_err_if_outside_function, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/err_if_undefined_ident", test_codegen_err_if_undefined_ident, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/err_if_undefined_label", test_codegen_err_if_undefined_label, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/err_if_not_lbl", test_codegen_err_if_not_lbl, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/err_goto_undefined", test_codegen_err_goto_undefined, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/err_goto_not_lbl", test_codegen_err_goto_not_lbl, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/err_goto_outside_scope", test_codegen_err_goto_outside_scope, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/err_goto_outside_function", test_codegen_err_goto_outside_function, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
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