#ifndef TESTS_LEXER_H
#define TESTS_LEXER_H

#include "../include/munit.h"
#include <micro/lexer.h>
#include <stdio.h>

MunitResult test_lexer_null_in(const MunitParameter params[], void *data)
{
    micro_init();

    micro_tok_vec_t *toks = malloc(sizeof(micro_tok_vec_t));
    micro_tok_vec_init(toks);
    munit_assert_size(toks->size, ==, 0);
    
    micro_tokenize("", 0, toks);
    munit_assert_size(toks->size, ==, 0);
    munit_assert_size(micro_err_stk_size, ==, 0);

    micro_tok_vec_deinit(toks);

    micro_deinit();

    return MUNIT_OK;
}

MunitResult test_lexer_base_test1(const MunitParameter params[], void *data)
{
    micro_init();

    micro_tok_vec_t *toks = malloc(sizeof(micro_tok_vec_t));
    micro_tok_vec_init(toks);
    munit_assert_size(toks->size, ==, 0);
    
    const char *text = "fun myf\n"
                      "    i8 a\n"
                      "    i16 b\n"
                      "    i32 c\n"
                      "    ret ptr\n"
                      "start\n"
                      "    var i32 h = + 6 4 - 3 * 2 / 4 & 2 # 5 $ 2 ! 4 ~ 6;\n"
                      "    call _ myf 2 3 -4;\n"
                      "    set h 5;\n"
                      "    if h : lbl;\n"
                      "    goto lbl2;\n"
                      "end\n";
                    
    micro_tok_vec_t *right_toks = malloc(sizeof(micro_tok_vec_t));
    micro_tok_vec_init(right_toks);

    micro_token_t t;

    // line 1
    micro_push_tok(right_toks, (micro_token_t){ .type = MICRO_TT_KW_FUN, .line_ref = 1, .chpos_ref = 1 });
    t = (micro_token_t){ .type = MICRO_TT_IDENT, .line_ref = 1, .chpos_ref = 5 }; strcpy(t.val, "myf"); micro_push_tok(right_toks, t);

    // line 2
    t = (micro_token_t){ .type = MICRO_TT_TYPE_NAME, .line_ref = 2, .chpos_ref = 5 }; strcpy(t.val, "i8"); micro_push_tok(right_toks, t);
    t = (micro_token_t){ .type = MICRO_TT_IDENT, .line_ref = 2, .chpos_ref = 8 }; strcpy(t.val, "a"); micro_push_tok(right_toks, t);

    // line 3
    t = (micro_token_t){ .type = MICRO_TT_TYPE_NAME, .line_ref = 3, .chpos_ref = 5 }; strcpy(t.val, "i16"); micro_push_tok(right_toks, t);
    t = (micro_token_t){ .type = MICRO_TT_IDENT, .line_ref = 3, .chpos_ref = 9 }; strcpy(t.val, "b"); micro_push_tok(right_toks, t);

    // line 4
    t = (micro_token_t){ .type = MICRO_TT_TYPE_NAME, .line_ref = 4, .chpos_ref = 5 }; strcpy(t.val, "i32"); micro_push_tok(right_toks, t);
    t = (micro_token_t){ .type = MICRO_TT_IDENT, .line_ref = 4, .chpos_ref = 9 }; strcpy(t.val, "c"); micro_push_tok(right_toks, t);

    // line 5
    micro_push_tok(right_toks, (micro_token_t){ .type = MICRO_TT_KW_RET, .line_ref = 5, .chpos_ref = 5 });
    t = (micro_token_t){ .type = MICRO_TT_TYPE_NAME, .line_ref = 5, .chpos_ref = 9 }; strcpy(t.val, "ptr"); micro_push_tok(right_toks, t);

    // line 6
    micro_push_tok(right_toks, (micro_token_t){ .type = MICRO_TT_KW_START, .line_ref = 6, .chpos_ref = 1 });

    // line 7
    micro_push_tok(right_toks, (micro_token_t){ .type = MICRO_TT_KW_VAR, .line_ref = 7, .chpos_ref = 5 });
    t = (micro_token_t){ .type = MICRO_TT_TYPE_NAME, .line_ref = 7, .chpos_ref = 9 }; strcpy(t.val, "i32"); micro_push_tok(right_toks, t);
    t = (micro_token_t){ .type = MICRO_TT_IDENT, .line_ref = 7, .chpos_ref = 13 }; strcpy(t.val, "h"); micro_push_tok(right_toks, t);
    micro_push_tok(right_toks, (micro_token_t){ .type = MICRO_TT_EQ, .line_ref = 7, .chpos_ref = 15 });
    micro_push_tok(right_toks, (micro_token_t){ .type = MICRO_TT_PLUS, .line_ref = 7, .chpos_ref = 17 });
    t = (micro_token_t){ .type = MICRO_TT_LIT_INT, .line_ref = 7, .chpos_ref = 19 }; strcpy(t.val, "6"); micro_push_tok(right_toks, t);
    t = (micro_token_t){ .type = MICRO_TT_LIT_INT, .line_ref = 7, .chpos_ref = 21 }; strcpy(t.val, "4"); micro_push_tok(right_toks, t);
    micro_push_tok(right_toks, (micro_token_t){ .type = MICRO_TT_MINUS, .line_ref = 7, .chpos_ref = 23 });
    t = (micro_token_t){ .type = MICRO_TT_LIT_INT, .line_ref = 7, .chpos_ref = 25 }; strcpy(t.val, "3"); micro_push_tok(right_toks, t);
    micro_push_tok(right_toks, (micro_token_t){ .type = MICRO_TT_STAR, .line_ref = 7, .chpos_ref = 27 });
    t = (micro_token_t){ .type = MICRO_TT_LIT_INT, .line_ref = 7, .chpos_ref = 29 }; strcpy(t.val, "2"); micro_push_tok(right_toks, t);
    micro_push_tok(right_toks, (micro_token_t){ .type = MICRO_TT_SLASH, .line_ref = 7, .chpos_ref = 31 });
    t = (micro_token_t){ .type = MICRO_TT_LIT_INT, .line_ref = 7, .chpos_ref = 33 }; strcpy(t.val, "4"); micro_push_tok(right_toks, t);
    micro_push_tok(right_toks, (micro_token_t){ .type = MICRO_TT_AMPERSAND, .line_ref = 7, .chpos_ref = 35 });
    t = (micro_token_t){ .type = MICRO_TT_LIT_INT, .line_ref = 7, .chpos_ref = 37 }; strcpy(t.val, "2"); micro_push_tok(right_toks, t);
    micro_push_tok(right_toks, (micro_token_t){ .type = MICRO_TT_HASH, .line_ref = 7, .chpos_ref = 39 });
    t = (micro_token_t){ .type = MICRO_TT_LIT_INT, .line_ref = 7, .chpos_ref = 41 }; strcpy(t.val, "5"); micro_push_tok(right_toks, t);
    micro_push_tok(right_toks, (micro_token_t){ .type = MICRO_TT_DOLLAR, .line_ref = 7, .chpos_ref = 43 });
    t = (micro_token_t){ .type = MICRO_TT_LIT_INT, .line_ref = 7, .chpos_ref = 45 }; strcpy(t.val, "2"); micro_push_tok(right_toks, t);
    micro_push_tok(right_toks, (micro_token_t){ .type = MICRO_TT_EXCLAMATION, .line_ref = 7, .chpos_ref = 47 });
    t = (micro_token_t){ .type = MICRO_TT_LIT_INT, .line_ref = 7, .chpos_ref = 49 }; strcpy(t.val, "4"); micro_push_tok(right_toks, t);
    micro_push_tok(right_toks, (micro_token_t){ .type = MICRO_TT_TILDE, .line_ref = 7, .chpos_ref = 51 });
    t = (micro_token_t){ .type = MICRO_TT_LIT_INT, .line_ref = 7, .chpos_ref = 53 }; strcpy(t.val, "6"); micro_push_tok(right_toks, t);
    micro_push_tok(right_toks, (micro_token_t){ .type = MICRO_TT_SEMICOLON, .line_ref = 7, .chpos_ref = 54 });

    // line 8
    micro_push_tok(right_toks, (micro_token_t){ .type = MICRO_TT_KW_CALL, .line_ref = 8, .chpos_ref = 5 });
    t = (micro_token_t){ .type = MICRO_TT_IDENT, .line_ref = 8, .chpos_ref = 10 }; strcpy(t.val, "_"); micro_push_tok(right_toks, t);
    t = (micro_token_t){ .type = MICRO_TT_IDENT, .line_ref = 8, .chpos_ref = 12 }; strcpy(t.val, "myf"); micro_push_tok(right_toks, t);
    t = (micro_token_t){ .type = MICRO_TT_LIT_INT, .line_ref = 8, .chpos_ref = 16 }; strcpy(t.val, "2"); micro_push_tok(right_toks, t);
    t = (micro_token_t){ .type = MICRO_TT_LIT_INT, .line_ref = 8, .chpos_ref = 18 }; strcpy(t.val, "3"); micro_push_tok(right_toks, t);
    t = (micro_token_t){ .type = MICRO_TT_LIT_INT, .line_ref = 8, .chpos_ref = 20 }; strcpy(t.val, "-4"); micro_push_tok(right_toks, t);
    micro_push_tok(right_toks, (micro_token_t){ .type = MICRO_TT_SEMICOLON, .line_ref = 8, .chpos_ref = 23 });

    // line 9
    micro_push_tok(right_toks, (micro_token_t){ .type = MICRO_TT_KW_SET, .line_ref = 9, .chpos_ref = 5 });
    t = (micro_token_t){ .type = MICRO_TT_IDENT, .line_ref = 9, .chpos_ref = 9 }; strcpy(t.val, "h"); micro_push_tok(right_toks, t);
    t = (micro_token_t){ .type = MICRO_TT_LIT_INT, .line_ref = 9, .chpos_ref = 11 }; strcpy(t.val, "5"); micro_push_tok(right_toks, t);
    micro_push_tok(right_toks, (micro_token_t){ .type = MICRO_TT_SEMICOLON, .line_ref = 9, .chpos_ref = 12 });

    // line 10
    micro_push_tok(right_toks, (micro_token_t){ .type = MICRO_TT_KW_IF, .line_ref = 10, .chpos_ref = 5 });
    t = (micro_token_t){ .type = MICRO_TT_IDENT, .line_ref = 10, .chpos_ref = 8 }; strcpy(t.val, "h"); micro_push_tok(right_toks, t);
    micro_push_tok(right_toks, (micro_token_t){ .type = MICRO_TT_COLON, .line_ref = 10, .chpos_ref = 10 });
    t = (micro_token_t){ .type = MICRO_TT_IDENT, .line_ref = 10, .chpos_ref = 12 }; strcpy(t.val, "lbl"); micro_push_tok(right_toks, t);
    micro_push_tok(right_toks, (micro_token_t){ .type = MICRO_TT_SEMICOLON, .line_ref = 10, .chpos_ref = 15 });

    // line 11
    micro_push_tok(right_toks, (micro_token_t){ .type = MICRO_TT_KW_GOTO, .line_ref = 11, .chpos_ref = 5 });
    t = (micro_token_t){ .type = MICRO_TT_IDENT, .line_ref = 11, .chpos_ref = 10 }; strcpy(t.val, "lbl2"); micro_push_tok(right_toks, t);
    micro_push_tok(right_toks, (micro_token_t){ .type = MICRO_TT_SEMICOLON, .line_ref = 11, .chpos_ref = 14 });

    // line 12
    micro_push_tok(right_toks, (micro_token_t){ .type = MICRO_TT_KW_END, .line_ref = 12, .chpos_ref = 1 });
    
    micro_tokenize(text, strlen(text), toks);
    munit_assert_size(toks->size, ==, right_toks->size);
    munit_assert_size(micro_err_stk_size, ==, 0);

    for (size_t i = 0; i < right_toks->size; i++) {
        munit_assert_int((int)toks->toks[i].type, ==, (int)right_toks->toks[i].type);
        munit_assert_string_equal(toks->toks[i].val, right_toks->toks[i].val);
        munit_assert_size(toks->toks[i].line_ref, ==, right_toks->toks[i].line_ref);
        munit_assert_size(toks->toks[i].chpos_ref, ==, right_toks->toks[i].chpos_ref);
    }

    micro_tok_vec_deinit(toks);

    micro_deinit();

    return MUNIT_OK;
}


static MunitTest lexer_tests[] = {
    { "/null_in", test_lexer_null_in, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/base1", test_lexer_base_test1, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite lexer_suite = {
    "/lexer", lexer_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};

#endif