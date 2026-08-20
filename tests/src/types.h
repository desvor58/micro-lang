#ifndef TESTS_TYPES_H
#define TESTS_TYPES_H

#include "../include/munit.h"
#include <SCT/vector.h>
#include <SCT/list.h>
#include <stdio.h>
#include <string.h>

MunitResult test_vector_int(const MunitParameter params[], void *data)
{
    sct_vector_t vec;
    sct_vector_init(&vec, sizeof(int));

    munit_assert_ptr_not_null(vec.data);
    munit_assert_size(vec.cap, ==, SCT_VECTOR_ALLOC_SIZE);
    munit_assert_size(vec.size, ==, 0);
    munit_assert_size(vec._item_size, ==, sizeof(int));

    for (size_t i = 0; i < 200; i++) {
        for (size_t j = 0; j < i; j++) {
            int *Jint = sct_vector_get(&vec, j);
            munit_assert_int(*Jint, ==, j);
        }

        munit_assert_size(vec.size, ==, i);

        sct_vector_push(&vec, &i);

        munit_assert_size(vec.size, ==, i + 1);

        for (size_t j = 0; j < i + 1; j++) {
            int *Jint = sct_vector_get(&vec, j);
            munit_assert_int(*Jint, ==, j);
        }
    }

    sct_vector_deinit(&vec);

    return MUNIT_OK;
}

typedef struct {
    int a;
    size_t b;
    char s[16];
} my_vec_t;

MunitResult test_vector_struct(const MunitParameter params[], void *data)
{
    sct_vector_t vec;
    sct_vector_init(&vec, sizeof(my_vec_t));

    munit_assert_ptr_not_null(vec.data);
    munit_assert_size(vec.size, ==, 0);
    munit_assert_size(vec._item_size, ==, sizeof(my_vec_t));

    for (size_t i = 0; i < 200; i++) {
        for (size_t j = 0; j < i; j++) {
            my_vec_t *Jstruct = sct_vector_get(&vec, j);
            munit_assert_int(Jstruct->a, ==, 68);
            munit_assert_size(Jstruct->b, ==, j);
            munit_assert_string_equal(Jstruct->s, "hello");
        }

        munit_assert_size(vec.size, ==, i);

        sct_vector_push(&vec, &(my_vec_t){
            .a = 68,
            .b = i,
            .s = "hello"
        });

        munit_assert_size(vec.size, ==, i + 1);

        for (size_t j = 0; j < i + 1; j++) {
            my_vec_t *Jstruct = sct_vector_get(&vec, j);
            munit_assert_int(Jstruct->a, ==, 68);
            munit_assert_size(Jstruct->b, ==, j);
            munit_assert_string_equal(Jstruct->s, "hello");
        }
    }

    sct_vector_deinit(&vec);

    return MUNIT_OK;
}

MunitResult test_list_int(const MunitParameter params[], void *data)
{
    sct_list_t list;
    sct_list_init(&list, sizeof(int));

    munit_assert_size(list.size, ==, 0);
    munit_assert_size(list._item_size, ==, sizeof(int));

    for (size_t i = 0; i < 200; i++) {
        for (size_t j = 0; j < i; j++) {
            int *Jint = sct_list_get(&list, j);
            munit_assert_ptr_not_null(Jint);
            munit_assert_int(*Jint, ==, j);
        }

        munit_assert_size(list.size, ==, i);

        sct_list_push(&list, &i);

        munit_assert_size(list.size, ==, i + 1);

        for (size_t j = 0; j < i + 1; j++) {
            int *Jint = sct_list_get(&list, j);
            munit_assert_ptr_not_null(Jint);
            munit_assert_int(*Jint, ==, j);
        }
    }

    sct_list_deinit(&list);

    return MUNIT_OK;
}

typedef struct {
    int a;
    size_t b;
    char s[16];
} my_list_t;

MunitResult test_list_struct(const MunitParameter params[], void *data)
{
    sct_list_t list;
    sct_list_init(&list, sizeof(my_list_t));

    munit_assert_size(list.size, ==, 0);
    munit_assert_size(list._item_size, ==, sizeof(my_list_t));

    for (size_t i = 0; i < 200; i++) {
        for (size_t j = 0; j < i; j++) {
            my_list_t *Jstruct = sct_list_get(&list, j);
            munit_assert_ptr_not_null(Jstruct);
            munit_assert_int(Jstruct->a, ==, 68);
            munit_assert_size(Jstruct->b, ==, j);
            munit_assert_string_equal(Jstruct->s, "hello");
        }

        munit_assert_size(list.size, ==, i);

        sct_list_push(&list, &(my_list_t){
            .a = 68,
            .b = i,
            .s = "hello"
        });

        munit_assert_size(list.size, ==, i + 1);

        for (size_t j = 0; j < i + 1; j++) {
            my_list_t *Jstruct = sct_list_get(&list, j);
            munit_assert_ptr_not_null(Jstruct);
            munit_assert_int(Jstruct->a, ==, 68);
            munit_assert_size(Jstruct->b, ==, j);
            munit_assert_string_equal(Jstruct->s, "hello");
        }
    }

    sct_list_deinit(&list);

    return MUNIT_OK;
}

MunitResult test_list_erase(const MunitParameter params[], void *data)
{
    sct_list_t list;
    sct_list_init(&list, sizeof(int));

    for (size_t i = 0; i < 10; i++) {
        sct_list_push(&list, &i);
    }

    sct_list_erase(&list, 0);
    munit_assert_size(list.size, ==, 9);
    munit_assert_int(*(int*)sct_list_get(&list, 0), ==, 1);

    sct_list_erase(&list, 5);
    munit_assert_size(list.size, ==, 8);
    munit_assert_int(*(int*)sct_list_get(&list, 5), ==, 7);

    sct_list_deinit(&list);

    return MUNIT_OK;
}


static MunitTest types_tests[] = {
    { "/vector/int", test_vector_int, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/vector/struct", test_vector_struct, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/list/int", test_list_int, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/list/struct", test_list_struct, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/list/erase", test_list_erase, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite types_suite = {
    "/types", types_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};

#endif