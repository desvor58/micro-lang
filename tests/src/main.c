#include "types.h"

static MunitSuite all_suites[] =  {
    types_suite,
    { NULL, NULL, NULL, 0, MUNIT_SUITE_OPTION_NONE }
};

static const MunitSuite main_suite = {
    "", NULL, all_suites, 1, MUNIT_SUITE_OPTION_NONE
};

int main(int argc, char **argv)
{
    return munit_suite_main(&main_suite, NULL, argc, argv);
}