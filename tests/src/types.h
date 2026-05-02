#ifndef TESTS_TYPES_H
#define TESTS_TYPES_H

#include "../include/munit.h"
#include <micro/types/flat_vector.h>
#include <micro/types/plist.h>
#include <stdio.h>

MunitResult test_flat_vector_int(const MunitParameter params[], void *data)
{
    micro_flat_vector_t vec;
    micro_flat_vector_init(&vec, sizeof(int), MICRO_FLAT_VECTOR_STD_EXT_SZ);

    munit_assert_ptr_not_null(vec.data);
    munit_assert_size(vec.__extend_size, ==, MICRO_FLAT_VECTOR_STD_EXT_SZ);
    munit_assert_size(vec.__type_size, ==, sizeof(int));
    munit_assert_size(vec.real_size, ==, MICRO_FLAT_VECTOR_STD_EXT_SZ);
    munit_assert_size(vec.size, ==, 0);

    for (size_t i = 0; i < 200; i++) {
        for (size_t j = 0; j < i; j++) {
            int *Jint = micro_flat_vector_get(&vec, j);
            munit_assert_int(*Jint, ==, j);
        }

        munit_assert_size(vec.size, ==, i);

        micro_flat_vector_push(&vec, &i);

        munit_assert_size(vec.size, ==, i + 1);

        for (size_t j = 0; j < i + 1; j++) {
            int *Jint = micro_flat_vector_get(&vec, j);
            munit_assert_int(*Jint, ==, j);
        }
    }

    return MUNIT_OK;
}

typedef struct {
    int a;
    size_t b;
    char s[16];
} my_t;

MunitResult test_flat_vector_struct(const MunitParameter params[], void *data)
{
    micro_flat_vector_t vec;
    micro_flat_vector_init(&vec, sizeof(my_t), MICRO_FLAT_VECTOR_STD_EXT_SZ);

    munit_assert_ptr_not_null(vec.data);
    munit_assert_size(vec.__extend_size, ==, MICRO_FLAT_VECTOR_STD_EXT_SZ);
    munit_assert_size(vec.__type_size, ==, sizeof(my_t));
    munit_assert_size(vec.real_size, ==, MICRO_FLAT_VECTOR_STD_EXT_SZ);
    munit_assert_size(vec.size, ==, 0);

    for (size_t i = 0; i < 200; i++) {
        for (size_t j = 0; j < i; j++) {
            my_t *Jstruct = micro_flat_vector_get(&vec, j);
            munit_assert_int(Jstruct->a, ==, 68);
            munit_assert_size(Jstruct->b, ==, j);
            munit_assert_string_equal(Jstruct->s, "hello");
        }

        munit_assert_size(vec.size, ==, i);

        micro_flat_vector_push(&vec, &(my_t){
            .a = 68,
            .b = i,
            .s = "hello"
        });

        munit_assert_size(vec.size, ==, i + 1);

        for (size_t j = 0; j < i + 1; j++) {
            my_t *Jstruct = micro_flat_vector_get(&vec, j);
            munit_assert_int(Jstruct->a, ==, 68);
            munit_assert_size(Jstruct->b, ==, j);
            munit_assert_string_equal(Jstruct->s, "hello");
        }
    }

    return MUNIT_OK;
}


static MunitTest types_tests[] = {
    { "/types/flat_vector/int", test_flat_vector_int, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/types/flat_vector/struct", test_flat_vector_struct, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite types_suite = {
    "/types", types_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};

#endif