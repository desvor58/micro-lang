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
#include <sys/mman.h>
#include <unistd.h>

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

    micro_codegen386_init(&ctx->cg, (micro_codegen_flags_t){}, &ctx->asm_instrs, &ctx->arena, NULL);
    ctx->cg.emit(&ctx->cg, &ctx->ig.instructions);

    test_put_errors("codegen");
}

static void cg_gen_instrs(cg_ctx_t *ctx, sct_vector_t *instrs, sct_hashmap_t *tramps)
{
    sct_vector_init(&ctx->toks, sizeof(mc_token_t));
    mc_instrgen_init(&ctx->ig, &ctx->toks);

    sct_vector_init(&ctx->asm_instrs, sizeof(micro_asm386_instruction_t));
    sct_vector_init(&ctx->outbuf, sizeof(u8));
    sct_arena_init(&ctx->arena);

    micro_codegen386_init(&ctx->cg, (micro_codegen_flags_t){}, &ctx->asm_instrs, &ctx->arena, tramps);
    ctx->cg.emit(&ctx->cg, instrs);

    test_put_errors("codegen");
}

static void cg_gen_tramps(cg_ctx_t *ctx, const char *text, sct_hashmap_t *tramps)
{
    sct_vector_init(&ctx->toks, sizeof(mc_token_t));
    mc_tokenize(text, strlen(text), &ctx->toks);

    mc_instrgen_init(&ctx->ig, &ctx->toks);
    mc_instrgen_gen(&ctx->ig);

    sct_vector_init(&ctx->asm_instrs, sizeof(micro_asm386_instruction_t));
    sct_vector_init(&ctx->outbuf, sizeof(u8));
    sct_arena_init(&ctx->arena);

    micro_codegen386_init(&ctx->cg, (micro_codegen_flags_t){}, &ctx->asm_instrs, &ctx->arena, tramps);
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

static i32 cg_run0(cg_ctx_t *ctx)
{
    micro_asm386_optimize(&ctx->asm_instrs);
    micro_asm386_emit(&ctx->asm_instrs, &ctx->outbuf);

    long page_size = sysconf(_SC_PAGESIZE);
    void *mem = mmap(NULL, page_size, PROT_READ | PROT_WRITE | PROT_EXEC,
                     MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    munit_assert_ptr_not_null(mem);
    munit_assert_true(mem != MAP_FAILED);
    memcpy(mem, ctx->outbuf.data, ctx->outbuf.size);

    i32 (*fn)(void) = (i32 (*)(void))mem;
    return fn();
}

static size_t cg_count_callee_save_pushes(cg_ctx_t *ctx)
{
    size_t count = 0;
    for (size_t i = 0; i < cg_asm_size(ctx); i++) {
        micro_asm386_instruction_t *instr = cg_asm(ctx, i);
        if (instr->opcode != MICRO_ASM386_INSTR_PUSH_R32) {
            continue;
        }
        if (instr->operand1.reg == MICRO_ASM386_REG32_EBX
         || instr->operand1.reg == MICRO_ASM386_REG32_ESI
         || instr->operand1.reg == MICRO_ASM386_REG32_EDI) {
            count++;
        }
    }
    return count;
}

MunitResult test_codegen_lifetime_keeps_vreg(const MunitParameter params[], void *data)
{
    micro_init();

    cg_ctx_t ctx;
    cg_gen(&ctx, "fun entry\n"
                 "    ret i32\n"
                 "start\n"
                 "    set i32 a 5 {lifetime:1};\n"
                 "    set i32 b a;\n"
                 "    ret b;\n"
                 "end\n");

    munit_assert_size(micro_err_stk_size, ==, 0);
    munit_assert_int(cg_run0(&ctx), ==, 5);

    cg_cleanup(&ctx);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_lifetime_counts_from_declaration(const MunitParameter params[], void *data)
{
    micro_init();

    cg_ctx_t ctx;
    cg_gen(&ctx, "fun entry\n"
                 "    ret i32\n"
                 "start\n"
                 "    set i32 p 1;\n"
                 "    set i32 q 2;\n"
                 "    set i32 a 5 {lifetime:2};\n"
                 "    set i32 b a;\n"
                 "    set i32 c a;\n"
                 "    ret b;\n"
                 "end\n");

    munit_assert_size(micro_err_stk_size, ==, 0);
    munit_assert_int(cg_run0(&ctx), ==, 5);

    cg_cleanup(&ctx);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_lifetime_counts_from_declaration_expired(const MunitParameter params[], void *data)
{
    micro_init();

    cg_ctx_t ctx;
    cg_gen(&ctx, "fun entry\n"
                 "    ret i32\n"
                 "start\n"
                 "    set i32 p 1;\n"
                 "    set i32 q 2;\n"
                 "    set i32 a 5 {lifetime:1};\n"
                 "    set i32 b a;\n"
                 "    set i32 c 3;\n"
                 "    set i32 d a;\n"
                 "    ret b;\n"
                 "end\n");

    munit_assert_size(micro_err_stk_size, ==, 1);
    munit_assert_int((int)micro_err_stk[0].err, ==, (int)MICRO_ERROR_UNDEFINED_IDENT);
    munit_assert_int((int)micro_err_stk[0].instr, ==, (int)MICRO_INSTR_SET);

    cg_cleanup(&ctx);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_lifetime_expires(const MunitParameter params[], void *data)
{
    micro_init();

    cg_ctx_t ctx;
    cg_gen(&ctx, "fun entry\n"
                 "    ret i32\n"
                 "start\n"
                 "    set i32 a 5 {lifetime:1};\n"
                 "    set i32 b 1;\n"
                 "    set i32 c a;\n"
                 "    ret c;\n"
                 "end\n");

    munit_assert_size(micro_err_stk_size, ==, 1);
    munit_assert_int((int)micro_err_stk[0].err, ==, (int)MICRO_ERROR_UNDEFINED_IDENT);
    munit_assert_int((int)micro_err_stk[0].instr, ==, (int)MICRO_INSTR_SET);

    cg_cleanup(&ctx);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_lifetime_zero_expires_immediately(const MunitParameter params[], void *data)
{
    micro_init();

    cg_ctx_t ctx;
    cg_gen(&ctx, "fun entry\n"
                 "    ret i32\n"
                 "start\n"
                 "    set i32 a 5 {lifetime:0};\n"
                 "    set i32 b a;\n"
                 "    ret b;\n"
                 "end\n");

    munit_assert_size(micro_err_stk_size, ==, 1);
    munit_assert_int((int)micro_err_stk[0].err, ==, (int)MICRO_ERROR_UNDEFINED_IDENT);

    cg_cleanup(&ctx);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_lifetime_frees_name(const MunitParameter params[], void *data)
{
    micro_init();

    cg_ctx_t ctx;
    cg_gen(&ctx, "fun entry\n"
                 "    ret i32\n"
                 "start\n"
                 "    set i32 a 5 {lifetime:1};\n"
                 "    set i32 b 1;\n"
                 "    set i32 a 9;\n"
                 "    ret a;\n"
                 "end\n");

    munit_assert_size(micro_err_stk_size, ==, 0);
    munit_assert_int(cg_run0(&ctx), ==, 9);

    cg_cleanup(&ctx);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_no_lifetime_lives_to_end(const MunitParameter params[], void *data)
{
    micro_init();

    cg_ctx_t ctx;
    cg_gen(&ctx, "fun entry\n"
                 "    ret i32\n"
                 "start\n"
                 "    set i32 a 5;\n"
                 "    set i32 b 1;\n"
                 "    set i32 c 2;\n"
                 "    set i32 d 3;\n"
                 "    ret a;\n"
                 "end\n");

    munit_assert_size(micro_err_stk_size, ==, 0);
    munit_assert_int(cg_run0(&ctx), ==, 5);

    cg_cleanup(&ctx);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_lifetime_frees_registers(const MunitParameter params[], void *data)
{
    micro_init();

    cg_ctx_t plain;
    cg_gen(&plain, "fun entry\n"
                   "    ret i32\n"
                   "start\n"
                   "    set i32 a 1;\n"
                   "    set i32 b 2;\n"
                   "    set i32 c 3;\n"
                   "    set i32 d 4;\n"
                   "    set i32 e 5;\n"
                   "    set i32 f 6;\n"
                   "    set i32 g 7;\n"
                   "    set i32 h 8;\n"
                   "    set i32 i 9;\n"
                   "    ret + a b;\n"
                   "end\n");
    munit_assert_size(micro_err_stk_size, ==, 0);

    cg_ctx_t hinted;
    cg_gen(&hinted, "fun entry\n"
                   "    ret i32\n"
                   "start\n"
                   "    set i32 a 1 {lifetime:0};\n"
                   "    set i32 b 2 {lifetime:0};\n"
                   "    set i32 c 3 {lifetime:0};\n"
                   "    set i32 d 4 {lifetime:0};\n"
                   "    set i32 e 5 {lifetime:0};\n"
                   "    set i32 f 6 {lifetime:0};\n"
                   "    set i32 g 7 {lifetime:0};\n"
                   "    set i32 h 8 {lifetime:0};\n"
                   "    set i32 i 9 {lifetime:0};\n"
                   "    ret 42;\n"
                   "end\n");
    munit_assert_size(micro_err_stk_size, ==, 0);

    /* nine live virtual registers do not fit into the machine registers, */
    /* with the hint every one of them reuses the same one */
    munit_assert_size(cg_count_callee_save_pushes(&plain), ==, 3);
    munit_assert_size(cg_count_callee_save_pushes(&hinted), ==, 0);
    munit_assert_size(cg_asm_size(&hinted), <, cg_asm_size(&plain));

    cg_cleanup(&plain);
    cg_cleanup(&hinted);

    micro_deinit();

    return MUNIT_OK;
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

    cg_assert_asm_opcode(&ctx, 2, MICRO_ASM386_INSTR_SUB_R32I32);
    cg_assert_asm_opcode(&ctx, 3, MICRO_ASM386_INSTR_PUSH_R32);
    cg_assert_asm_opcode(&ctx, 4, MICRO_ASM386_INSTR_PUSH_R32);
    cg_assert_asm_opcode(&ctx, 5, MICRO_ASM386_INSTR_PUSH_R32);
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

MunitResult test_codegen_call_spills_results(const MunitParameter params[], void *data)
{
    micro_init();

    /* Recursion test: two bare sets, then a call for each, then `ret + f1 f2`.
     * Registers holding f1/f2 must be spilled to the stack frame before the
     * calls (the callee clobbers eax/ecx/edx) and each result must be written
     * back to its own stack slot, otherwise `ret` reads garbage. */
    cg_ctx_t ctx;
    cg_gen(&ctx, "fun fib\n"
                 "    i32 n\n"
                 "    ret i32\n"
                 "start\n"
                 "    if <= n 1 : end_rec;\n"
                 "    set i32 f1;\n"
                 "    set i32 f2;\n"
                 "    call f1 fib - n 1;\n"
                 "    call f2 fib - n 2;\n"
                 "    ret + f1 f2;\n"
                 "end_rec:\n"
                 "    ret n;\n"
                 "end\n");

    munit_assert_size(micro_err_stk_size, ==, 0);

    // spill f1 (eax) and f2 (ecx) to [ebp-4] and [ebp-8]
    cg_assert_asm_opcode(&ctx, 6, MICRO_ASM386_INSTR_MOV_S32R32);
    cg_assert_asm_imm(&ctx, 6, 1, -4);
    cg_assert_asm_reg(&ctx, 6, 2, MICRO_ASM386_REG32_EAX);

    cg_assert_asm_opcode(&ctx, 7, MICRO_ASM386_INSTR_MOV_S32R32);
    cg_assert_asm_imm(&ctx, 7, 1, -8);
    cg_assert_asm_reg(&ctx, 7, 2, MICRO_ASM386_REG32_ECX);

    // first call result stored to f1's slot
    cg_assert_asm_opcode(&ctx, 13, MICRO_ASM386_INSTR_MOV_S32R32);
    cg_assert_asm_imm(&ctx, 13, 1, -4);
    cg_assert_asm_reg(&ctx, 13, 2, MICRO_ASM386_REG32_EAX);

    // second call result stored to f2's slot
    cg_assert_asm_opcode(&ctx, 19, MICRO_ASM386_INSTR_MOV_S32R32);
    cg_assert_asm_imm(&ctx, 19, 1, -8);
    cg_assert_asm_reg(&ctx, 19, 2, MICRO_ASM386_REG32_EAX);

    // ret + f1 f2: eax = [ebp-4] ; add eax, [ebp-8]
    cg_assert_asm_opcode(&ctx, 20, MICRO_ASM386_INSTR_MOV_R32S32);
    cg_assert_asm_reg(&ctx, 20, 1, MICRO_ASM386_REG32_EAX);
    cg_assert_asm_imm(&ctx, 20, 2, -4);

    cg_assert_asm_opcode(&ctx, 21, MICRO_ASM386_INSTR_ADD_R32S32);
    cg_assert_asm_reg(&ctx, 21, 1, MICRO_ASM386_REG32_EAX);
    cg_assert_asm_imm(&ctx, 21, 2, -8);

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

    // `if = n 1` is lowered as CMP n,1 + a conditional jump on the live
    // flags: jump to target when equal (JZ). (No SETz/TEST roundtrip.)
    int found_jz = 0;
    for (size_t i = 0; i < cg_asm_size(&ctx); i++) {
        micro_asm386_instruction_t *instr = cg_asm(&ctx, i);
        if (instr && instr->opcode == MICRO_ASM386_INSTR_JZ_L32 &&
            !strcmp(instr->operand1.lbl_name, "f.target")) {
            found_jz = 1;
            break;
        }
    }
    munit_assert_int(found_jz, ==, 1);

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

static void cg_assert_asm_sib(cg_ctx_t *ctx, size_t i, int scale, micro_asm386_reg_t index, micro_asm386_reg_t base)
{
    micro_asm386_instruction_t *instr = cg_asm(ctx, i);
    munit_assert_ptr_not_null(instr);
    munit_assert_int(1 << instr->sib.scale, ==, scale);
    munit_assert_int((int)instr->sib.index, ==, (int)index);
    munit_assert_int((int)instr->sib.base, ==, (int)base);
}

static void cg_assert_no_add_imm(cg_ctx_t *ctx, int imm)
{
    for (size_t i = 0; i < cg_asm_size(ctx); i++) {
        micro_asm386_instruction_t *instr = cg_asm(ctx, i);
        if (instr->opcode == MICRO_ASM386_INSTR_ADD_R32I32) {
            munit_assert_int(instr->operand2.imm.val, !=, imm);
        }
    }
}

static void cg_assert_no_lea(cg_ctx_t *ctx)
{
    for (size_t i = 0; i < cg_asm_size(ctx); i++) {
        munit_assert_int((int)cg_asm(ctx, i)->opcode, !=, (int)MICRO_ASM386_INSTR_LEA_R32SIB);
        munit_assert_int((int)cg_asm(ctx, i)->opcode, !=, (int)MICRO_ASM386_INSTR_LEA_R32SIBI8);
        munit_assert_int((int)cg_asm(ctx, i)->opcode, !=, (int)MICRO_ASM386_INSTR_LEA_R32SIBI32);
        munit_assert_int((int)cg_asm(ctx, i)->opcode, !=, (int)MICRO_ASM386_INSTR_LEA_R32SIBABS);
    }
}

// a and b are allocated to eax/ecx, c to edx, the lea is emitted before the ret
#define CG_LEA_PROLOG "fun f\n" \
                        "start\n" \
                        "    set u32 a 4;\n" \
                        "    set u32 b 8;\n"

MunitResult test_codegen_lea_mul_scale(const MunitParameter params[], void *data)
{
    micro_init();

    cg_ctx_t ctx;
    cg_gen(&ctx, CG_LEA_PROLOG
                 "    set u32 c * a 4;\n"
                 "    ret c;\n"
                 "end\n");

    munit_assert_size(micro_err_stk_size, ==, 0);

    cg_assert_asm_opcode(&ctx, 9, MICRO_ASM386_INSTR_LEA_R32SIBABS);
    cg_assert_asm_reg(&ctx, 9, 1, MICRO_ASM386_REG32_EDX);
    cg_assert_asm_sib(&ctx, 9, 4, MICRO_ASM386_REG32_EAX, MICRO_ASM386_REG32_NO_BASE);

    cg_cleanup(&ctx);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_lea_mul_scale_lit_first(const MunitParameter params[], void *data)
{
    micro_init();

    cg_ctx_t ctx;
    cg_gen(&ctx, CG_LEA_PROLOG
                 "    set u32 c * 4 a;\n"
                 "    ret c;\n"
                 "end\n");

    munit_assert_size(micro_err_stk_size, ==, 0);

    cg_assert_asm_opcode(&ctx, 9, MICRO_ASM386_INSTR_LEA_R32SIBABS);
    cg_assert_asm_reg(&ctx, 9, 1, MICRO_ASM386_REG32_EDX);
    cg_assert_asm_sib(&ctx, 9, 4, MICRO_ASM386_REG32_EAX, MICRO_ASM386_REG32_NO_BASE);

    cg_cleanup(&ctx);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_lea_plus_index_scale(const MunitParameter params[], void *data)
{
    micro_init();

    cg_ctx_t ctx;
    cg_gen(&ctx, CG_LEA_PROLOG
                 "    set u32 c + a * b 4;\n"
                 "    ret c;\n"
                 "end\n");

    munit_assert_size(micro_err_stk_size, ==, 0);

    cg_assert_asm_opcode(&ctx, 9, MICRO_ASM386_INSTR_LEA_R32SIB);
    cg_assert_asm_reg(&ctx, 9, 1, MICRO_ASM386_REG32_EDX);
    cg_assert_asm_sib(&ctx, 9, 4, MICRO_ASM386_REG32_ECX, MICRO_ASM386_REG32_EAX);

    cg_cleanup(&ctx);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_lea_sum_disp8(const MunitParameter params[], void *data)
{
    micro_init();

    cg_ctx_t ctx;
    cg_gen(&ctx, CG_LEA_PROLOG
                 "    set u32 c + + a * b 4 12;\n"
                 "    ret c;\n"
                 "end\n");

    munit_assert_size(micro_err_stk_size, ==, 0);

    cg_assert_asm_opcode(&ctx, 9, MICRO_ASM386_INSTR_LEA_R32SIBI8);
    cg_assert_asm_sib(&ctx, 9, 4, MICRO_ASM386_REG32_ECX, MICRO_ASM386_REG32_EAX);
    cg_assert_asm_imm(&ctx, 9, 2, 12);

    cg_cleanup(&ctx);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_lea_sum_disp8_neg(const MunitParameter params[], void *data)
{
    micro_init();

    cg_ctx_t ctx;
    cg_gen(&ctx, CG_LEA_PROLOG
                 "    set u32 c + + a * b 4 -12;\n"
                 "    ret c;\n"
                 "end\n");

    munit_assert_size(micro_err_stk_size, ==, 0);

    cg_assert_asm_opcode(&ctx, 9, MICRO_ASM386_INSTR_LEA_R32SIBI8);
    cg_assert_asm_imm(&ctx, 9, 2, -12);

    cg_cleanup(&ctx);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_lea_sum_disp32(const MunitParameter params[], void *data)
{
    micro_init();

    cg_ctx_t ctx;
    cg_gen(&ctx, CG_LEA_PROLOG
                 "    set u32 c + + a * b 4 4096;\n"
                 "    ret c;\n"
                 "end\n");

    munit_assert_size(micro_err_stk_size, ==, 0);

    cg_assert_asm_opcode(&ctx, 9, MICRO_ASM386_INSTR_LEA_R32SIBI32);
    cg_assert_asm_sib(&ctx, 9, 4, MICRO_ASM386_REG32_ECX, MICRO_ASM386_REG32_EAX);
    cg_assert_asm_imm(&ctx, 9, 2, 4096);

    cg_cleanup(&ctx);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_lea_scaled_addend(const MunitParameter params[], void *data)
{
    micro_init();

    cg_ctx_t ctx;
    cg_gen(&ctx, CG_LEA_PROLOG
                 "    set u32 c + * a 4 12;\n"
                 "    ret c;\n"
                 "end\n");

    munit_assert_size(micro_err_stk_size, ==, 0);

    cg_assert_asm_opcode(&ctx, 9, MICRO_ASM386_INSTR_LEA_R32SIBABS);
    cg_assert_asm_sib(&ctx, 9, 4, MICRO_ASM386_REG32_EAX, MICRO_ASM386_REG32_NO_BASE);
    cg_assert_asm_imm(&ctx, 9, 2, 12);

    cg_cleanup(&ctx);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_lea_scaled_plus_base(const MunitParameter params[], void *data)
{
    micro_init();

    cg_ctx_t ctx;
    cg_gen(&ctx, CG_LEA_PROLOG
                 "    set u32 c + * a 4 b;\n"
                 "    ret c;\n"
                 "end\n");

    munit_assert_size(micro_err_stk_size, ==, 0);

    cg_assert_asm_opcode(&ctx, 9, MICRO_ASM386_INSTR_LEA_R32SIB);
    cg_assert_asm_sib(&ctx, 9, 4, MICRO_ASM386_REG32_EAX, MICRO_ASM386_REG32_ECX);

    cg_cleanup(&ctx);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_lea_lit_plus_scaled(const MunitParameter params[], void *data)
{
    micro_init();

    cg_ctx_t ctx;
    cg_gen(&ctx, CG_LEA_PROLOG
                 "    set u32 c + 12 * a 4;\n"
                 "    ret c;\n"
                 "end\n");

    munit_assert_size(micro_err_stk_size, ==, 0);

    cg_assert_asm_opcode(&ctx, 9, MICRO_ASM386_INSTR_LEA_R32SIBABS);
    cg_assert_asm_sib(&ctx, 9, 4, MICRO_ASM386_REG32_EAX, MICRO_ASM386_REG32_NO_BASE);
    cg_assert_asm_imm(&ctx, 9, 2, 12);

    cg_cleanup(&ctx);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_lea_index_offset(const MunitParameter params[], void *data)
{
    micro_init();

    cg_ctx_t ctx;
    cg_gen(&ctx, CG_LEA_PROLOG
                 "    set u32 c + a * + b 1 4;\n"
                 "    ret c;\n"
                 "end\n");

    munit_assert_size(micro_err_stk_size, ==, 0);

    cg_assert_asm_opcode(&ctx, 9, MICRO_ASM386_INSTR_LEA_R32SIBI8);
    cg_assert_asm_reg(&ctx, 9, 1, MICRO_ASM386_REG32_EDX);
    cg_assert_asm_sib(&ctx, 9, 4, MICRO_ASM386_REG32_ECX, MICRO_ASM386_REG32_EAX);
    cg_assert_asm_imm(&ctx, 9, 2, 4);

    cg_cleanup(&ctx);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_lea_index_offset_mul(const MunitParameter params[], void *data)
{
    micro_init();

    cg_ctx_t ctx;
    cg_gen(&ctx, CG_LEA_PROLOG
                 "    set u32 c * + a 1 4;\n"
                 "    ret c;\n"
                 "end\n");

    munit_assert_size(micro_err_stk_size, ==, 0);

    cg_assert_asm_opcode(&ctx, 9, MICRO_ASM386_INSTR_LEA_R32SIBABS);
    cg_assert_asm_sib(&ctx, 9, 4, MICRO_ASM386_REG32_EAX, MICRO_ASM386_REG32_NO_BASE);
    cg_assert_asm_imm(&ctx, 9, 2, 4);

    cg_cleanup(&ctx);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_lea_index_offset_lit_first(const MunitParameter params[], void *data)
{
    micro_init();

    cg_ctx_t ctx;
    cg_gen(&ctx, CG_LEA_PROLOG
                 "    set u32 c * + 8 a 4;\n"
                 "    ret c;\n"
                 "end\n");

    munit_assert_size(micro_err_stk_size, ==, 0);

    cg_assert_asm_opcode(&ctx, 9, MICRO_ASM386_INSTR_LEA_R32SIBABS);
    cg_assert_asm_sib(&ctx, 9, 4, MICRO_ASM386_REG32_EAX, MICRO_ASM386_REG32_NO_BASE);
    cg_assert_asm_imm(&ctx, 9, 2, 32);

    cg_cleanup(&ctx);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_lea_index_offset_addend(const MunitParameter params[], void *data)
{
    micro_init();

    cg_ctx_t ctx;
    cg_gen(&ctx, CG_LEA_PROLOG
                 "    set u32 c + * + a 1 4 b;\n"
                 "    ret c;\n"
                 "end\n");

    munit_assert_size(micro_err_stk_size, ==, 0);

    cg_assert_asm_opcode(&ctx, 9, MICRO_ASM386_INSTR_LEA_R32SIBI8);
    cg_assert_asm_sib(&ctx, 9, 4, MICRO_ASM386_REG32_EAX, MICRO_ASM386_REG32_ECX);
    cg_assert_asm_imm(&ctx, 9, 2, 4);

    cg_cleanup(&ctx);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_no_lea_for_non_scale(const MunitParameter params[], void *data)
{
    micro_init();

    cg_ctx_t ctx;
    cg_gen(&ctx, CG_LEA_PROLOG
                 "    set u32 c + a * b 3;\n"
                 "    ret c;\n"
                 "end\n");

    munit_assert_size(micro_err_stk_size, ==, 0);

    cg_assert_no_lea(&ctx);
    cg_assert_asm_opcode(&ctx, 13, MICRO_ASM386_INSTR_IMUL_R32I32);
    cg_assert_asm_imm(&ctx, 13, 2, 3);

    cg_cleanup(&ctx);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_no_lea_for_non_scale_mul(const MunitParameter params[], void *data)
{
    micro_init();

    cg_ctx_t ctx;
    cg_gen(&ctx, CG_LEA_PROLOG
                 "    set u32 c * a 6;\n"
                 "    ret c;\n"
                 "end\n");

    munit_assert_size(micro_err_stk_size, ==, 0);

    cg_assert_no_lea(&ctx);

    cg_cleanup(&ctx);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_no_lea_for_stack_vreg(const MunitParameter params[], void *data)
{
    micro_init();

    cg_ctx_t ctx;
    cg_gen(&ctx, "fun f\n"
                 "    i32 arg\n"
                 "    ret i32\n"
                 "start\n"
                 "    set i32 a 4;\n"
                 "    set i32 c + a * arg 4;\n"
                 "    ret c;\n"
                 "end\n");

    munit_assert_size(micro_err_stk_size, ==, 0);

    cg_assert_no_lea(&ctx);

    cg_cleanup(&ctx);

    micro_deinit();

    return MUNIT_OK;
}

#undef CG_LEA_PROLOG


static int cg_find_asm_opcode(cg_ctx_t *ctx, micro_asm386_instruction_type_t opcode)
{
    for (size_t i = 0; i < cg_asm_size(ctx); i++) {
        if (cg_asm(ctx, i)->opcode == opcode) {
            return (int)i;
        }
    }
    return -1;
}

MunitResult test_codegen_add_one_to_inc(const MunitParameter params[], void *data)
{
    micro_init();

    cg_ctx_t ctx;
    cg_gen(&ctx, "fun f\n"
                 "start\n"
                 "    set i32 a 4;\n"
                 "    set i32 b 5;\n"
                 "    set i32 c + a 1;\n"
                 "    ret c;\n"
                 "end\n");

    munit_assert_size(micro_err_stk_size, ==, 0);

    munit_assert_true(cg_find_asm_opcode(&ctx, MICRO_ASM386_INSTR_ADD_R32I32) >= 0);
    munit_assert_true(cg_find_asm_opcode(&ctx, MICRO_ASM386_INSTR_INC_R32) < 0);

    micro_asm386_optimize(&ctx.asm_instrs);

    int inc = cg_find_asm_opcode(&ctx, MICRO_ASM386_INSTR_INC_R32);
    munit_assert_true(inc >= 0);
    cg_assert_asm_reg(&ctx, (size_t)inc, 1, MICRO_ASM386_REG32_EDX);
    munit_assert_true(cg_find_asm_opcode(&ctx, MICRO_ASM386_INSTR_ADD_R32I32) < 0);
    munit_assert_true(cg_find_asm_opcode(&ctx, MICRO_ASM386_INSTR_DEC_R32) < 0);

    cg_cleanup(&ctx);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_sub_one_to_dec(const MunitParameter params[], void *data)
{
    micro_init();

    cg_ctx_t ctx;
    cg_gen(&ctx, "fun f\n"
                 "start\n"
                 "    set i32 a 4;\n"
                 "    set i32 b 5;\n"
                 "    set i32 c - b 1;\n"
                 "    ret c;\n"
                 "end\n");

    munit_assert_size(micro_err_stk_size, ==, 0);

    munit_assert_true(cg_find_asm_opcode(&ctx, MICRO_ASM386_INSTR_SUB_R32I32) >= 0);
    munit_assert_true(cg_find_asm_opcode(&ctx, MICRO_ASM386_INSTR_DEC_R32) < 0);

    micro_asm386_optimize(&ctx.asm_instrs);

    int dec = cg_find_asm_opcode(&ctx, MICRO_ASM386_INSTR_DEC_R32);
    munit_assert_true(dec >= 0);
    cg_assert_asm_reg(&ctx, (size_t)dec, 1, MICRO_ASM386_REG32_EDX);
    munit_assert_true(cg_find_asm_opcode(&ctx, MICRO_ASM386_INSTR_SUB_R32I32) < 0);
    munit_assert_true(cg_find_asm_opcode(&ctx, MICRO_ASM386_INSTR_INC_R32) < 0);

    cg_cleanup(&ctx);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_add_zero_stays_gone(const MunitParameter params[], void *data)
{
    micro_init();

    cg_ctx_t ctx;
    cg_gen(&ctx, "fun f\n"
                 "start\n"
                 "    set i32 a 4;\n"
                 "    set i32 c + a 0;\n"
                 "    ret c;\n"
                 "end\n");

    munit_assert_size(micro_err_stk_size, ==, 0);

    micro_asm386_optimize(&ctx.asm_instrs);

    munit_assert_true(cg_find_asm_opcode(&ctx, MICRO_ASM386_INSTR_ADD_R32I32) < 0);
    munit_assert_true(cg_find_asm_opcode(&ctx, MICRO_ASM386_INSTR_INC_R32) < 0);

    cg_cleanup(&ctx);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_add_lit_to_lea(const MunitParameter params[], void *data)
{
    micro_init();

    cg_ctx_t ctx;
    cg_gen(&ctx, "fun f\n"
                 "start\n"
                 "    set i32 a 4;\n"
                 "    set i32 c + a 2;\n"
                 "    ret c;\n"
                 "end\n");

    munit_assert_size(micro_err_stk_size, ==, 0);

    int lea = cg_find_asm_opcode(&ctx, MICRO_ASM386_INSTR_LEA_R32SIBI8);
    munit_assert_true(lea >= 0);
    cg_assert_asm_reg(&ctx, (size_t)lea, 1, MICRO_ASM386_REG32_ECX);
    cg_assert_asm_sib(&ctx, (size_t)lea, 1, MICRO_ASM386_REG32_NO_INDEX, MICRO_ASM386_REG32_EAX);
    cg_assert_asm_imm(&ctx, (size_t)lea, 2, 2);
    cg_assert_no_add_imm(&ctx, 2);

    cg_cleanup(&ctx);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_add_lit_first_to_lea(const MunitParameter params[], void *data)
{
    micro_init();

    cg_ctx_t ctx;
    cg_gen(&ctx, "fun f\n"
                 "start\n"
                 "    set i32 a 4;\n"
                 "    set i32 c + 2 a;\n"
                 "    ret c;\n"
                 "end\n");

    munit_assert_size(micro_err_stk_size, ==, 0);

    int lea = cg_find_asm_opcode(&ctx, MICRO_ASM386_INSTR_LEA_R32SIBI8);
    munit_assert_true(lea >= 0);
    cg_assert_asm_sib(&ctx, (size_t)lea, 1, MICRO_ASM386_REG32_NO_INDEX, MICRO_ASM386_REG32_EAX);
    cg_assert_asm_imm(&ctx, (size_t)lea, 2, 2);
    cg_assert_no_add_imm(&ctx, 2);

    cg_cleanup(&ctx);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_add_lit_disp32_to_lea(const MunitParameter params[], void *data)
{
    micro_init();

    cg_ctx_t ctx;
    cg_gen(&ctx, "fun f\n"
                 "start\n"
                 "    set i32 a 4;\n"
                 "    set i32 c + a 300;\n"
                 "    ret c;\n"
                 "end\n");

    munit_assert_size(micro_err_stk_size, ==, 0);

    int lea = cg_find_asm_opcode(&ctx, MICRO_ASM386_INSTR_LEA_R32SIBI32);
    munit_assert_true(lea >= 0);
    cg_assert_asm_imm(&ctx, (size_t)lea, 2, 300);
    cg_assert_no_add_imm(&ctx, 300);

    cg_cleanup(&ctx);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_add_lit_stack_base_stays_add(const MunitParameter params[], void *data)
{
    micro_init();

    cg_ctx_t ctx;
    cg_gen(&ctx, "fun f\n"
                 "    i32 a\n"
                 "    ret i32\n"
                 "start\n"
                 "    set i32 c + a 2;\n"
                 "    ret c;\n"
                 "end\n");

    munit_assert_size(micro_err_stk_size, ==, 0);

    micro_asm386_optimize(&ctx.asm_instrs);

    cg_assert_no_lea(&ctx);
    int add = cg_find_asm_opcode(&ctx, MICRO_ASM386_INSTR_ADD_R32I32);
    munit_assert_true(add >= 0);
    cg_assert_asm_imm(&ctx, (size_t)add, 2, 2);

    cg_cleanup(&ctx);

    micro_deinit();

    return MUNIT_OK;
}

static i32 cg_tramp_seen_args[8];
static size_t cg_tramp_seen_args_num;
static micro_type_t cg_tramp_seen_ret_type;
static size_t cg_tramp_seen_calls;

static i32 cg_tramp_add(const micro_tramp_frame_t *frame)
{
    for (size_t i = 0; i < frame->args_num && i < 8; i++) {
        cg_tramp_seen_args[i] = frame->args[i];
    }
    cg_tramp_seen_args_num = frame->args_num;
    cg_tramp_seen_ret_type = frame->ret_type;
    cg_tramp_seen_calls++;
    return frame->args[0] + frame->args[1] * 10;
}

static i32 cg_tramp_sum(const micro_tramp_frame_t *frame)
{
    i32 sum = 0;
    for (size_t i = 0; i < frame->args_num; i++) {
        sum += frame->args[i];
    }
    cg_tramp_seen_args_num = frame->args_num;
    cg_tramp_seen_ret_type = frame->ret_type;
    cg_tramp_seen_calls++;
    return sum;
}

static void cg_tramp_push_arg(sct_vector_t *args, micro_type_t type, const char *name)
{
    micro_instruction_fun_arg_t arg = { .type = type };
    strncpy(arg.name, name, MICRO_MAX_SYMBOL_SIZE - 1);
    sct_vector_push(args, &arg);
}

static void cg_tramp_make_expr(sct_vector_t *out, const char *expr)
{
    sct_vector_init(out, sizeof(micro_expr_tok_t));
    micro_make_expr(out, expr);
}

static micro_expr_tok_t *cg_tramp_expr_ptr[8];

static void cg_tramp_push_expr(sct_vector_t *args, sct_vector_t *expr)
{
    for (size_t i = 0; i < sizeof(cg_tramp_expr_ptr) / sizeof(cg_tramp_expr_ptr[0]); i++) {
        if (cg_tramp_expr_ptr[i] == NULL) {
            cg_tramp_expr_ptr[i] = (micro_expr_tok_t *)expr->data;
            sct_vector_push(args, &cg_tramp_expr_ptr[i]);
            return;
        }
    }
}

static i32 cg_tramp_run(cg_ctx_t *ctx, i32 a, i32 b)
{
    micro_asm386_optimize(&ctx->asm_instrs);
    micro_asm386_emit(&ctx->asm_instrs, &ctx->outbuf);

    long page_size = sysconf(_SC_PAGESIZE);
    void *mem = mmap(NULL, page_size, PROT_READ | PROT_WRITE | PROT_EXEC,
                     MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    munit_assert_ptr_not_null(mem);
    munit_assert_true(mem != MAP_FAILED);
    memcpy(mem, ctx->outbuf.data, ctx->outbuf.size);

    i32 (*fn)(i32, i32) = (i32 (*)(i32, i32))mem;
    return fn(a, b);
}

static i32 cg_tramp_run_ptr(cg_ctx_t *ctx, i32 *slot, i32 n)
{
    micro_asm386_optimize(&ctx->asm_instrs);
    micro_asm386_emit(&ctx->asm_instrs, &ctx->outbuf);

    long page_size = sysconf(_SC_PAGESIZE);
    void *mem = mmap(NULL, page_size, PROT_READ | PROT_WRITE | PROT_EXEC,
                     MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    munit_assert_ptr_not_null(mem);
    munit_assert_true(mem != MAP_FAILED);
    memcpy(mem, ctx->outbuf.data, ctx->outbuf.size);

    i32 (*fn)(i32, i32) = (i32 (*)(i32, i32))mem;
    return fn((i32)(intptr_t)slot, n);
}

static void cg_tramp_build(sct_vector_t *instrs, sct_vector_t *tramp_args,
                           const char *tramp_name, micro_type_t ret_type,
                           size_t args_num, size_t call_args_num,
                           micro_type_t fun_ret_type, micro_type_t res_type)
{
    sct_vector_t fun_args;
    sct_vector_init(&fun_args, sizeof(micro_instruction_fun_arg_t));
    cg_tramp_push_arg(&fun_args, MICRO_TYPE_I32, "x");
    cg_tramp_push_arg(&fun_args, MICRO_TYPE_I32, "y");

    sct_vector_t exprs[2];
    cg_tramp_make_expr(&exprs[0], "x");
    cg_tramp_make_expr(&exprs[1], "y");

    sct_vector_t call_args;
    sct_vector_init(&call_args, sizeof(micro_expr_tok_t *));
    for (size_t i = 0; i < call_args_num; i++) {
        cg_tramp_push_expr(&call_args, &exprs[i % 2]);
    }

    sct_vector_t body;
    sct_vector_init(&body, sizeof(micro_instruction_t));

    micro_instruction_hints_t hints = { .lifetime = -1 };
    micro_instr_gen_tramp(instrs, tramp_name, tramp_args, ret_type, hints);
    micro_instr_gen_set(&body, res_type, "r", NULL, hints);
    micro_instr_gen_call(&body, "r", tramp_name, &call_args, hints);

    sct_vector_t ret_expr;
    cg_tramp_make_expr(&ret_expr, "r");
    micro_instr_gen_ret(&body, &ret_expr, hints);
    micro_instr_gen_fun(instrs, "f", &fun_args, fun_ret_type, &body, hints);
}

MunitResult test_codegen_tramp_source_call(const MunitParameter params[], void *data)
{
    micro_init();

    cg_tramp_seen_calls = 0;
    cg_tramp_seen_args_num = 99;
    cg_tramp_seen_ret_type = MICRO_TYPE_NULL;
    memset(cg_tramp_seen_args, 0, sizeof(cg_tramp_seen_args));

    sct_hashmap_t tramps;
    sct_hashmap_init(&tramps, sizeof(micro_tramp_t));
    micro_tramp_t handler = cg_tramp_add;
    sct_hashmap_add(&tramps, "vm_add", &handler);

    cg_ctx_t ctx;
    cg_gen_tramps(&ctx,
        "tramp vm_add\n"
        "    i32 a\n"
        "    i32 b\n"
        "    ret i32\n"
        "end\n"
        "\n"
        "fun call_add\n"
        "    i32 a\n"
        "    i32 b\n"
        "    ret i32\n"
        "start\n"
        "    set i32 res;\n"
        "    call res vm_add a b;\n"
        "    ret res;\n"
        "end\n", &tramps);
    munit_assert_size(micro_err_stk_size, ==, 0);

    munit_assert_int(cg_tramp_run(&ctx, 3, 4), ==, 43);
    munit_assert_int((int)cg_tramp_seen_calls, ==, 1);
    munit_assert_size(cg_tramp_seen_args_num, ==, 2);
    munit_assert_int((int)cg_tramp_seen_ret_type, ==, (int)MICRO_TYPE_I32);
    munit_assert_int(cg_tramp_seen_args[0], ==, 3);
    munit_assert_int(cg_tramp_seen_args[1], ==, 4);

    cg_cleanup(&ctx);
    sct_hashmap_deinit(&tramps);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_tramp_source_no_map(const MunitParameter params[], void *data)
{
    micro_init();

    cg_ctx_t ctx;
    cg_gen_tramps(&ctx,
        "tramp out_fun\n"
        "    ret i32\n"
        "end\n"
        "\n"
        "fun test\n"
        "    ret i32\n"
        "start\n"
        "    set i32 res;\n"
        "    call res out_fun;\n"
        "    ret res;\n"
        "end\n", NULL);

    munit_assert_size(micro_err_stk_size, >=, 1);
    munit_assert_int((int)micro_err_stk[0].err, ==, (int)MICRO_ERROR_UNDEFINED_TRAMP);
    munit_assert_int((int)micro_err_stk[0].instr, ==, (int)MICRO_INSTR_TRAMP);

    cg_cleanup(&ctx);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_tramp_fun_name_collision(const MunitParameter params[], void *data)
{
    micro_init();

    sct_hashmap_t tramps;
    sct_hashmap_init(&tramps, sizeof(micro_tramp_t));
    micro_tramp_t handler = cg_tramp_add;
    sct_hashmap_add(&tramps, "shared", &handler);

    cg_ctx_t ctx;
    cg_gen_tramps(&ctx,
        "fun shared\n"
        "    ret i32\n"
        "start\n"
        "    ret 0;\n"
        "end\n"
        "\n"
        "tramp shared\n"
        "    ret i32\n"
        "end\n", &tramps);

    munit_assert_size(micro_err_stk_size, ==, 1);
    munit_assert_int((int)micro_err_stk[0].err, ==, (int)MICRO_ERROR_TRAMP_REDEFINED);
    munit_assert_int((int)micro_err_stk[0].instr, ==, (int)MICRO_INSTR_TRAMP);

    cg_cleanup(&ctx);
    sct_hashmap_deinit(&tramps);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_tramp_after_fun(const MunitParameter params[], void *data)
{
    micro_init();

    sct_hashmap_t tramps;
    sct_hashmap_init(&tramps, sizeof(micro_tramp_t));
    micro_tramp_t handler = cg_tramp_add;
    sct_hashmap_add(&tramps, "vm_add", &handler);

    cg_ctx_t ctx;
    cg_gen_tramps(&ctx,
        "fun first\n"
        "    i32 a\n"
        "    ret i32\n"
        "start\n"
        "    ret a;\n"
        "end\n"
        "\n"
        "tramp vm_add\n"
        "    i32 a\n"
        "    i32 b\n"
        "    ret i32\n"
        "end\n", &tramps);
    munit_assert_size(micro_err_stk_size, ==, 0);

    munit_assert_int(cg_tramp_run(&ctx, 7, 0), ==, 7);

    cg_cleanup(&ctx);
    sct_hashmap_deinit(&tramps);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_tramp_inside_fun(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t tramp_args;
    sct_vector_init(&tramp_args, sizeof(micro_instruction_fun_arg_t));
    cg_tramp_push_arg(&tramp_args, MICRO_TYPE_I32, "a");

    micro_instruction_hints_t hints = { .lifetime = -1 };

    sct_vector_t body;
    sct_vector_init(&body, sizeof(micro_instruction_t));
    micro_instr_gen_tramp(&body, "vm_add", &tramp_args, MICRO_TYPE_I32, hints);

    sct_vector_t fun_args;
    sct_vector_init(&fun_args, sizeof(micro_instruction_fun_arg_t));
    cg_tramp_push_arg(&fun_args, MICRO_TYPE_I32, "x");

    sct_vector_t ret_expr;
    cg_tramp_make_expr(&ret_expr, "x");

    sct_vector_t body_ret;
    sct_vector_init(&body_ret, sizeof(micro_instruction_t));
    micro_instr_gen_ret(&body_ret, &ret_expr, hints);

    sct_vector_push(&body, sct_vector_get(&body_ret, 0));

    sct_vector_t instrs;
    sct_vector_init(&instrs, sizeof(micro_instruction_t));
    micro_instr_gen_fun(&instrs, "f", &fun_args, MICRO_TYPE_I32, &body, hints);

    sct_hashmap_t tramps;
    sct_hashmap_init(&tramps, sizeof(micro_tramp_t));
    micro_tramp_t handler = cg_tramp_add;
    sct_hashmap_add(&tramps, "vm_add", &handler);

    cg_ctx_t ctx;
    cg_gen_instrs(&ctx, &instrs, &tramps);

    munit_assert_size(micro_err_stk_size, ==, 1);
    munit_assert_int((int)micro_err_stk[0].err, ==, (int)MICRO_ERROR_TRAMP_INSIDE_FUNCTION);
    munit_assert_int((int)micro_err_stk[0].instr, ==, (int)MICRO_INSTR_TRAMP);

    cg_cleanup(&ctx);
    sct_hashmap_deinit(&tramps);
    sct_vector_deinit(&instrs);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_tramp_call(const MunitParameter params[], void *data)
{
    micro_init();

    cg_tramp_seen_calls = 0;
    cg_tramp_seen_args_num = 99;
    cg_tramp_seen_ret_type = MICRO_TYPE_NULL;
    memset(cg_tramp_seen_args, 0, sizeof(cg_tramp_seen_args));

    sct_vector_t tramp_args;
    sct_vector_init(&tramp_args, sizeof(micro_instruction_fun_arg_t));
    cg_tramp_push_arg(&tramp_args, MICRO_TYPE_I32, "a");
    cg_tramp_push_arg(&tramp_args, MICRO_TYPE_I32, "b");

    sct_vector_t instrs;
    sct_vector_init(&instrs, sizeof(micro_instruction_t));
    cg_tramp_build(&instrs, &tramp_args, "vm_add", MICRO_TYPE_I32, 2, 2, MICRO_TYPE_I32, MICRO_TYPE_I32);

    sct_hashmap_t tramps;
    sct_hashmap_init(&tramps, sizeof(micro_tramp_t));
    micro_tramp_t handler = cg_tramp_add;
    sct_hashmap_add(&tramps, "vm_add", &handler);

    cg_ctx_t ctx;
    cg_gen_instrs(&ctx, &instrs, &tramps);
    munit_assert_size(micro_err_stk_size, ==, 0);

    munit_assert_int(cg_tramp_run(&ctx, 3, 4), ==, 43);
    munit_assert_int((int)cg_tramp_seen_calls, ==, 1);
    munit_assert_size(cg_tramp_seen_args_num, ==, 2);
    munit_assert_int((int)cg_tramp_seen_ret_type, ==, (int)MICRO_TYPE_I32);
    munit_assert_int(cg_tramp_seen_args[0], ==, 3);
    munit_assert_int(cg_tramp_seen_args[1], ==, 4);

    cg_cleanup(&ctx);
    sct_hashmap_deinit(&tramps);
    sct_vector_deinit(&instrs);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_tramp_call_many_args(const MunitParameter params[], void *data)
{
    micro_init();

    cg_tramp_seen_calls = 0;

    sct_vector_t tramp_args;
    sct_vector_init(&tramp_args, sizeof(micro_instruction_fun_arg_t));
    for (int i = 0; i < 6; i++) {
        cg_tramp_push_arg(&tramp_args, MICRO_TYPE_I32, "a");
    }

    sct_vector_t instrs;
    sct_vector_init(&instrs, sizeof(micro_instruction_t));
    cg_tramp_build(&instrs, &tramp_args, "vm_sum", MICRO_TYPE_I32, 6, 6, MICRO_TYPE_I32, MICRO_TYPE_I32);

    sct_hashmap_t tramps;
    sct_hashmap_init(&tramps, sizeof(micro_tramp_t));
    micro_tramp_t handler = cg_tramp_sum;
    sct_hashmap_add(&tramps, "vm_sum", &handler);

    cg_ctx_t ctx;
    cg_gen_instrs(&ctx, &instrs, &tramps);
    munit_assert_size(micro_err_stk_size, ==, 0);

    munit_assert_int(cg_tramp_run(&ctx, 10, 20), ==, 90);
    munit_assert_size(cg_tramp_seen_args_num, ==, 6);
    munit_assert_int((int)cg_tramp_seen_calls, ==, 1);

    cg_cleanup(&ctx);
    sct_hashmap_deinit(&tramps);
    sct_vector_deinit(&instrs);

    micro_deinit();

    return MUNIT_OK;
}

static i32 cg_tramp_seen_ptr_value;

static i32 cg_tramp_none(const micro_tramp_frame_t *frame)
{
    cg_tramp_seen_args_num = frame->args_num;
    cg_tramp_seen_ret_type = frame->ret_type;
    cg_tramp_seen_calls++;
    return 77;
}

static i32 cg_tramp_ptr(const micro_tramp_frame_t *frame)
{
    i32 *slot = (i32 *)(intptr_t)frame->args[0];
    cg_tramp_seen_ptr_value = *slot;
    cg_tramp_seen_args_num = frame->args_num;
    cg_tramp_seen_calls++;
    return frame->args[1];
}

static void cg_tramp_build_ptr_caller(sct_vector_t *instrs, sct_vector_t *tramp_args)
{
    sct_vector_t fun_args;
    sct_vector_init(&fun_args, sizeof(micro_instruction_fun_arg_t));
    cg_tramp_push_arg(&fun_args, MICRO_TYPE_PTR, "p");
    cg_tramp_push_arg(&fun_args, MICRO_TYPE_I32, "n");

    sct_vector_t exprs[2];
    cg_tramp_make_expr(&exprs[0], "p");
    cg_tramp_make_expr(&exprs[1], "n");

    sct_vector_t call_args;
    sct_vector_init(&call_args, sizeof(micro_expr_tok_t *));
    cg_tramp_push_expr(&call_args, &exprs[0]);
    cg_tramp_push_expr(&call_args, &exprs[1]);

    sct_vector_t body;
    sct_vector_init(&body, sizeof(micro_instruction_t));

    micro_instruction_hints_t hints = { .lifetime = -1 };
    micro_instr_gen_tramp(instrs, "vm_ptr", tramp_args, MICRO_TYPE_I32, hints);
    micro_instr_gen_set(&body, MICRO_TYPE_I32, "r", NULL, hints);
    micro_instr_gen_call(&body, "r", "vm_ptr", &call_args, hints);

    sct_vector_t ret_expr;
    cg_tramp_make_expr(&ret_expr, "r");
    micro_instr_gen_ret(&body, &ret_expr, hints);
    micro_instr_gen_fun(instrs, "f", &fun_args, MICRO_TYPE_I32, &body, hints);
}

MunitResult test_codegen_tramp_call_no_args(const MunitParameter params[], void *data)
{
    micro_init();

    cg_tramp_seen_calls = 0;

    sct_vector_t tramp_args;
    sct_vector_init(&tramp_args, sizeof(micro_instruction_fun_arg_t));

    sct_vector_t instrs;
    sct_vector_init(&instrs, sizeof(micro_instruction_t));
    cg_tramp_build(&instrs, &tramp_args, "vm_none", MICRO_TYPE_I32, 0, 0,
                   MICRO_TYPE_I32, MICRO_TYPE_I32);

    sct_hashmap_t tramps;
    sct_hashmap_init(&tramps, sizeof(micro_tramp_t));
    micro_tramp_t handler = cg_tramp_none;
    sct_hashmap_add(&tramps, "vm_none", &handler);

    cg_ctx_t ctx;
    cg_gen_instrs(&ctx, &instrs, &tramps);
    munit_assert_size(micro_err_stk_size, ==, 0);

    munit_assert_int(cg_tramp_run(&ctx, 0, 0), ==, 77);
    munit_assert_size(cg_tramp_seen_args_num, ==, 0);
    munit_assert_int((int)cg_tramp_seen_calls, ==, 1);

    cg_cleanup(&ctx);
    sct_hashmap_deinit(&tramps);
    sct_vector_deinit(&instrs);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_tramp_call_ptr_arg(const MunitParameter params[], void *data)
{
    micro_init();

    cg_tramp_seen_calls = 0;
    cg_tramp_seen_ptr_value = -1;

    sct_vector_t tramp_args;
    sct_vector_init(&tramp_args, sizeof(micro_instruction_fun_arg_t));
    cg_tramp_push_arg(&tramp_args, MICRO_TYPE_PTR, "p");
    cg_tramp_push_arg(&tramp_args, MICRO_TYPE_I32, "n");

    sct_vector_t instrs;
    sct_vector_init(&instrs, sizeof(micro_instruction_t));
    cg_tramp_build_ptr_caller(&instrs, &tramp_args);

    sct_hashmap_t tramps;
    sct_hashmap_init(&tramps, sizeof(micro_tramp_t));
    micro_tramp_t handler = cg_tramp_ptr;
    sct_hashmap_add(&tramps, "vm_ptr", &handler);

    cg_ctx_t ctx;
    cg_gen_instrs(&ctx, &instrs, &tramps);
    munit_assert_size(micro_err_stk_size, ==, 0);

    i32 slot = 21;
    munit_assert_int(cg_tramp_run_ptr(&ctx, &slot, 7), ==, 7);
    munit_assert_int((int)cg_tramp_seen_calls, ==, 1);
    munit_assert_size(cg_tramp_seen_args_num, ==, 2);
    munit_assert_int(cg_tramp_seen_ptr_value, ==, 21);

    cg_cleanup(&ctx);
    sct_hashmap_deinit(&tramps);
    sct_vector_deinit(&instrs);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_tramp_undefined(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t tramp_args;
    sct_vector_init(&tramp_args, sizeof(micro_instruction_fun_arg_t));
    cg_tramp_push_arg(&tramp_args, MICRO_TYPE_I32, "a");

    sct_vector_t instrs;
    sct_vector_init(&instrs, sizeof(micro_instruction_t));
    micro_instr_gen_tramp(&instrs, "vm_missing", &tramp_args, MICRO_TYPE_I32,
                          (micro_instruction_hints_t){ .lifetime = -1 });

    sct_hashmap_t tramps;
    sct_hashmap_init(&tramps, sizeof(micro_tramp_t));

    cg_ctx_t ctx;
    cg_gen_instrs(&ctx, &instrs, &tramps);

    munit_assert_size(micro_err_stk_size, ==, 1);
    munit_assert_int((int)micro_err_stk[0].err, ==, (int)MICRO_ERROR_UNDEFINED_TRAMP);
    munit_assert_int((int)micro_err_stk[0].instr, ==, (int)MICRO_INSTR_TRAMP);

    cg_cleanup(&ctx);
    sct_hashmap_deinit(&tramps);
    sct_vector_deinit(&instrs);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_tramp_no_map(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t tramp_args;
    sct_vector_init(&tramp_args, sizeof(micro_instruction_fun_arg_t));
    cg_tramp_push_arg(&tramp_args, MICRO_TYPE_I32, "a");

    sct_vector_t instrs;
    sct_vector_init(&instrs, sizeof(micro_instruction_t));
    micro_instr_gen_tramp(&instrs, "vm_add", &tramp_args, MICRO_TYPE_I32,
                          (micro_instruction_hints_t){ .lifetime = -1 });

    cg_ctx_t ctx;
    cg_gen_instrs(&ctx, &instrs, NULL);

    munit_assert_size(micro_err_stk_size, ==, 1);
    munit_assert_int((int)micro_err_stk[0].err, ==, (int)MICRO_ERROR_UNDEFINED_TRAMP);

    cg_cleanup(&ctx);
    sct_vector_deinit(&instrs);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_tramp_redefined(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t tramp_args;
    sct_vector_init(&tramp_args, sizeof(micro_instruction_fun_arg_t));
    cg_tramp_push_arg(&tramp_args, MICRO_TYPE_I32, "a");

    sct_vector_t instrs;
    sct_vector_init(&instrs, sizeof(micro_instruction_t));
    micro_instruction_hints_t hints = { .lifetime = -1 };
    micro_instr_gen_tramp(&instrs, "vm_add", &tramp_args, MICRO_TYPE_I32, hints);
    micro_instr_gen_tramp(&instrs, "vm_add", &tramp_args, MICRO_TYPE_I32, hints);

    sct_hashmap_t tramps;
    sct_hashmap_init(&tramps, sizeof(micro_tramp_t));
    micro_tramp_t handler = cg_tramp_add;
    sct_hashmap_add(&tramps, "vm_add", &handler);

    cg_ctx_t ctx;
    cg_gen_instrs(&ctx, &instrs, &tramps);

    munit_assert_size(micro_err_stk_size, ==, 1);
    munit_assert_int((int)micro_err_stk[0].err, ==, (int)MICRO_ERROR_TRAMP_REDEFINED);

    cg_cleanup(&ctx);
    sct_hashmap_deinit(&tramps);
    sct_vector_deinit(&instrs);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_tramp_call_arg_mismatch(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t tramp_args;
    sct_vector_init(&tramp_args, sizeof(micro_instruction_fun_arg_t));
    cg_tramp_push_arg(&tramp_args, MICRO_TYPE_I32, "a");
    cg_tramp_push_arg(&tramp_args, MICRO_TYPE_I32, "b");

    sct_vector_t instrs;
    sct_vector_init(&instrs, sizeof(micro_instruction_t));
    cg_tramp_build(&instrs, &tramp_args, "vm_add", MICRO_TYPE_I32, 2, 1, MICRO_TYPE_I32, MICRO_TYPE_I32);

    sct_hashmap_t tramps;
    sct_hashmap_init(&tramps, sizeof(micro_tramp_t));
    micro_tramp_t handler = cg_tramp_add;
    sct_hashmap_add(&tramps, "vm_add", &handler);

    cg_ctx_t ctx;
    cg_gen_instrs(&ctx, &instrs, &tramps);

    munit_assert_size(micro_err_stk_size, ==, 1);
    munit_assert_int((int)micro_err_stk[0].err, ==, (int)MICRO_ERROR_TOO_FEW_ARGS);
    munit_assert_int((int)micro_err_stk[0].instr, ==, (int)MICRO_INSTR_CALL);

    cg_cleanup(&ctx);
    sct_hashmap_deinit(&tramps);
    sct_vector_deinit(&instrs);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_tramp_call_result_type(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t tramp_args;
    sct_vector_init(&tramp_args, sizeof(micro_instruction_fun_arg_t));
    cg_tramp_push_arg(&tramp_args, MICRO_TYPE_I32, "a");

    sct_vector_t instrs;
    sct_vector_init(&instrs, sizeof(micro_instruction_t));
    cg_tramp_build(&instrs, &tramp_args, "vm_add", MICRO_TYPE_I32, 1, 1, MICRO_TYPE_U32, MICRO_TYPE_U32);

    sct_hashmap_t tramps;
    sct_hashmap_init(&tramps, sizeof(micro_tramp_t));
    micro_tramp_t handler = cg_tramp_add;
    sct_hashmap_add(&tramps, "vm_add", &handler);

    cg_ctx_t ctx;
    cg_gen_instrs(&ctx, &instrs, &tramps);

    munit_assert_size(micro_err_stk_size, ==, 1);
    munit_assert_int((int)micro_err_stk[0].err, ==, (int)MICRO_ERROR_RESULT_TYPE_MISMATCH);

    cg_cleanup(&ctx);
    sct_hashmap_deinit(&tramps);
    sct_vector_deinit(&instrs);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_err_unimplemented_op(const MunitParameter params[], void *data)
{
    micro_init();

    cg_ctx_t ctx;
    cg_gen(&ctx, "fun f\n"
                 "start\n"
                 "    set i32 a 4;\n"
                 "    set i32 b ~ a;\n"
                 "end\n");

    munit_assert_size(micro_err_stk_size, ==, 1);
    munit_assert_int((int)micro_err_stk[0].err, ==, (int)MICRO_ERROR_EXPR_PARSE);
    munit_assert_int((int)micro_err_stk[0].instr, ==, (int)MICRO_INSTR_SET);

    cg_cleanup(&ctx);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_err_unimplemented_op_in_if(const MunitParameter params[], void *data)
{
    micro_init();

    cg_ctx_t ctx;
    cg_gen(&ctx, "fun f\n"
                 "start\n"
                 "    set i32 a 4;\n"
                 "    if ~ = a 1 : end_if;\n"
                 "end_if:\n"
                 "end\n");

    munit_assert_size(micro_err_stk_size, ==, 1);
    munit_assert_int((int)micro_err_stk[0].err, ==, (int)MICRO_ERROR_EXPR_PARSE);

    cg_cleanup(&ctx);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_drset_keeps_operand_reg(const MunitParameter params[], void *data)
{
    micro_init();

    cg_ctx_t ctx;
    cg_gen(&ctx, "fun f\n"
                 "    ptr p\n"
                 "    ret i32\n"
                 "start\n"
                 "    set i32 v $p;\n"
                 "    set i32 $p + v 1;\n"
                 "    ret v;\n"
                 "end\n");

    munit_assert_size(micro_err_stk_size, ==, 0);

    // v is allocated to eax, the deref target must not be computed there
    micro_asm386_emit(&ctx.asm_instrs, &ctx.outbuf);

    cg_cleanup(&ctx);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_codegen_call_discard_result(const MunitParameter params[], void *data)
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
                 "fun f\n"
                 "    ret i32\n"
                 "start\n"
                 "    call _ add 3 4;\n"
                 "    set i32 res 1;\n"
                 "    ret res;\n"
                 "end\n");

    munit_assert_size(micro_err_stk_size, ==, 0);

    int calls = 0;
    for (size_t i = 0; i < cg_asm_size(&ctx); i++) {
        if (cg_asm(&ctx, i)->opcode == MICRO_ASM386_INSTR_CALL_L32) {
            calls++;
        }
    }
    munit_assert_int(calls, ==, 1);

    micro_asm386_emit(&ctx.asm_instrs, &ctx.outbuf);

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
    { "/lifetime_keeps_vreg", test_codegen_lifetime_keeps_vreg, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/lifetime_counts_from_declaration", test_codegen_lifetime_counts_from_declaration, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/lifetime_counts_from_declaration_expired", test_codegen_lifetime_counts_from_declaration_expired, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/lifetime_expires", test_codegen_lifetime_expires, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/lifetime_zero_expires_immediately", test_codegen_lifetime_zero_expires_immediately, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/lifetime_frees_name", test_codegen_lifetime_frees_name, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/no_lifetime_lives_to_end", test_codegen_no_lifetime_lives_to_end, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/lifetime_frees_registers", test_codegen_lifetime_frees_registers, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
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
    { "/lea_mul_scale", test_codegen_lea_mul_scale, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/lea_mul_scale_lit_first", test_codegen_lea_mul_scale_lit_first, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/lea_plus_index_scale", test_codegen_lea_plus_index_scale, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/lea_sum_disp8", test_codegen_lea_sum_disp8, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/lea_sum_disp8_neg", test_codegen_lea_sum_disp8_neg, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/lea_sum_disp32", test_codegen_lea_sum_disp32, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/lea_scaled_addend", test_codegen_lea_scaled_addend, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/lea_scaled_plus_base", test_codegen_lea_scaled_plus_base, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/lea_lit_plus_scaled", test_codegen_lea_lit_plus_scaled, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/lea_index_offset", test_codegen_lea_index_offset, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/lea_index_offset_mul", test_codegen_lea_index_offset_mul, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/lea_index_offset_lit_first", test_codegen_lea_index_offset_lit_first, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/lea_index_offset_addend", test_codegen_lea_index_offset_addend, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/no_lea_for_non_scale", test_codegen_no_lea_for_non_scale, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/add_one_to_inc", test_codegen_add_one_to_inc, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/sub_one_to_dec", test_codegen_sub_one_to_dec, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/add_zero_stays_gone", test_codegen_add_zero_stays_gone, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/add_lit_to_lea", test_codegen_add_lit_to_lea, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/add_lit_first_to_lea", test_codegen_add_lit_first_to_lea, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/add_lit_disp32_to_lea", test_codegen_add_lit_disp32_to_lea, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/add_lit_stack_base_stays_add", test_codegen_add_lit_stack_base_stays_add, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/no_lea_for_non_scale_mul", test_codegen_no_lea_for_non_scale_mul, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/no_lea_for_stack_vreg", test_codegen_no_lea_for_stack_vreg, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/stack_overflow", test_codegen_stack_overflow, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/ret_expr", test_codegen_ret_expr, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/ret_vreg", test_codegen_ret_vreg, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/call", test_codegen_call, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/call_spills_results", test_codegen_call_spills_results, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
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
    { "/drset_keeps_operand_reg", test_codegen_drset_keeps_operand_reg, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/call_discard_result", test_codegen_call_discard_result, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/tramp_source_call", test_codegen_tramp_source_call, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/tramp_source_no_map", test_codegen_tramp_source_no_map, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/tramp_fun_name_collision", test_codegen_tramp_fun_name_collision, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/tramp_after_fun", test_codegen_tramp_after_fun, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/tramp_inside_fun", test_codegen_tramp_inside_fun, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/tramp_call", test_codegen_tramp_call, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/tramp_call_many_args", test_codegen_tramp_call_many_args, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/tramp_call_no_args", test_codegen_tramp_call_no_args, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/tramp_call_ptr_arg", test_codegen_tramp_call_ptr_arg, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/tramp_undefined", test_codegen_tramp_undefined, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/tramp_no_map", test_codegen_tramp_no_map, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/tramp_redefined", test_codegen_tramp_redefined, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/tramp_call_arg_mismatch", test_codegen_tramp_call_arg_mismatch, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/tramp_call_result_type", test_codegen_tramp_call_result_type, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/err_unimplemented_op", test_codegen_err_unimplemented_op, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/err_unimplemented_op_in_if", test_codegen_err_unimplemented_op_in_if, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/err_call_result_undef", test_codegen_err_call_result_undef, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/err_call_result_type", test_codegen_err_call_result_type, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite codegen386_suite = {
    "/codegen386", codegen386_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};

#endif