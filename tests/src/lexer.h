#ifndef TESTS_LEXER_H
#define TESTS_LEXER_H

#include "../include/munit.h"
#include "errors.h"
#include <microc/lexer.h>
#include <stdio.h>
#include <string.h>

static void lexer_assert_tok(sct_vector_t *toks, size_t i, mc_token_type_t type,
                             const char *val, size_t line_ref, size_t chpos_ref)
{
    mc_token_t *tok = sct_vector_get(toks, i);
    munit_assert_ptr_not_null(tok);
    munit_assert_int((int)tok->type, ==, (int)type);
    munit_assert_string_equal(tok->val, val);
    munit_assert_size(tok->line_ref, ==, line_ref);
    munit_assert_size(tok->chpos_ref, ==, chpos_ref);
}

MunitResult test_lexer_empty_in(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t toks;
    sct_vector_init(&toks, sizeof(mc_token_t));

    const char *text = "";
    mc_tokenize(text, strlen(text), &toks);
    munit_assert_size(toks.size, ==, 0);
    munit_assert_size(micro_err_stk_size, ==, 0);

    sct_vector_deinit(&toks);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_lexer_whitespace_in(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t toks;
    sct_vector_init(&toks, sizeof(mc_token_t));

    const char *text = "   \t\n  \t\n";
    mc_tokenize(text, strlen(text), &toks);
    munit_assert_size(toks.size, ==, 0);
    munit_assert_size(micro_err_stk_size, ==, 0);

    sct_vector_deinit(&toks);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_lexer_keywords(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t toks;
    sct_vector_init(&toks, sizeof(mc_token_t));

    const char *text = "fun set if else while start end ret call goto";
    mc_tokenize(text, strlen(text), &toks);
    munit_assert_size(toks.size, ==, 10);
    munit_assert_size(micro_err_stk_size, ==, 0);

    lexer_assert_tok(&toks, 0, MC_TOK_KW_FUN, "", 1, 1);
    lexer_assert_tok(&toks, 1, MC_TOK_KW_SET, "", 1, 5);
    lexer_assert_tok(&toks, 2, MC_TOK_KW_IF, "", 1, 9);
    lexer_assert_tok(&toks, 3, MC_TOK_KW_ELSE, "", 1, 12);
    lexer_assert_tok(&toks, 4, MC_TOK_KW_WHILE, "", 1, 17);
    lexer_assert_tok(&toks, 5, MC_TOK_KW_START, "", 1, 23);
    lexer_assert_tok(&toks, 6, MC_TOK_KW_END, "", 1, 29);
    lexer_assert_tok(&toks, 7, MC_TOK_KW_RET, "", 1, 33);
    lexer_assert_tok(&toks, 8, MC_TOK_KW_CALL, "", 1, 37);
    lexer_assert_tok(&toks, 9, MC_TOK_KW_GOTO, "", 1, 42);

    sct_vector_deinit(&toks);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_lexer_type_names(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t toks;
    sct_vector_init(&toks, sizeof(mc_token_t));

    const char *text = "i8 u8 i16 u16 i32 u32 f32 ptr";
    mc_tokenize(text, strlen(text), &toks);
    munit_assert_size(toks.size, ==, 8);
    munit_assert_size(micro_err_stk_size, ==, 0);

    lexer_assert_tok(&toks, 0, MC_TOK_TYPE_NAME, "i8", 1, 1);
    lexer_assert_tok(&toks, 1, MC_TOK_TYPE_NAME, "u8", 1, 4);
    lexer_assert_tok(&toks, 2, MC_TOK_TYPE_NAME, "i16", 1, 7);
    lexer_assert_tok(&toks, 3, MC_TOK_TYPE_NAME, "u16", 1, 11);
    lexer_assert_tok(&toks, 4, MC_TOK_TYPE_NAME, "i32", 1, 15);
    lexer_assert_tok(&toks, 5, MC_TOK_TYPE_NAME, "u32", 1, 19);
    lexer_assert_tok(&toks, 6, MC_TOK_TYPE_NAME, "f32", 1, 23);
    lexer_assert_tok(&toks, 7, MC_TOK_TYPE_NAME, "ptr", 1, 27);

    sct_vector_deinit(&toks);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_lexer_idents(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t toks;
    sct_vector_init(&toks, sizeof(mc_token_t));

    const char *text = "foo _bar my_var2 a1";
    mc_tokenize(text, strlen(text), &toks);
    munit_assert_size(toks.size, ==, 4);
    munit_assert_size(micro_err_stk_size, ==, 0);

    lexer_assert_tok(&toks, 0, MC_TOK_IDENT, "foo", 1, 1);
    lexer_assert_tok(&toks, 1, MC_TOK_IDENT, "_bar", 1, 5);
    lexer_assert_tok(&toks, 2, MC_TOK_IDENT, "my_var2", 1, 10);
    lexer_assert_tok(&toks, 3, MC_TOK_IDENT, "a1", 1, 18);

    sct_vector_deinit(&toks);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_lexer_lit_int(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t toks;
    sct_vector_init(&toks, sizeof(mc_token_t));

    const char *text = "5 -3 0 2147483647 -2147483648";
    mc_tokenize(text, strlen(text), &toks);
    munit_assert_size(toks.size, ==, 5);
    munit_assert_size(micro_err_stk_size, ==, 0);

    lexer_assert_tok(&toks, 0, MC_TOK_LIT_INT, "5", 1, 1);
    lexer_assert_tok(&toks, 1, MC_TOK_LIT_INT, "-3", 1, 3);
    lexer_assert_tok(&toks, 2, MC_TOK_LIT_INT, "0", 1, 6);
    lexer_assert_tok(&toks, 3, MC_TOK_LIT_INT, "2147483647", 1, 8);
    lexer_assert_tok(&toks, 4, MC_TOK_LIT_INT, "-2147483648", 1, 19);

    sct_vector_deinit(&toks);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_lexer_lit_float(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t toks;
    sct_vector_init(&toks, sizeof(mc_token_t));

    const char *text = "2.5 -3.14 0.01 1.0";
    mc_tokenize(text, strlen(text), &toks);
    munit_assert_size(toks.size, ==, 4);
    munit_assert_size(micro_err_stk_size, ==, 0);

    lexer_assert_tok(&toks, 0, MC_TOK_LIT_FLOAT, "2.5", 1, 1);
    lexer_assert_tok(&toks, 1, MC_TOK_LIT_FLOAT, "-3.14", 1, 5);
    lexer_assert_tok(&toks, 2, MC_TOK_LIT_FLOAT, "0.01", 1, 11);
    lexer_assert_tok(&toks, 3, MC_TOK_LIT_FLOAT, "1.0", 1, 16);

    sct_vector_deinit(&toks);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_lexer_lit_str(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t toks;
    sct_vector_init(&toks, sizeof(mc_token_t));

    const char *text = "\"hello, world\"";
    mc_tokenize(text, strlen(text), &toks);
    munit_assert_size(toks.size, ==, 1);
    munit_assert_size(micro_err_stk_size, ==, 0);

    lexer_assert_tok(&toks, 0, MC_TOK_LIT_STR, "hello, world", 1, 1);

    sct_vector_deinit(&toks);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_lexer_lit_str_empty(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t toks;
    sct_vector_init(&toks, sizeof(mc_token_t));

    const char *text = "\"\"";
    mc_tokenize(text, strlen(text), &toks);
    munit_assert_size(toks.size, ==, 1);
    munit_assert_size(micro_err_stk_size, ==, 0);

    lexer_assert_tok(&toks, 0, MC_TOK_LIT_STR, "", 1, 1);

    sct_vector_deinit(&toks);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_lexer_operators_single(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t toks;
    sct_vector_init(&toks, sizeof(mc_token_t));

    const char *text = "+-*/. ,:;&$#`~=!";
    mc_tokenize(text, strlen(text), &toks);
    munit_assert_size(toks.size, ==, 15);
    munit_assert_size(micro_err_stk_size, ==, 0);

    lexer_assert_tok(&toks,  0, MC_TOK_PLUS, "", 1, 1);
    lexer_assert_tok(&toks,  1, MC_TOK_MINUS, "", 1, 2);
    lexer_assert_tok(&toks,  2, MC_TOK_STAR, "", 1, 3);
    lexer_assert_tok(&toks,  3, MC_TOK_SLASH, "", 1, 4);
    lexer_assert_tok(&toks,  4, MC_TOK_DOT, "", 1, 5);
    lexer_assert_tok(&toks,  5, MC_TOK_COMA, "", 1, 7);
    lexer_assert_tok(&toks,  6, MC_TOK_COLON, "", 1, 8);
    lexer_assert_tok(&toks,  7, MC_TOK_SEMICOLON, "", 1, 9);
    lexer_assert_tok(&toks,  8, MC_TOK_AMPERSAND, "", 1, 10);
    lexer_assert_tok(&toks,  9, MC_TOK_DOLLAR, "", 1, 11);
    lexer_assert_tok(&toks, 10, MC_TOK_HASH, "", 1, 12);
    lexer_assert_tok(&toks, 11, MC_TOK_APOSTROPHE, "", 1, 13);
    lexer_assert_tok(&toks, 12, MC_TOK_TILDE, "", 1, 14);
    lexer_assert_tok(&toks, 13, MC_TOK_EQ, "", 1, 15);
    lexer_assert_tok(&toks, 14, MC_TOK_EXCLAMATION, "", 1, 16);

    sct_vector_deinit(&toks);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_lexer_operators_great_less(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t toks;
    sct_vector_init(&toks, sizeof(mc_token_t));

    const char *text = "> <";
    mc_tokenize(text, strlen(text), &toks);
    munit_assert_size(toks.size, ==, 2);
    munit_assert_size(micro_err_stk_size, ==, 0);

    lexer_assert_tok(&toks, 0, MC_TOK_GREAT, "", 1, 1);
    lexer_assert_tok(&toks, 1, MC_TOK_LESS, "", 1, 3);

    sct_vector_deinit(&toks);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_lexer_operators_great_or_eq_less_or_eq(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t toks;
    sct_vector_init(&toks, sizeof(mc_token_t));

    const char *text = ">= <=";
    mc_tokenize(text, strlen(text), &toks);
    munit_assert_size(toks.size, ==, 2);
    munit_assert_size(micro_err_stk_size, ==, 0);

    lexer_assert_tok(&toks, 0, MC_TOK_GREAT_OR_EQ, "", 1, 1);
    lexer_assert_tok(&toks, 1, MC_TOK_LESS_OR_EQ, "", 1, 4);

    sct_vector_deinit(&toks);

    micro_deinit();

    return MUNIT_OK;
}

/* The reference (docs/micro-language-ref.md) declares a full operator set
 * that includes bitwise or '|', bitwise xor '^' and the logical and/or
 * '&&'/'||'. These are not lexed yet, so these tests are marked TODO: they
 * document the intended behaviour and start passing as soon as the lexer
 * recognizes each operator as a single token. */
MunitResult test_lexer_op_pipe(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t toks;
    sct_vector_init(&toks, sizeof(mc_token_t));

    const char *text = "|";
    mc_tokenize(text, strlen(text), &toks);
    munit_assert_size(toks.size, ==, 1);

    sct_vector_deinit(&toks);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_lexer_op_caret(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t toks;
    sct_vector_init(&toks, sizeof(mc_token_t));

    const char *text = "^";
    mc_tokenize(text, strlen(text), &toks);
    munit_assert_size(toks.size, ==, 1);

    sct_vector_deinit(&toks);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_lexer_op_logic_and(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t toks;
    sct_vector_init(&toks, sizeof(mc_token_t));

    const char *text = "&&";
    mc_tokenize(text, strlen(text), &toks);
    munit_assert_size(toks.size, ==, 1);

    sct_vector_deinit(&toks);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_lexer_op_logic_or(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t toks;
    sct_vector_init(&toks, sizeof(mc_token_t));

    const char *text = "||";
    mc_tokenize(text, strlen(text), &toks);
    munit_assert_size(toks.size, ==, 1);

    sct_vector_deinit(&toks);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_lexer_line_chpos(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t toks;
    sct_vector_init(&toks, sizeof(mc_token_t));

    const char *text = "set i32 a 5;\nret a;\n";
    mc_tokenize(text, strlen(text), &toks);
    munit_assert_size(toks.size, ==, 8);
    munit_assert_size(micro_err_stk_size, ==, 0);

    lexer_assert_tok(&toks, 0, MC_TOK_KW_SET, "", 1, 1);
    lexer_assert_tok(&toks, 1, MC_TOK_TYPE_NAME, "i32", 1, 5);
    lexer_assert_tok(&toks, 2, MC_TOK_IDENT, "a", 1, 9);
    lexer_assert_tok(&toks, 3, MC_TOK_LIT_INT, "5", 1, 11);
    lexer_assert_tok(&toks, 4, MC_TOK_SEMICOLON, "", 1, 12);
    lexer_assert_tok(&toks, 5, MC_TOK_KW_RET, "", 2, 1);
    lexer_assert_tok(&toks, 6, MC_TOK_IDENT, "a", 2, 5);
    lexer_assert_tok(&toks, 7, MC_TOK_SEMICOLON, "", 2, 6);

    sct_vector_deinit(&toks);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_lexer_comment_single(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t toks;
    sct_vector_init(&toks, sizeof(mc_token_t));

    const char *text = "\\ hello \\\nset i32 a 5;";
    mc_tokenize(text, strlen(text), &toks);
    munit_assert_size(toks.size, ==, 5);
    munit_assert_size(micro_err_stk_size, ==, 0);

    lexer_assert_tok(&toks, 0, MC_TOK_KW_SET, "", 2, 1);
    lexer_assert_tok(&toks, 1, MC_TOK_TYPE_NAME, "i32", 2, 5);
    lexer_assert_tok(&toks, 2, MC_TOK_IDENT, "a", 2, 9);
    lexer_assert_tok(&toks, 3, MC_TOK_LIT_INT, "5", 2, 11);
    lexer_assert_tok(&toks, 4, MC_TOK_SEMICOLON, "", 2, 12);

    sct_vector_deinit(&toks);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_lexer_comment_multiline(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t toks;
    sct_vector_init(&toks, sizeof(mc_token_t));

    const char *text = "\\ comment\n  across lines \\\nret;";
    mc_tokenize(text, strlen(text), &toks);
    munit_assert_size(toks.size, ==, 2);
    munit_assert_size(micro_err_stk_size, ==, 0);

    lexer_assert_tok(&toks, 0, MC_TOK_KW_RET, "", 3, 1);
    lexer_assert_tok(&toks, 1, MC_TOK_SEMICOLON, "", 3, 4);

    sct_vector_deinit(&toks);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_lexer_err_unclosed_comment(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t toks;
    sct_vector_init(&toks, sizeof(mc_token_t));

    const char *text = "\\ never closed";
    mc_tokenize(text, strlen(text), &toks);
    test_put_errors("lexer");
    munit_assert_size(toks.size, ==, 0);
    munit_assert_size(micro_err_stk_size, ==, 1);
    munit_assert_int((int)micro_err_stk[0].err, ==, (int)MICRO_ERROR_EXPECTED_COMMENT_CLOSE);

    sct_vector_deinit(&toks);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_lexer_err_unclosed_string(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t toks;
    sct_vector_init(&toks, sizeof(mc_token_t));

    const char *text = "\"abc";
    mc_tokenize(text, strlen(text), &toks);
    test_put_errors("lexer");
    munit_assert_size(toks.size, ==, 0);
    munit_assert_size(micro_err_stk_size, ==, 1);
    munit_assert_int((int)micro_err_stk[0].err, ==, (int)MICRO_ERROR_EXPECTED_STRING_CLOSE);

    sct_vector_deinit(&toks);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_lexer_base1(const MunitParameter params[], void *data)
{
    micro_init();

    sct_vector_t toks;
    sct_vector_init(&toks, sizeof(mc_token_t));

    const char *text = "fun myf\n"
                      "    i8 a\n"
                      "    i16 b\n"
                      "    i32 c\n"
                      "    ret ptr\n"
                      "start\n"
                      "    set i32 h + 6 4 - 3 * 2 / 4 & 2 # 5 $ 2 ! 4 ~ 6;\n"
                      "    call _ myf 2 3 -4;\n"
                      "    set h 5;\n"
                      "    if h : lbl;\n"
                      "    goto lbl2;\n"
                      "end\n";

    mc_tokenize(text, strlen(text), &toks);
    munit_assert_size(micro_err_stk_size, ==, 0);

    // line 1
    lexer_assert_tok(&toks, 0, MC_TOK_KW_FUN, "", 1, 1);
    lexer_assert_tok(&toks, 1, MC_TOK_IDENT, "myf", 1, 5);

    // line 2
    lexer_assert_tok(&toks, 2, MC_TOK_TYPE_NAME, "i8", 2, 5);
    lexer_assert_tok(&toks, 3, MC_TOK_IDENT, "a", 2, 8);

    // line 3
    lexer_assert_tok(&toks, 4, MC_TOK_TYPE_NAME, "i16", 3, 5);
    lexer_assert_tok(&toks, 5, MC_TOK_IDENT, "b", 3, 9);

    // line 4
    lexer_assert_tok(&toks, 6, MC_TOK_TYPE_NAME, "i32", 4, 5);
    lexer_assert_tok(&toks, 7, MC_TOK_IDENT, "c", 4, 9);

    // line 5
    lexer_assert_tok(&toks, 8, MC_TOK_KW_RET, "", 5, 5);
    lexer_assert_tok(&toks, 9, MC_TOK_TYPE_NAME, "ptr", 5, 9);

    // line 6
    lexer_assert_tok(&toks, 10, MC_TOK_KW_START, "", 6, 1);

    // line 7
    lexer_assert_tok(&toks, 11, MC_TOK_KW_SET, "", 7, 5);
    lexer_assert_tok(&toks, 12, MC_TOK_TYPE_NAME, "i32", 7, 9);
    lexer_assert_tok(&toks, 13, MC_TOK_IDENT, "h", 7, 13);
    lexer_assert_tok(&toks, 14, MC_TOK_PLUS, "", 7, 15);
    lexer_assert_tok(&toks, 15, MC_TOK_LIT_INT, "6", 7, 17);
    lexer_assert_tok(&toks, 16, MC_TOK_LIT_INT, "4", 7, 19);
    lexer_assert_tok(&toks, 17, MC_TOK_MINUS, "", 7, 21);
    lexer_assert_tok(&toks, 18, MC_TOK_LIT_INT, "3", 7, 23);
    lexer_assert_tok(&toks, 19, MC_TOK_STAR, "", 7, 25);
    lexer_assert_tok(&toks, 20, MC_TOK_LIT_INT, "2", 7, 27);
    lexer_assert_tok(&toks, 21, MC_TOK_SLASH, "", 7, 29);
    lexer_assert_tok(&toks, 22, MC_TOK_LIT_INT, "4", 7, 31);
    lexer_assert_tok(&toks, 23, MC_TOK_AMPERSAND, "", 7, 33);
    lexer_assert_tok(&toks, 24, MC_TOK_LIT_INT, "2", 7, 35);
    lexer_assert_tok(&toks, 25, MC_TOK_HASH, "", 7, 37);
    lexer_assert_tok(&toks, 26, MC_TOK_LIT_INT, "5", 7, 39);
    lexer_assert_tok(&toks, 27, MC_TOK_DOLLAR, "", 7, 41);
    lexer_assert_tok(&toks, 28, MC_TOK_LIT_INT, "2", 7, 43);
    lexer_assert_tok(&toks, 29, MC_TOK_EXCLAMATION, "", 7, 45);
    lexer_assert_tok(&toks, 30, MC_TOK_LIT_INT, "4", 7, 47);
    lexer_assert_tok(&toks, 31, MC_TOK_TILDE, "", 7, 49);
    lexer_assert_tok(&toks, 32, MC_TOK_LIT_INT, "6", 7, 51);
    lexer_assert_tok(&toks, 33, MC_TOK_SEMICOLON, "", 7, 52);

    // line 8
    lexer_assert_tok(&toks, 34, MC_TOK_KW_CALL, "", 8, 5);
    lexer_assert_tok(&toks, 35, MC_TOK_IDENT, "_", 8, 10);
    lexer_assert_tok(&toks, 36, MC_TOK_IDENT, "myf", 8, 12);
    lexer_assert_tok(&toks, 37, MC_TOK_LIT_INT, "2", 8, 16);
    lexer_assert_tok(&toks, 38, MC_TOK_LIT_INT, "3", 8, 18);
    lexer_assert_tok(&toks, 39, MC_TOK_LIT_INT, "-4", 8, 20);
    lexer_assert_tok(&toks, 40, MC_TOK_SEMICOLON, "", 8, 22);

    // line 9
    lexer_assert_tok(&toks, 41, MC_TOK_KW_SET, "", 9, 5);
    lexer_assert_tok(&toks, 42, MC_TOK_IDENT, "h", 9, 9);
    lexer_assert_tok(&toks, 43, MC_TOK_LIT_INT, "5", 9, 11);
    lexer_assert_tok(&toks, 44, MC_TOK_SEMICOLON, "", 9, 12);

    // line 10
    lexer_assert_tok(&toks, 45, MC_TOK_KW_IF, "", 10, 5);
    lexer_assert_tok(&toks, 46, MC_TOK_IDENT, "h", 10, 8);
    lexer_assert_tok(&toks, 47, MC_TOK_COLON, "", 10, 10);
    lexer_assert_tok(&toks, 48, MC_TOK_IDENT, "lbl", 10, 12);
    lexer_assert_tok(&toks, 49, MC_TOK_SEMICOLON, "", 10, 15);

    // line 11
    lexer_assert_tok(&toks, 50, MC_TOK_KW_GOTO, "", 11, 5);
    lexer_assert_tok(&toks, 51, MC_TOK_IDENT, "lbl2", 11, 10);
    lexer_assert_tok(&toks, 52, MC_TOK_SEMICOLON, "", 11, 14);

    // line 12
    lexer_assert_tok(&toks, 53, MC_TOK_KW_END, "", 12, 1);

    munit_assert_size(toks.size, ==, 54);

    sct_vector_deinit(&toks);

    micro_deinit();

    return MUNIT_OK;
}

static MunitTest lexer_tests[] = {
    { "/empty_in", test_lexer_empty_in, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/whitespace_in", test_lexer_whitespace_in, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/keywords", test_lexer_keywords, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/type_names", test_lexer_type_names, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/idents", test_lexer_idents, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/lit_int", test_lexer_lit_int, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/lit_float", test_lexer_lit_float, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/lit_str", test_lexer_lit_str, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/lit_str_empty", test_lexer_lit_str_empty, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/operators_single", test_lexer_operators_single, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/operators_great_less", test_lexer_operators_great_less, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/operators_great_or_eq_less_or_eq", test_lexer_operators_great_or_eq_less_or_eq, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/op_pipe", test_lexer_op_pipe, NULL, NULL, MUNIT_TEST_OPTION_TODO, NULL },
    { "/op_caret", test_lexer_op_caret, NULL, NULL, MUNIT_TEST_OPTION_TODO, NULL },
    { "/op_logic_and", test_lexer_op_logic_and, NULL, NULL, MUNIT_TEST_OPTION_TODO, NULL },
    { "/op_logic_or", test_lexer_op_logic_or, NULL, NULL, MUNIT_TEST_OPTION_TODO, NULL },
    { "/line_chpos", test_lexer_line_chpos, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/comment_single", test_lexer_comment_single, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/comment_multiline", test_lexer_comment_multiline, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/err_unclosed_comment", test_lexer_err_unclosed_comment, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/err_unclosed_string", test_lexer_err_unclosed_string, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/base1", test_lexer_base1, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite lexer_suite = {
    "/lexer", lexer_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};

#endif