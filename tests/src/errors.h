#ifndef TESTS_ERRORS_H
#define TESTS_ERRORS_H

#include <micro/common.h>
#include <stdio.h>

/* Error messages, mirrors the `err_str[]` table in src/microc/microc.c */
static const char *test_err_str[] = {
    [MICRO_ERROR_NONE]                      = "",

    [MICRO_ERROR_EXPECTED_COMMENT_CLOSE]    = "Expected comment close",
    [MICRO_ERROR_EXPECTED_STRING_CLOSE]     = "Expected string close",

    [MICRO_ERROR_EXPECTED_EXPRESSION]       = "Expected expression",
    [MICRO_ERROR_EXPECTED_SEMICOLON]        = "Expected ';'",
    [MICRO_ERROR_EXPECTED_TYPE_NAME]        = "Expected type name",
    [MICRO_ERROR_UNDEFINED_TYPE_NAME]       = "Undefined type name",
    [MICRO_ERROR_UNEXPECTED_TOKEN]          = "Unexpected token",
    [MICRO_ERROR_UNEXPECTED_END_KW]         = "Unexpected 'end' keyword",

    [MICRO_ERROR_EXPECTED_SET_KW]           = "Expected 'set' keyword",
    [MICRO_ERROR_EXPECTED_FUN_KW]           = "Expected 'fun' keyword",
    [MICRO_ERROR_EXPECTED_RET_KW]           = "Expected 'ret' keyword",
    [MICRO_ERROR_EXPECTED_CALL_KW]          = "Expected 'call' keyword",
    [MICRO_ERROR_EXPECTED_GOTO_KW]          = "Expected 'goto' keyword",
    [MICRO_ERROR_EXPECTED_START_KW]         = "Expected 'start' keyword",
    [MICRO_ERROR_EXPECTED_END_KW]           = "Expected 'end' keyword",

    [MICRO_ERROR_EXPECTED_FUN_NAME]         = "Expected function name",
    [MICRO_ERROR_EXPECTED_ARG_TYPE]         = "Expected argument type",
    [MICRO_ERROR_EXPECTED_ARG_NAME]         = "Expected argument name",
    [MICRO_ERROR_EXPECTED_RET_TYPE]         = "Expected return type",
    [MICRO_ERROR_EXPECTED_VREG_NAME]        = "Expected vreg name",
    [MICRO_ERROR_EXPECTED_RESULT_REG]       = "Expected result register",
    [MICRO_ERROR_EXPECTED_LABEL_NAME]       = "Expected label name",
    [MICRO_ERROR_EXPECTED_COLON]            = "Expected ':'",

    [MICRO_ERROR_SET_OUTSIDE_FUNCTION]      = "'set' can be only in function body",
    [MICRO_ERROR_FUN_INSIDE_FUNCTION]       = "'fun' can be only outside function",
    [MICRO_ERROR_RET_OUTSIDE_FUNCTION]      = "'ret' can be only in function body",
    [MICRO_ERROR_CALL_OUTSIDE_FUNCTION]     = "'call' can be only in function body",
    [MICRO_ERROR_GOTO_OUTSIDE_FUNCTION]     = "'goto' can be only in function body",
    [MICRO_ERROR_LBL_OUTSIDE_FUNCTION]      = "'lbl' can be only in function body",

    [MICRO_ERROR_UNDEFINED_IDENT]           = "Undefined identifier",
    [MICRO_ERROR_UNDEFINED_FUN]             = "Undefined function",
    [MICRO_ERROR_EXPECTED_VREG_RESULT]      = "Expected vreg as call result",
    [MICRO_ERROR_RESULT_TYPE_MISMATCH]      = "Call result type mismatch",
    [MICRO_ERROR_TOO_FEW_ARGS]              = "Too few arguments in call",
    [MICRO_ERROR_TOO_MANY_ARGS]             = "Too many arguments in call",
    [MICRO_ERROR_VREG_TYPE_MISMATCH]        = "Type mismatch with declared vreg",
    [MICRO_ERROR_EXPR_PARSE]                = "Expression parse error",

    [MICRO_ERROR_IDENT_NOT_VREG]            = "Identifier is not a virtual register",
    [MICRO_ERROR_IDENT_NOT_LBL]             = "Identifier is not a label",

    [MICRO_ERROR_UNDEFINED_LBL]             = "Undefined label",
};

/* Print a single error exactly like microc's put_err() */
static void test_put_err(const char *file, micro_error_t err)
{
    fprintf(stderr, "Error: %s:%lu:%lu: %s\n", file,
            err.line_ref, err.chpos_ref, test_err_str[err.err]);
}

/* Print all accumulated errors from the stack, if any */
static void test_put_errors(const char *file)
{
    for (size_t i = 0; i < micro_err_stk_size; i++) {
        test_put_err(file, micro_err_stk[i]);
    }
}

#endif