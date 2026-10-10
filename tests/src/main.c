#include "types.h"
#include "asm386.h"
#include "lexer.h"
#include "instrgen.h"
#include "codegen386.h"
#include "iropter.h"

static MunitSuite all_suites[] =  {
    types_suite,
    asm386_suite,
    lexer_suite,
    instrgen_suite,
    codegen386_suite,
    iropter_suite,
    { NULL, NULL, NULL, 0, MUNIT_SUITE_OPTION_NONE }
};

static const MunitSuite main_suite = {
    "", NULL, all_suites, 1, MUNIT_SUITE_OPTION_NONE
};

int main(int argc, char **argv)
{
    return munit_suite_main(&main_suite, NULL, argc, argv);
}