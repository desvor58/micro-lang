#ifndef TESTS_INSTRGEN_H
#define TESTS_INSTRGEN_H

#include "../include/munit.h"
#include "errors.h"
#include <microc/instrgen.h>
#include <stdio.h>
#include <string.h>

static void ig_tokenize(sct_vector_t *toks, const char *text)
{
    mc_tokenize(text, strlen(text), toks);
}

static void ig_gen(const char *text, sct_vector_t *toks, mc_instrgen_t *ig)
{
    sct_vector_init(toks, sizeof(mc_token_t));
    ig_tokenize(toks, text);

    mc_instrgen_init(ig, toks);
    mc_instrgen_gen(ig);

    /* dump accumulated errors, if any, like microc does */
    test_put_errors("instrgen");
}

static micro_instruction_t *ig_instr(mc_instrgen_t *ig, size_t i)
{
    return sct_vector_get(&ig->instructions, i);
}

static micro_instruction_t *ig_body_instr(micro_instruction_t *fun_instr, size_t i)
{
    return sct_vector_get(&fun_instr->fun.body, i);
}

MunitResult test_instrgen_type_str_parse(const MunitParameter params[], void *data)
{
    micro_init();

    munit_assert_int((int)mc_type_str_parse("i8"), ==, (int)MICRO_TYPE_I8);
    munit_assert_int((int)mc_type_str_parse("u8"), ==, (int)MICRO_TYPE_U8);
    munit_assert_int((int)mc_type_str_parse("i16"), ==, (int)MICRO_TYPE_I16);
    munit_assert_int((int)mc_type_str_parse("u16"), ==, (int)MICRO_TYPE_U16);
    munit_assert_int((int)mc_type_str_parse("i32"), ==, (int)MICRO_TYPE_I32);
    munit_assert_int((int)mc_type_str_parse("u32"), ==, (int)MICRO_TYPE_U32);
    munit_assert_int((int)mc_type_str_parse("f32"), ==, (int)MICRO_TYPE_F32);
    munit_assert_int((int)mc_type_str_parse("ptr"), ==, (int)MICRO_TYPE_PTR);
    munit_assert_int((int)mc_type_str_parse("foo"), ==, (int)MICRO_TYPE_NULL);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_instrgen_fun_empty(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t toks;
    mc_instrgen_t ig;
    ig_gen("fun empty\nstart\nend\n", &toks, &ig);

    munit_assert_size(micro_err_stk_size, ==, 0);
    munit_assert_size(ig.instructions.size, ==, 1);

    micro_instruction_t *fun = ig_instr(&ig, 0);
    munit_assert_int((int)fun->type, ==, (int)MICRO_INSTR_FUN);
    munit_assert_string_equal(fun->fun.name, "empty");
    munit_assert_int((int)fun->fun.ret_type, ==, (int)MICRO_TYPE_NULL);
    munit_assert_size(fun->fun.args.size, ==, 0);
    munit_assert_size(fun->fun.body.size, ==, 0);

    mc_instrgen_deinit(&ig);
    sct_vector_deinit(&toks);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_instrgen_fun_args_ret(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t toks;
    mc_instrgen_t ig;
    ig_gen("fun add\n"
           "    i32 a\n"
           "    i32 b\n"
           "    ret i32\n"
           "start\n"
           "end\n", &toks, &ig);

    munit_assert_size(micro_err_stk_size, ==, 0);
    munit_assert_size(ig.instructions.size, ==, 1);

    micro_instruction_t *fun = ig_instr(&ig, 0);
    munit_assert_int((int)fun->type, ==, (int)MICRO_INSTR_FUN);
    munit_assert_string_equal(fun->fun.name, "add");
    munit_assert_int((int)fun->fun.ret_type, ==, (int)MICRO_TYPE_I32);
    munit_assert_size(fun->fun.args.size, ==, 2);

    micro_instruction_fun_arg_t *arg0 = sct_vector_get(&fun->fun.args, 0);
    munit_assert_int((int)arg0->type, ==, (int)MICRO_TYPE_I32);
    munit_assert_string_equal(arg0->name, "a");

    micro_instruction_fun_arg_t *arg1 = sct_vector_get(&fun->fun.args, 1);
    munit_assert_int((int)arg1->type, ==, (int)MICRO_TYPE_I32);
    munit_assert_string_equal(arg1->name, "b");

    mc_instrgen_deinit(&ig);
    sct_vector_deinit(&toks);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_instrgen_set_no_val(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t toks;
    mc_instrgen_t ig;
    ig_gen("fun f\n"
           "start\n"
           "    set i32 count;\n"
           "end\n", &toks, &ig);

    munit_assert_size(micro_err_stk_size, ==, 0);
    munit_assert_size(ig.instructions.size, ==, 1);

    micro_instruction_t *set = ig_body_instr(ig_instr(&ig, 0), 0);
    munit_assert_int((int)set->type, ==, (int)MICRO_INSTR_SET);
    munit_assert_int((int)set->set.type, ==, (int)MICRO_TYPE_I32);
    munit_assert_string_equal(set->set.reg_name, "count");
    munit_assert_ptr_null(set->set.val_expr);

    mc_instrgen_deinit(&ig);
    sct_vector_deinit(&toks);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_instrgen_set_lit(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t toks;
    mc_instrgen_t ig;
    ig_gen("fun f\n"
           "start\n"
           "    set i32 number 5;\n"
           "end\n", &toks, &ig);

    munit_assert_size(micro_err_stk_size, ==, 0);

    micro_instruction_t *set = ig_body_instr(ig_instr(&ig, 0), 0);
    munit_assert_int((int)set->type, ==, (int)MICRO_INSTR_SET);
    munit_assert_int((int)set->set.type, ==, (int)MICRO_TYPE_I32);
    munit_assert_string_equal(set->set.reg_name, "number");
    munit_assert_ptr_not_null(set->set.val_expr);
    munit_assert_int((int)set->set.val_expr->type, ==, (int)MC_TOK_LIT_INT);
    munit_assert_string_equal(set->set.val_expr->val, "5");

    mc_instrgen_deinit(&ig);
    sct_vector_deinit(&toks);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_instrgen_set_expr(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t toks;
    mc_instrgen_t ig;
    ig_gen("fun f\n"
           "start\n"
           "    set i32 total + number 2;\n"
           "end\n", &toks, &ig);

    munit_assert_size(micro_err_stk_size, ==, 0);

    micro_instruction_t *set = ig_body_instr(ig_instr(&ig, 0), 0);
    munit_assert_int((int)set->type, ==, (int)MICRO_INSTR_SET);
    munit_assert_int((int)set->set.type, ==, (int)MICRO_TYPE_I32);
    munit_assert_string_equal(set->set.reg_name, "total");
    munit_assert_ptr_not_null(set->set.val_expr);
    munit_assert_int((int)set->set.val_expr->type, ==, (int)MC_TOK_PLUS);

    mc_instrgen_deinit(&ig);
    sct_vector_deinit(&toks);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_instrgen_drset(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t toks;
    mc_instrgen_t ig;
    ig_gen("fun f\n"
           "start\n"
           "    set i32 $slot 42;\n"
           "end\n", &toks, &ig);

    munit_assert_size(micro_err_stk_size, ==, 0);

    micro_instruction_t *drset = ig_body_instr(ig_instr(&ig, 0), 0);
    munit_assert_int((int)drset->type, ==, (int)MICRO_INSTR_DRSET);
    munit_assert_int((int)drset->drset.type, ==, (int)MICRO_TYPE_I32);
    munit_assert_string_equal(drset->drset.reg_name, "slot");
    munit_assert_ptr_not_null(drset->drset.val_expr);
    munit_assert_int((int)drset->drset.val_expr->type, ==, (int)MC_TOK_LIT_INT);
    munit_assert_string_equal(drset->drset.val_expr->val, "42");

    mc_instrgen_deinit(&ig);
    sct_vector_deinit(&toks);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_instrgen_ret_with_val(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t toks;
    mc_instrgen_t ig;
    ig_gen("fun f\n"
           "    ret i32\n"
           "start\n"
           "    ret + a b;\n"
           "end\n", &toks, &ig);

    munit_assert_size(micro_err_stk_size, ==, 0);

    micro_instruction_t *ret = ig_body_instr(ig_instr(&ig, 0), 0);
    munit_assert_int((int)ret->type, ==, (int)MICRO_INSTR_RET);
    munit_assert_ptr_not_null(ret->ret.val_expr);
    munit_assert_int((int)ret->ret.val_expr->type, ==, (int)MC_TOK_PLUS);

    mc_instrgen_deinit(&ig);
    sct_vector_deinit(&toks);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_instrgen_ret_no_val(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t toks;
    mc_instrgen_t ig;
    ig_gen("fun f\n"
           "start\n"
           "    ret;\n"
           "end\n", &toks, &ig);

    munit_assert_size(micro_err_stk_size, ==, 0);

    micro_instruction_t *ret = ig_body_instr(ig_instr(&ig, 0), 0);
    munit_assert_int((int)ret->type, ==, (int)MICRO_INSTR_RET);
    munit_assert_ptr_null(ret->ret.val_expr);

    mc_instrgen_deinit(&ig);
    sct_vector_deinit(&toks);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_instrgen_call(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t toks;
    mc_instrgen_t ig;
    ig_gen("fun f\n"
           "    ret i32\n"
           "start\n"
           "    set i32 res;\n"
           "    call res add 10 5;\n"
           "end\n", &toks, &ig);

    munit_assert_size(micro_err_stk_size, ==, 0);

    micro_instruction_t *call = ig_body_instr(ig_instr(&ig, 0), 1);
    munit_assert_int((int)call->type, ==, (int)MICRO_INSTR_CALL);
    munit_assert_string_equal(call->call.ret_reg_name, "res");
    munit_assert_string_equal(call->call.fun_name, "add");
    munit_assert_size(call->call.arg_exprs.size, ==, 2);

    mc_token_t **arg0 = sct_vector_get(&call->call.arg_exprs, 0);
    munit_assert_int((int)(*arg0)->type, ==, (int)MC_TOK_LIT_INT);
    munit_assert_string_equal((*arg0)->val, "10");

    mc_token_t **arg1 = sct_vector_get(&call->call.arg_exprs, 1);
    munit_assert_int((int)(*arg1)->type, ==, (int)MC_TOK_LIT_INT);
    munit_assert_string_equal((*arg1)->val, "5");

    mc_instrgen_deinit(&ig);
    sct_vector_deinit(&toks);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_instrgen_call_no_args(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t toks;
    mc_instrgen_t ig;
    ig_gen("fun f\n"
           "start\n"
           "    call _ empty;\n"
           "end\n", &toks, &ig);

    munit_assert_size(micro_err_stk_size, ==, 0);

    micro_instruction_t *call = ig_body_instr(ig_instr(&ig, 0), 0);
    munit_assert_int((int)call->type, ==, (int)MICRO_INSTR_CALL);
    munit_assert_string_equal(call->call.ret_reg_name, "_");
    munit_assert_string_equal(call->call.fun_name, "empty");
    munit_assert_size(call->call.arg_exprs.size, ==, 0);

    mc_instrgen_deinit(&ig);
    sct_vector_deinit(&toks);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_instrgen_lbl_goto(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t toks;
    mc_instrgen_t ig;
    ig_gen("fun f\n"
           "start\n"
           "    goto my_lbl;\n"
           "my_lbl:\n"
           "    ret;\n"
           "end\n", &toks, &ig);

    munit_assert_size(micro_err_stk_size, ==, 0);

    micro_instruction_t *goto_instr = ig_body_instr(ig_instr(&ig, 0), 0);
    munit_assert_int((int)goto_instr->type, ==, (int)MICRO_INSTR_GOTO);
    munit_assert_string_equal(goto_instr->goto_lbl.lbl, "my_lbl");

    micro_instruction_t *lbl = ig_body_instr(ig_instr(&ig, 0), 1);
    munit_assert_int((int)lbl->type, ==, (int)MICRO_INSTR_LBL);
    munit_assert_string_equal(lbl->lbl.name, "my_lbl");

    mc_instrgen_deinit(&ig);
    sct_vector_deinit(&toks);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_instrgen_if_vreg(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t toks;
    mc_instrgen_t ig;
    ig_gen("fun f\n"
           "start\n"
           "    set i32 n 1;\n"
           "    if n : target;\n"
           "end\n", &toks, &ig);

    munit_assert_size(micro_err_stk_size, ==, 0);

    micro_instruction_t *if_instr = ig_body_instr(ig_instr(&ig, 0), 1);
    munit_assert_int((int)if_instr->type, ==, (int)MICRO_INSTR_IF);
    munit_assert_ptr_not_null(if_instr->if_goto.cond_expr);
    munit_assert_int((int)if_instr->if_goto.cond_expr->type, ==, (int)MC_TOK_IDENT);
    munit_assert_string_equal(if_instr->if_goto.cond_expr->val, "n");
    munit_assert_string_equal(if_instr->if_goto.lbl_name, "target");

    mc_instrgen_deinit(&ig);
    sct_vector_deinit(&toks);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_instrgen_if_cmp(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t toks;
    mc_instrgen_t ig;
    ig_gen("fun f\n"
           "start\n"
           "    if <= n 1 : end_rec;\n"
           "end\n", &toks, &ig);

    munit_assert_size(micro_err_stk_size, ==, 0);

    micro_instruction_t *if_instr = ig_body_instr(ig_instr(&ig, 0), 0);
    munit_assert_int((int)if_instr->type, ==, (int)MICRO_INSTR_IF);
    munit_assert_ptr_not_null(if_instr->if_goto.cond_expr);
    munit_assert_int((int)if_instr->if_goto.cond_expr->type, ==, (int)MC_TOK_LESS_OR_EQ);
    munit_assert_string_equal(if_instr->if_goto.lbl_name, "end_rec");

    mc_instrgen_deinit(&ig);
    sct_vector_deinit(&toks);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_instrgen_if_not(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t toks;
    mc_instrgen_t ig;
    ig_gen("fun f\n"
           "start\n"
           "    if ! n : target;\n"
           "end\n", &toks, &ig);

    munit_assert_size(micro_err_stk_size, ==, 0);

    micro_instruction_t *if_instr = ig_body_instr(ig_instr(&ig, 0), 0);
    munit_assert_int((int)if_instr->type, ==, (int)MICRO_INSTR_IF);
    munit_assert_ptr_not_null(if_instr->if_goto.cond_expr);
    munit_assert_int((int)if_instr->if_goto.cond_expr->type, ==, (int)MC_TOK_EXCLAMATION);
    munit_assert_string_equal(if_instr->if_goto.lbl_name, "target");

    mc_instrgen_deinit(&ig);
    sct_vector_deinit(&toks);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_instrgen_err_if_expected_expression(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t toks;
    mc_instrgen_t ig;
    ig_gen("fun f\n"
           "start\n"
           "    if ;\n"
           "end\n", &toks, &ig);

    munit_assert_size(micro_err_stk_size, ==, 1);
    munit_assert_int((int)micro_err_stk[0].err, ==, (int)MICRO_ERROR_EXPECTED_EXPRESSION);

    mc_instrgen_deinit(&ig);
    sct_vector_deinit(&toks);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_instrgen_err_if_expected_colon(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t toks;
    mc_instrgen_t ig;
    ig_gen("fun f\n"
           "start\n"
           "    if n ;\n"
           "end\n", &toks, &ig);

    munit_assert_size(micro_err_stk_size, ==, 1);
    munit_assert_int((int)micro_err_stk[0].err, ==, (int)MICRO_ERROR_EXPECTED_COLON);

    mc_instrgen_deinit(&ig);
    sct_vector_deinit(&toks);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_instrgen_err_if_expected_label_name(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t toks;
    mc_instrgen_t ig;
    ig_gen("fun f\n"
           "start\n"
           "    if n : ;\n"
           "end\n", &toks, &ig);

    munit_assert_size(micro_err_stk_size, ==, 1);
    munit_assert_int((int)micro_err_stk[0].err, ==, (int)MICRO_ERROR_EXPECTED_LABEL_NAME);

    mc_instrgen_deinit(&ig);
    sct_vector_deinit(&toks);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_instrgen_err_set_outside_fun(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t toks;
    mc_instrgen_t ig;
    ig_gen("set i32 x 5;\n", &toks, &ig);

    munit_assert_size(micro_err_stk_size, ==, 1);
    munit_assert_int((int)micro_err_stk[0].err, ==, (int)MICRO_ERROR_SET_OUTSIDE_FUNCTION);

    mc_instrgen_deinit(&ig);
    sct_vector_deinit(&toks);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_instrgen_err_ret_outside_fun(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t toks;
    mc_instrgen_t ig;
    ig_gen("ret 5;\n", &toks, &ig);

    munit_assert_size(micro_err_stk_size, ==, 1);
    munit_assert_int((int)micro_err_stk[0].err, ==, (int)MICRO_ERROR_RET_OUTSIDE_FUNCTION);

    mc_instrgen_deinit(&ig);
    sct_vector_deinit(&toks);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_instrgen_err_call_outside_fun(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t toks;
    mc_instrgen_t ig;
    ig_gen("call _ f;\n", &toks, &ig);

    munit_assert_size(micro_err_stk_size, ==, 1);
    munit_assert_int((int)micro_err_stk[0].err, ==, (int)MICRO_ERROR_CALL_OUTSIDE_FUNCTION);

    mc_instrgen_deinit(&ig);
    sct_vector_deinit(&toks);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_instrgen_err_goto_outside_fun(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t toks;
    mc_instrgen_t ig;
    ig_gen("goto lbl;\n", &toks, &ig);

    munit_assert_size(micro_err_stk_size, ==, 1);
    munit_assert_int((int)micro_err_stk[0].err, ==, (int)MICRO_ERROR_GOTO_OUTSIDE_FUNCTION);

    mc_instrgen_deinit(&ig);
    sct_vector_deinit(&toks);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_instrgen_err_lbl_outside_fun(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t toks;
    mc_instrgen_t ig;
    ig_gen("mylabel:\n", &toks, &ig);

    munit_assert_size(micro_err_stk_size, ==, 1);
    munit_assert_int((int)micro_err_stk[0].err, ==, (int)MICRO_ERROR_LBL_OUTSIDE_FUNCTION);

    mc_instrgen_deinit(&ig);
    sct_vector_deinit(&toks);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_instrgen_err_fun_inside_fun(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t toks;
    mc_instrgen_t ig;
    ig_gen("fun a\n"
           "start\n"
           "    fun b\n"
           "    start\n"
           "    end\n"
           "end\n", &toks, &ig);

    munit_assert_size(micro_err_stk_size, >=, 1);
    munit_assert_int((int)micro_err_stk[0].err, ==, (int)MICRO_ERROR_FUN_INSIDE_FUNCTION);

    mc_instrgen_deinit(&ig);
    sct_vector_deinit(&toks);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_instrgen_err_expected_type_name(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t toks;
    mc_instrgen_t ig;
    ig_gen("fun a\n"
           "start\n"
           "    set foo x 5;\n"
           "end\n", &toks, &ig);

    munit_assert_size(micro_err_stk_size, ==, 1);
    munit_assert_int((int)micro_err_stk[0].err, ==, (int)MICRO_ERROR_EXPECTED_TYPE_NAME);

    mc_instrgen_deinit(&ig);
    sct_vector_deinit(&toks);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_instrgen_err_expected_vreg_name(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t toks;
    mc_instrgen_t ig;
    ig_gen("fun a\n"
           "start\n"
           "    set i32 5;\n"
           "end\n", &toks, &ig);

    munit_assert_size(micro_err_stk_size, ==, 1);
    munit_assert_int((int)micro_err_stk[0].err, ==, (int)MICRO_ERROR_EXPECTED_VREG_NAME);

    mc_instrgen_deinit(&ig);
    sct_vector_deinit(&toks);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_instrgen_err_expected_expression(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t toks;
    mc_instrgen_t ig;
    ig_gen("fun a\n"
           "start\n"
           "    set i32 x ,;\n"
           "end\n", &toks, &ig);

    munit_assert_size(micro_err_stk_size, ==, 1);
    munit_assert_int((int)micro_err_stk[0].err, ==, (int)MICRO_ERROR_EXPECTED_EXPRESSION);

    mc_instrgen_deinit(&ig);
    sct_vector_deinit(&toks);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_instrgen_err_expected_semicolon(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t toks;
    mc_instrgen_t ig;
    ig_gen("fun a\n"
           "start\n"
           "    set i32 x 5\n"
           "end\n", &toks, &ig);

    munit_assert_size(micro_err_stk_size, ==, 1);
    munit_assert_int((int)micro_err_stk[0].err, ==, (int)MICRO_ERROR_EXPECTED_SEMICOLON);

    mc_instrgen_deinit(&ig);
    sct_vector_deinit(&toks);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_instrgen_err_expected_fun_name(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t toks;
    mc_instrgen_t ig;
    ig_gen("fun 5\n"
           "start\n"
           "end\n", &toks, &ig);

    munit_assert_size(micro_err_stk_size, ==, 1);
    munit_assert_int((int)micro_err_stk[0].err, ==, (int)MICRO_ERROR_EXPECTED_FUN_NAME);

    mc_instrgen_deinit(&ig);
    sct_vector_deinit(&toks);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_instrgen_err_expected_arg_type(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t toks;
    mc_instrgen_t ig;
    ig_gen("fun a\n"
           "    foo x\n"
           "start\n"
           "end\n", &toks, &ig);

    munit_assert_size(micro_err_stk_size, ==, 1);
    munit_assert_int((int)micro_err_stk[0].err, ==, (int)MICRO_ERROR_EXPECTED_ARG_TYPE);

    mc_instrgen_deinit(&ig);
    sct_vector_deinit(&toks);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_instrgen_err_expected_ret_type(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t toks;
    mc_instrgen_t ig;
    ig_gen("fun a\n"
           "    ret 5\n"
           "start\n"
           "end\n", &toks, &ig);

    munit_assert_size(micro_err_stk_size, ==, 1);
    munit_assert_int((int)micro_err_stk[0].err, ==, (int)MICRO_ERROR_EXPECTED_RET_TYPE);

    mc_instrgen_deinit(&ig);
    sct_vector_deinit(&toks);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_instrgen_err_expected_start(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t toks;
    mc_instrgen_t ig;
    ig_gen("fun a\n"
           "    ret i32\n"
           "    5\n"
           "end\n", &toks, &ig);

    munit_assert_size(micro_err_stk_size, ==, 1);
    munit_assert_int((int)micro_err_stk[0].err, ==, (int)MICRO_ERROR_EXPECTED_START_KW);

    mc_instrgen_deinit(&ig);
    sct_vector_deinit(&toks);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_instrgen_err_expected_end(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t toks;
    mc_instrgen_t ig;
    ig_gen("fun a\n"
           "start\n", &toks, &ig);

    munit_assert_size(micro_err_stk_size, ==, 1);
    munit_assert_int((int)micro_err_stk[0].err, ==, (int)MICRO_ERROR_EXPECTED_END_KW);

    mc_instrgen_deinit(&ig);
    sct_vector_deinit(&toks);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_instrgen_err_unexpected_token(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t toks;
    mc_instrgen_t ig;
    ig_gen("start\n", &toks, &ig);

    munit_assert_size(micro_err_stk_size, ==, 1);
    munit_assert_int((int)micro_err_stk[0].err, ==, (int)MICRO_ERROR_UNEXPECTED_TOKEN);

    mc_instrgen_deinit(&ig);
    sct_vector_deinit(&toks);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_instrgen_err_unexpected_end_kw(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t toks;
    mc_instrgen_t ig;
    ig_gen("end\n", &toks, &ig);

    munit_assert_size(micro_err_stk_size, ==, 1);
    munit_assert_int((int)micro_err_stk[0].err, ==, (int)MICRO_ERROR_UNEXPECTED_END_KW);

    mc_instrgen_deinit(&ig);
    sct_vector_deinit(&toks);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_instrgen_fun_ret_no_args(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t toks;
    mc_instrgen_t ig;
    ig_gen("fun get_five\n"
           "    ret i32\n"
           "start\n"
           "    ret 5;\n"
           "end\n", &toks, &ig);

    munit_assert_size(micro_err_stk_size, ==, 0);
    munit_assert_size(ig.instructions.size, ==, 1);

    micro_instruction_t *fun = ig_instr(&ig, 0);
    munit_assert_int((int)fun->type, ==, (int)MICRO_INSTR_FUN);
    munit_assert_string_equal(fun->fun.name, "get_five");
    munit_assert_int((int)fun->fun.ret_type, ==, (int)MICRO_TYPE_I32);
    munit_assert_size(fun->fun.args.size, ==, 0);
    munit_assert_size(fun->fun.body.size, ==, 1);

    micro_instruction_t *ret = ig_body_instr(fun, 0);
    munit_assert_int((int)ret->type, ==, (int)MICRO_INSTR_RET);
    munit_assert_ptr_not_null(ret->ret.val_expr);
    munit_assert_int((int)ret->ret.val_expr->type, ==, (int)MC_TOK_LIT_INT);
    munit_assert_string_equal(ret->ret.val_expr->val, "5");

    mc_instrgen_deinit(&ig);
    sct_vector_deinit(&toks);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_instrgen_fun_arg_types(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t toks;
    mc_instrgen_t ig;
    ig_gen("fun f\n"
           "    i8 a\n"
           "    u8 b\n"
           "    i16 c\n"
           "    u16 d\n"
           "    i32 e\n"
           "    u32 f\n"
           "    f32 g\n"
           "    ptr h\n"
           "    ret i32\n"
           "start\n"
           "end\n", &toks, &ig);

    munit_assert_size(micro_err_stk_size, ==, 0);
    munit_assert_size(ig.instructions.size, ==, 1);

    micro_instruction_t *fun = ig_instr(&ig, 0);
    munit_assert_int((int)fun->type, ==, (int)MICRO_INSTR_FUN);
    munit_assert_size(fun->fun.args.size, ==, 8);
    munit_assert_int((int)fun->fun.ret_type, ==, (int)MICRO_TYPE_I32);

    const micro_type_t expect_types[] = {
        MICRO_TYPE_I8, MICRO_TYPE_U8, MICRO_TYPE_I16, MICRO_TYPE_U16,
        MICRO_TYPE_I32, MICRO_TYPE_U32, MICRO_TYPE_F32, MICRO_TYPE_PTR
    };
    const char *expect_names[] = { "a", "b", "c", "d", "e", "f", "g", "h" };

    for (size_t i = 0; i < 8; i++) {
        micro_instruction_fun_arg_t *arg = sct_vector_get(&fun->fun.args, i);
        munit_assert_ptr_not_null(arg);
        munit_assert_int((int)arg->type, ==, (int)expect_types[i]);
        munit_assert_string_equal(arg->name, expect_names[i]);
    }

    mc_instrgen_deinit(&ig);
    sct_vector_deinit(&toks);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_instrgen_set_all_types(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t toks;
    mc_instrgen_t ig;
    ig_gen("fun f\n"
           "start\n"
           "    set i8 a 1;\n"
           "    set u8 b 1;\n"
           "    set i16 c 1;\n"
           "    set u16 d 1;\n"
           "    set i32 e 1;\n"
           "    set u32 f 1;\n"
           "    set f32 g 1;\n"
           "    set ptr h 1;\n"
           "end\n", &toks, &ig);

    munit_assert_size(micro_err_stk_size, ==, 0);
    munit_assert_size(ig.instructions.size, ==, 1);

    micro_instruction_t *fun = ig_instr(&ig, 0);
    munit_assert_size(fun->fun.body.size, ==, 8);

    const micro_type_t expect_types[] = {
        MICRO_TYPE_I8, MICRO_TYPE_U8, MICRO_TYPE_I16, MICRO_TYPE_U16,
        MICRO_TYPE_I32, MICRO_TYPE_U32, MICRO_TYPE_F32, MICRO_TYPE_PTR
    };
    const char *expect_names[] = { "a", "b", "c", "d", "e", "f", "g", "h" };

    for (size_t i = 0; i < 8; i++) {
        micro_instruction_t *set = ig_body_instr(fun, i);
        munit_assert_int((int)set->type, ==, (int)MICRO_INSTR_SET);
        munit_assert_int((int)set->set.type, ==, (int)expect_types[i]);
        munit_assert_string_equal(set->set.reg_name, expect_names[i]);
        munit_assert_ptr_not_null(set->set.val_expr);
        munit_assert_int((int)set->set.val_expr->type, ==, (int)MC_TOK_LIT_INT);
    }

    mc_instrgen_deinit(&ig);
    sct_vector_deinit(&toks);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_instrgen_drset_no_val(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t toks;
    mc_instrgen_t ig;
    ig_gen("fun f\n"
           "start\n"
           "    set i32 t 1;\n"
           "    set i32 $p;\n"
           "end\n", &toks, &ig);

    munit_assert_size(micro_err_stk_size, ==, 0);

    micro_instruction_t *drset = ig_body_instr(ig_instr(&ig, 0), 1);
    munit_assert_int((int)drset->type, ==, (int)MICRO_INSTR_DRSET);
    munit_assert_int((int)drset->drset.type, ==, (int)MICRO_TYPE_I32);
    munit_assert_string_equal(drset->drset.reg_name, "p");
    munit_assert_ptr_null(drset->drset.val_expr);

    mc_instrgen_deinit(&ig);
    sct_vector_deinit(&toks);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_instrgen_ret_vreg(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t toks;
    mc_instrgen_t ig;
    ig_gen("fun f\n"
           "    ret i32\n"
           "start\n"
           "    set i32 a 1;\n"
           "    ret a;\n"
           "end\n", &toks, &ig);

    munit_assert_size(micro_err_stk_size, ==, 0);

    micro_instruction_t *ret = ig_body_instr(ig_instr(&ig, 0), 1);
    munit_assert_int((int)ret->type, ==, (int)MICRO_INSTR_RET);
    munit_assert_ptr_not_null(ret->ret.val_expr);
    munit_assert_int((int)ret->ret.val_expr->type, ==, (int)MC_TOK_IDENT);
    munit_assert_string_equal(ret->ret.val_expr->val, "a");

    mc_instrgen_deinit(&ig);
    sct_vector_deinit(&toks);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_instrgen_call_multi_args(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t toks;
    mc_instrgen_t ig;
    ig_gen("fun f\n"
           "start\n"
           "    set i32 a 1;\n"
           "    call res add a 2 3;\n"
           "end\n", &toks, &ig);

    munit_assert_size(micro_err_stk_size, ==, 0);

    micro_instruction_t *call = ig_body_instr(ig_instr(&ig, 0), 1);
    munit_assert_int((int)call->type, ==, (int)MICRO_INSTR_CALL);
    munit_assert_string_equal(call->call.ret_reg_name, "res");
    munit_assert_string_equal(call->call.fun_name, "add");
    munit_assert_size(call->call.arg_exprs.size, ==, 3);

    mc_token_t **arg0 = sct_vector_get(&call->call.arg_exprs, 0);
    munit_assert_int((int)(*arg0)->type, ==, (int)MC_TOK_IDENT);
    munit_assert_string_equal((*arg0)->val, "a");

    mc_token_t **arg1 = sct_vector_get(&call->call.arg_exprs, 1);
    munit_assert_int((int)(*arg1)->type, ==, (int)MC_TOK_LIT_INT);
    munit_assert_string_equal((*arg1)->val, "2");

    mc_token_t **arg2 = sct_vector_get(&call->call.arg_exprs, 2);
    munit_assert_int((int)(*arg2)->type, ==, (int)MC_TOK_LIT_INT);
    munit_assert_string_equal((*arg2)->val, "3");

    mc_instrgen_deinit(&ig);
    sct_vector_deinit(&toks);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_instrgen_multiple_funs(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t toks;
    mc_instrgen_t ig;
    ig_gen("fun a\n"
           "    i8 x\n"
           "start\n"
           "    ret;\n"
           "end\n"
           "fun b\n"
           "    ret i32\n"
           "start\n"
           "    ret 1;\n"
           "end\n", &toks, &ig);

    munit_assert_size(micro_err_stk_size, ==, 0);
    munit_assert_size(ig.instructions.size, ==, 2);

    micro_instruction_t *fa = ig_instr(&ig, 0);
    munit_assert_int((int)fa->type, ==, (int)MICRO_INSTR_FUN);
    munit_assert_string_equal(fa->fun.name, "a");
    munit_assert_size(fa->fun.args.size, ==, 1);
    munit_assert_size(fa->fun.body.size, ==, 1);

    micro_instruction_t *fb = ig_instr(&ig, 1);
    munit_assert_int((int)fb->type, ==, (int)MICRO_INSTR_FUN);
    munit_assert_string_equal(fb->fun.name, "b");
    munit_assert_int((int)fb->fun.ret_type, ==, (int)MICRO_TYPE_I32);
    munit_assert_size(fb->fun.args.size, ==, 0);
    munit_assert_size(fb->fun.body.size, ==, 1);

    mc_instrgen_deinit(&ig);
    sct_vector_deinit(&toks);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_instrgen_err_expected_arg_name(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t toks;
    mc_instrgen_t ig;
    ig_gen("fun a\n"
           "    i32\n"
           "start\n"
           "end\n", &toks, &ig);

    munit_assert_size(micro_err_stk_size, ==, 1);
    munit_assert_int((int)micro_err_stk[0].err, ==, (int)MICRO_ERROR_EXPECTED_ARG_NAME);

    mc_instrgen_deinit(&ig);
    sct_vector_deinit(&toks);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_instrgen_err_expected_label_name(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t toks;
    mc_instrgen_t ig;
    ig_gen("fun a\n"
           "start\n"
           "    goto ;\n"
           "end\n", &toks, &ig);

    munit_assert_size(micro_err_stk_size, ==, 1);
    munit_assert_int((int)micro_err_stk[0].err, ==, (int)MICRO_ERROR_EXPECTED_LABEL_NAME);

    mc_instrgen_deinit(&ig);
    sct_vector_deinit(&toks);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_instrgen_err_expected_colon(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t toks;
    mc_instrgen_t ig;
    ig_gen("fun a\n"
           "start\n"
           "    mylbl\n"
           "end\n", &toks, &ig);

    munit_assert_size(micro_err_stk_size, ==, 1);
    munit_assert_int((int)micro_err_stk[0].err, ==, (int)MICRO_ERROR_EXPECTED_COLON);

    mc_instrgen_deinit(&ig);
    sct_vector_deinit(&toks);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_instrgen_err_expected_result_reg(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t toks;
    mc_instrgen_t ig;
    ig_gen("fun a\n"
           "start\n"
           "    call 5 f;\n"
           "end\n", &toks, &ig);

    munit_assert_size(micro_err_stk_size, ==, 1);
    munit_assert_int((int)micro_err_stk[0].err, ==, (int)MICRO_ERROR_EXPECTED_RESULT_REG);

    mc_instrgen_deinit(&ig);
    sct_vector_deinit(&toks);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_instrgen_err_expected_fun_name_call(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t toks;
    mc_instrgen_t ig;
    ig_gen("fun a\n"
           "start\n"
           "    call _ 5;\n"
           "end\n", &toks, &ig);

    munit_assert_size(micro_err_stk_size, ==, 1);
    munit_assert_int((int)micro_err_stk[0].err, ==, (int)MICRO_ERROR_EXPECTED_FUN_NAME);

    mc_instrgen_deinit(&ig);
    sct_vector_deinit(&toks);

    micro_deinit();

    return MUNIT_OK;
}

static MunitTest instrgen_tests[] = {
    { "/type_str_parse", test_instrgen_type_str_parse, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/fun_empty", test_instrgen_fun_empty, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/fun_args_ret", test_instrgen_fun_args_ret, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/fun_ret_no_args", test_instrgen_fun_ret_no_args, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/fun_arg_types", test_instrgen_fun_arg_types, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/set_no_val", test_instrgen_set_no_val, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/set_lit", test_instrgen_set_lit, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/set_expr", test_instrgen_set_expr, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/set_all_types", test_instrgen_set_all_types, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/drset", test_instrgen_drset, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/drset_no_val", test_instrgen_drset_no_val, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/ret_with_val", test_instrgen_ret_with_val, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/ret_no_val", test_instrgen_ret_no_val, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/ret_vreg", test_instrgen_ret_vreg, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/call", test_instrgen_call, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/call_no_args", test_instrgen_call_no_args, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/call_multi_args", test_instrgen_call_multi_args, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/lbl_goto", test_instrgen_lbl_goto, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/if_vreg", test_instrgen_if_vreg, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/if_cmp", test_instrgen_if_cmp, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/if_not", test_instrgen_if_not, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/err_if_expected_expression", test_instrgen_err_if_expected_expression, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/err_if_expected_colon", test_instrgen_err_if_expected_colon, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/err_if_expected_label_name", test_instrgen_err_if_expected_label_name, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/multiple_funs", test_instrgen_multiple_funs, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/err_set_outside_fun", test_instrgen_err_set_outside_fun, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/err_ret_outside_fun", test_instrgen_err_ret_outside_fun, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/err_call_outside_fun", test_instrgen_err_call_outside_fun, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/err_goto_outside_fun", test_instrgen_err_goto_outside_fun, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/err_lbl_outside_fun", test_instrgen_err_lbl_outside_fun, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/err_fun_inside_fun", test_instrgen_err_fun_inside_fun, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/err_expected_type_name", test_instrgen_err_expected_type_name, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/err_expected_vreg_name", test_instrgen_err_expected_vreg_name, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/err_expected_expression", test_instrgen_err_expected_expression, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/err_expected_semicolon", test_instrgen_err_expected_semicolon, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/err_expected_fun_name", test_instrgen_err_expected_fun_name, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/err_expected_arg_type", test_instrgen_err_expected_arg_type, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/err_expected_ret_type", test_instrgen_err_expected_ret_type, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/err_expected_start", test_instrgen_err_expected_start, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/err_expected_end", test_instrgen_err_expected_end, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/err_unexpected_token", test_instrgen_err_unexpected_token, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/err_unexpected_end_kw", test_instrgen_err_unexpected_end_kw, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/err_expected_arg_name", test_instrgen_err_expected_arg_name, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/err_expected_label_name", test_instrgen_err_expected_label_name, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/err_expected_colon", test_instrgen_err_expected_colon, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/err_expected_result_reg", test_instrgen_err_expected_result_reg, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/err_expected_fun_name_call", test_instrgen_err_expected_fun_name_call, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite instrgen_suite = {
    "/instrgen", instrgen_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};

#endif