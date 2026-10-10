#ifndef TESTS_IROPTER_H
#define TESTS_IROPTER_H

#include "../include/munit.h"
#include "errors.h"
#include <micro/micro.h>
#include <micro/asm/asm386.h>
#include <microc/lexer.h>
#include <microc/instrgen.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    sct_vector_t     toks;
    mc_instrgen_t    ig;
    micro_iropter_t  iropter;
    micro_codegen_t  cg;
    sct_vector_t     asm_instrs;
    sct_vector_t     outbuf;
    sct_arena_t      arena;
    int              inlined;
} iropter_ctx_t;

/* tokenize, generate instructions, run the inlining pass */
static void iropter_gen(iropter_ctx_t *ctx, const char *text)
{
    sct_vector_init(&ctx->toks, sizeof(mc_token_t));
    mc_tokenize(text, strlen(text), &ctx->toks);

    mc_instrgen_init(&ctx->ig, &ctx->toks);
    mc_instrgen_gen(&ctx->ig);

    micro_iropter_init(&ctx->iropter, &ctx->ig.instructions);
    micro_iropter_inlining_pass(&ctx->iropter);
    ctx->inlined = (int)ctx->iropter.inlined_calls;

    test_put_errors("instrgen");
}

static void iropter_cleanup(iropter_ctx_t *ctx)
{
    micro_iropter_deinit(&ctx->iropter);
    mc_instrgen_deinit(&ctx->ig);
    sct_vector_deinit(&ctx->toks);
}

/* the body of the named function of the top level list */
static sct_vector_t *iropter_body(sct_vector_t *instrs, const char *name)
{
    for (size_t i = 0; i < instrs->size; i++) {
        micro_instruction_t *instr = sct_vector_get(instrs, i);
        if (instr->type == MICRO_INSTR_FUN && !strcmp(instr->fun.name, name)) {
            return &instr->fun.body;
        }
    }
    return NULL;
}

static size_t iropter_calls_num(sct_vector_t *body)
{
    size_t num = 0;
    for (size_t i = 0; i < body->size; i++) {
        micro_instruction_t *instr = sct_vector_get(body, i);
        if (instr->type == MICRO_INSTR_CALL) {
            num++;
        }
    }
    return num;
}

static int iropter_has_name(sct_vector_t *body, const char *name)
{
    for (size_t i = 0; i < body->size; i++) {
        micro_instruction_t *instr = sct_vector_get(body, i);
        const char *reg_name = NULL;
        switch (instr->type) {
            case MICRO_INSTR_SET:   reg_name = instr->set.reg_name;   break;
            case MICRO_INSTR_DRSET: reg_name = instr->drset.reg_name; break;
            case MICRO_INSTR_CALL:  reg_name = instr->call.ret_reg_name; break;
            default: continue;
        }
        if (!strcmp(reg_name, name)) {
            return 1;
        }
    }
    return 0;
}

MunitResult test_iropter_inline_leaf(const MunitParameter params[], void *data)
{
    micro_init();

    iropter_ctx_t ctx;
    iropter_gen(&ctx, "fun leaf\n"
                      "    i32 a\n"
                      "    ret i32\n"
                      "start\n"
                      "    ret + a 1;\n"
                      "end\n"
                      "\n"
                      "fun caller\n"
                      "    i32 a\n"
                      "    ret i32\n"
                      "start\n"
                      "    set i32 r;\n"
                      "    call r leaf a;\n"
                      "    ret r;\n"
                      "end\n");

    munit_assert_size(micro_err_stk_size, ==, 0);
    munit_assert_int(ctx.inlined, ==, 1);

    sct_vector_t *caller = iropter_body(&ctx.ig.instructions, "caller");
    munit_assert_ptr_not_null(caller);
    munit_assert_size(iropter_calls_num(caller), ==, 0);

    /* the result register keeps its name, everything of the callee is prefixed */
    munit_assert_true(iropter_has_name(caller, "r"));
    munit_assert_false(iropter_has_name(caller, "a"));

    iropter_cleanup(&ctx);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_iropter_inline_rescans_chunk(const MunitParameter params[], void *data)
{
    micro_init();

    iropter_ctx_t ctx;
    iropter_gen(&ctx, "fun leaf\n"
                      "    i32 a\n"
                      "    ret i32\n"
                      "start\n"
                      "    ret + a 1;\n"
                      "end\n"
                      "\n"
                      "fun mid\n"
                      "    i32 a\n"
                      "    ret i32\n"
                      "start\n"
                      "    set i32 x;\n"
                      "    call x leaf a;\n"
                      "    ret x;\n"
                      "end\n"
                      "\n"
                      "fun caller\n"
                      "    i32 a\n"
                      "    ret i32\n"
                      "start\n"
                      "    set i32 r;\n"
                      "    call r mid a;\n"
                      "    ret r;\n"
                      "end\n");

    munit_assert_size(micro_err_stk_size, ==, 0);

    /* leaf goes into mid, mid goes into caller, and the call to leaf that came
       along with it is found by the rescan of the inserted chunk */
    munit_assert_int(ctx.inlined, ==, 3);
    munit_assert_size(iropter_calls_num(iropter_body(&ctx.ig.instructions, "caller")), ==, 0);
    munit_assert_size(iropter_calls_num(iropter_body(&ctx.ig.instructions, "mid")), ==, 0);

    iropter_cleanup(&ctx);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_iropter_skips_recursion(const MunitParameter params[], void *data)
{
    micro_init();

    iropter_ctx_t ctx;
    iropter_gen(&ctx, "fun fib\n"
                      "    i32 n\n"
                      "    ret i32\n"
                      "start\n"
                      "    if <= n 1 : base;\n"
                      "    set i32 n1 - n 1;\n"
                      "    set i32 f1;\n"
                      "    call f1 fib n1;\n"
                      "    ret + f1 n1;\n"
                      "base:\n"
                      "    ret n;\n"
                      "end\n");

    munit_assert_size(micro_err_stk_size, ==, 0);
    munit_assert_int(ctx.inlined, ==, 0);
    munit_assert_size(iropter_calls_num(iropter_body(&ctx.ig.instructions, "fib")), ==, 1);

    iropter_cleanup(&ctx);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_iropter_inline_control_flow(const MunitParameter params[], void *data)
{
    micro_init();

    iropter_ctx_t ctx;
    iropter_gen(&ctx, "fun pick\n"
                      "    i32 a\n"
                      "    i32 hi\n"
                      "    ret i32\n"
                      "start\n"
                      "    if > a hi : small;\n"
                      "    ret hi;\n"
                      "small:\n"
                      "    ret a;\n"
                      "end\n"
                      "\n"
                      "fun caller\n"
                      "    i32 a\n"
                      "    ret i32\n"
                      "start\n"
                      "    set i32 r;\n"
                      "    call r pick a 100;\n"
                      "    ret r;\n"
                      "end\n");

    munit_assert_size(micro_err_stk_size, ==, 0);
    munit_assert_int(ctx.inlined, ==, 1);

    sct_vector_t *caller = iropter_body(&ctx.ig.instructions, "caller");
    munit_assert_ptr_not_null(caller);

    /* the label of the callee keeps its name under a prefix, every one of its
       returns leaves the chunk through the continuation label */
    size_t labels = 0;
    size_t leaves = 0;
    size_t end_label = 0;
    for (size_t i = 0; i < caller->size; i++) {
        micro_instruction_t *instr = sct_vector_get(caller, i);
        if (instr->type == MICRO_INSTR_LBL && !strcmp(instr->lbl.name, "i0.small")) {
            labels++;
        }
        if (instr->type == MICRO_INSTR_LBL && !strcmp(instr->lbl.name, "i0")) {
            end_label++;
        }
        if (instr->type == MICRO_INSTR_GOTO && !strcmp(instr->goto_lbl.lbl, "i0")) {
            leaves++;
        }
    }
    munit_assert_size(labels, ==, 1);
    munit_assert_size(end_label, ==, 1);
    munit_assert_size(leaves, ==, 2);

    iropter_cleanup(&ctx);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_iropter_growth_budget(const MunitParameter params[], void *data)
{
    micro_init();

    iropter_ctx_t ctx;
    iropter_gen(&ctx, "fun leaf\n"
                      "    i32 a\n"
                      "    ret i32\n"
                      "start\n"
                      "    ret + a 1;\n"
                      "end\n"
                      "\n"
                      "fun mid\n"
                      "    i32 a\n"
                      "    ret i32\n"
                      "start\n"
                      "    set i32 x;\n"
                      "    call x leaf a;\n"
                      "    set i32 y;\n"
                      "    call y leaf x;\n"
                      "    ret + x y;\n"
                      "end\n"
                      "\n"
                      "fun caller\n"
                      "    i32 a\n"
                      "    ret i32\n"
                      "start\n"
                      "    set i32 r1;\n"
                      "    call r1 mid a;\n"
                      "    set i32 r2;\n"
                      "    call r2 mid r1;\n"
                      "    set i32 r3;\n"
                      "    call r3 mid r2;\n"
                      "    ret + r1 r3;\n"
                      "end\n");

    munit_assert_size(micro_err_stk_size, ==, 0);

    /* the unit may grow, but not without a limit */
    munit_assert_true(ctx.inlined > 0);
    munit_assert_true(ctx.iropter.inlined_instrs <= ctx.iropter.budget);

    iropter_cleanup(&ctx);

    micro_deinit();

    return MUNIT_OK;
}

static MunitTest iropter_tests[] = {
    { "/inline_leaf", test_iropter_inline_leaf, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/inline_rescans_chunk", test_iropter_inline_rescans_chunk, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/skips_recursion", test_iropter_skips_recursion, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/inline_control_flow", test_iropter_inline_control_flow, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/growth_budget", test_iropter_growth_budget, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite iropter_suite = {
    "/iropter", iropter_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};

#endif