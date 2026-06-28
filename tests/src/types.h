#ifndef TESTS_TYPES_H
#define TESTS_TYPES_H

#include "../include/munit.h"
#include <SCT/vector.h>
#include <SCT/list.h>
#include <stdio.h>

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

MunitResult test_flat_vector_struct(const MunitParameter params[], void *data)
{
    sct_vector_t vec;
    micro_flat_vector_init(&vec, sizeof(my_vec_t), MICRO_FLAT_VECTOR_STD_EXT_SZ);

    munit_assert_ptr_not_null(vec.data);
    munit_assert_size(vec.__extend_size, ==, MICRO_FLAT_VECTOR_STD_EXT_SZ);
    munit_assert_size(vec.__type_size, ==, sizeof(my_vec_t));
    munit_assert_size(vec.real_size, ==, MICRO_FLAT_VECTOR_STD_EXT_SZ);
    munit_assert_size(vec.size, ==, 0);

    for (size_t i = 0; i < 200; i++) {
        for (size_t j = 0; j < i; j++) {
            my_vec_t *Jstruct = micro_flat_vector_get(&vec, j);
            munit_assert_int(Jstruct->a, ==, 68);
            munit_assert_size(Jstruct->b, ==, j);
            munit_assert_string_equal(Jstruct->s, "hello");
        }

        munit_assert_size(vec.size, ==, i);

        micro_flat_vector_push(&vec, &(my_vec_t){
            .a = 68,
            .b = i,
            .s = "hello"
        });

        munit_assert_size(vec.size, ==, i + 1);

        for (size_t j = 0; j < i + 1; j++) {
            my_vec_t *Jstruct = micro_flat_vector_get(&vec, j);
            munit_assert_int(Jstruct->a, ==, 68);
            munit_assert_size(Jstruct->b, ==, j);
            munit_assert_string_equal(Jstruct->s, "hello");
        }
    }

    micro_flat_vector_deinit(&vec);

    return MUNIT_OK;
}

MunitResult test_plist_int(const MunitParameter params[], void *data)
{
    micro_plist_t *list = micro_plist_init();

    munit_assert_ptr_not_null(list);
    munit_assert_ptr_null(list->next);
    munit_assert_ptr_null(list->data);

    for (size_t i = 0; i < 200; i++) {
        for (size_t j = 0; j < i; j++) {
            int *Jint = micro_plist_get(list, j);
            munit_assert_ptr_not_null(Jint);
            munit_assert_int(*Jint, ==, j);
        }

        munit_assert_size(micro_plist_size(list), ==, i);

        int *mi = malloc(sizeof(int));
        *mi = i;

        munit_assert_int(micro_plist_push(list, mi), ==, 0);

        munit_assert_size(micro_plist_size(list), ==, i + 1);

        for (size_t j = 0; j < i + 1; j++) {
            int *Jint = micro_plist_get(list, j);
            munit_assert_ptr_not_null(Jint);
            munit_assert_int(*Jint, ==, j);
        }
    }

    micro_plist_deinit(list, 1);

    return MUNIT_OK;
}

typedef struct {
    int a;
    size_t b;
    char s[16];
} my_list_t;

MunitResult test_plist_struct(const MunitParameter params[], void *data)
{
    micro_plist_t *list = micro_plist_init();

    munit_assert_ptr_not_null(list);
    munit_assert_ptr_null(list->next);
    munit_assert_ptr_null(list->data);

    for (size_t i = 0; i < 200; i++) {
        for (size_t j = 0; j < i; j++) {
            my_list_t *Jstruct = micro_plist_get(list, j);
            munit_assert_ptr_not_null(Jstruct);
            munit_assert_int(Jstruct->a, ==, 68);
            munit_assert_size(Jstruct->b, ==, j);
            munit_assert_string_equal(Jstruct->s, "hello");
        }

        munit_assert_size(micro_plist_size(list), ==, i);

        my_list_t *ms = malloc(sizeof(my_list_t));
        *ms = (my_list_t){
            .a = 68,
            .b = i,
            .s = "hello",
        };

        munit_assert_int(micro_plist_push(list, ms), ==, 0);

        munit_assert_size(micro_plist_size(list), ==, i + 1);

        for (size_t j = 0; j < i + 1; j++) {
            my_list_t *Jstruct = micro_plist_get(list, j);
            munit_assert_ptr_not_null(Jstruct);
            munit_assert_int(Jstruct->a, ==, 68);
            munit_assert_size(Jstruct->b, ==, j);
            munit_assert_string_equal(Jstruct->s, "hello");
        }
    }

    micro_plist_deinit(list, 1);

    return MUNIT_OK;
}


static MunitTest types_tests[] = {
    { "/vector/int", test_vector_int, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/flat_vector/struct", test_flat_vector_struct, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/plist/int", test_plist_int, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/plist/struct", test_plist_struct, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite types_suite = {
    "/types", types_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};

#endif