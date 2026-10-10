#ifndef TESTS_ASM386_H
#define TESTS_ASM386_H

#include "../include/munit.h"
#include <micro/asm/asm386.h>
#include <string.h>

#define AS_REG(R) (micro_asm386_instruction_operand_t){ .type = MICRO_ASM386_INSTR_OPERAND_REG, .size = MICRO_SIZE_32, .reg = (R) }
#define AS_IMM(V) (micro_asm386_instruction_operand_t){ .type = MICRO_ASM386_INSTR_OPERAND_IMM, .size = MICRO_SIZE_32, .imm = { .val = (V) } }
#define AS_LBL(N) (micro_asm386_instruction_operand_t){ .type = MICRO_ASM386_INSTR_OPERAND_LBL, .size = MICRO_SIZE_32, .lbl_name = (N) }

#define AS_INSTR(OP, O1, O2) (micro_asm386_instruction_t){ \
        .opcode = (OP), \
        .operand1 = (O1), \
        .operand2 = (O2), \
    }

static void asm_push(sct_vector_t *instrs, micro_asm386_instruction_t instr)
{
    sct_vector_push(instrs, &instr);
}

static micro_asm386_instruction_t *asm_at(sct_vector_t *instrs, size_t i)
{
    return sct_vector_get(instrs, i);
}

static void asm_assert_mov(sct_vector_t *instrs, size_t i, micro_asm386_reg_t dst, micro_asm386_reg_t src)
{
    micro_asm386_instruction_t *instr = asm_at(instrs, i);
    munit_assert_ptr_not_null(instr);
    munit_assert_int((int)instr->opcode, ==, (int)MICRO_ASM386_INSTR_MOV_R32R32);
    munit_assert_int((int)instr->operand1.type, ==, (int)MICRO_ASM386_INSTR_OPERAND_REG);
    munit_assert_int((int)instr->operand1.reg, ==, (int)dst);
    munit_assert_int((int)instr->operand2.type, ==, (int)MICRO_ASM386_INSTR_OPERAND_REG);
    munit_assert_int((int)instr->operand2.reg, ==, (int)src);
}

MunitResult test_asm386_optimize_swap_dead(const MunitParameter params[], void *data)
{
    /* mov a, b ; mov b, a with both registers untouched afterwards is nothing */
    sct_vector_t instrs;
    sct_vector_init(&instrs, sizeof(micro_asm386_instruction_t));

    asm_push(&instrs, AS_INSTR(MICRO_ASM386_INSTR_PRELUDE, AS_IMM(0), AS_IMM(0)));
    asm_push(&instrs, AS_INSTR(MICRO_ASM386_INSTR_MOV_R32R32, AS_REG(MICRO_ASM386_REG32_EAX), AS_REG(MICRO_ASM386_REG32_ECX)));
    asm_push(&instrs, AS_INSTR(MICRO_ASM386_INSTR_MOV_R32R32, AS_REG(MICRO_ASM386_REG32_ECX), AS_REG(MICRO_ASM386_REG32_EAX)));
    asm_push(&instrs, AS_INSTR(MICRO_ASM386_INSTR_ADD_R32I32, AS_REG(MICRO_ASM386_REG32_EDI), AS_IMM(1)));
    asm_push(&instrs, AS_INSTR(MICRO_ASM386_INSTR_EPILOGUE, AS_IMM(0), AS_IMM(0)));

    micro_asm386_optimize(&instrs);

    munit_assert_size(instrs.size, ==, 3);
    munit_assert_int((int)asm_at(&instrs, 0)->opcode, ==, (int)MICRO_ASM386_INSTR_PRELUDE);
    munit_assert_int((int)asm_at(&instrs, 1)->opcode, ==, (int)MICRO_ASM386_INSTR_ADD_R32I32);
    munit_assert_int((int)asm_at(&instrs, 2)->opcode, ==, (int)MICRO_ASM386_INSTR_EPILOGUE);

    sct_vector_deinit(&instrs);

    return MUNIT_OK;
}

MunitResult test_asm386_optimize_swap_keeps_a(const MunitParameter params[], void *data)
{
    /* the swap only copies into a, so the second mov goes away even when a lives */
    sct_vector_t instrs;
    sct_vector_init(&instrs, sizeof(micro_asm386_instruction_t));

    asm_push(&instrs, AS_INSTR(MICRO_ASM386_INSTR_PRELUDE, AS_IMM(0), AS_IMM(0)));
    asm_push(&instrs, AS_INSTR(MICRO_ASM386_INSTR_MOV_R32R32, AS_REG(MICRO_ASM386_REG32_EAX), AS_REG(MICRO_ASM386_REG32_ECX)));
    asm_push(&instrs, AS_INSTR(MICRO_ASM386_INSTR_MOV_R32R32, AS_REG(MICRO_ASM386_REG32_ECX), AS_REG(MICRO_ASM386_REG32_EAX)));
    asm_push(&instrs, AS_INSTR(MICRO_ASM386_INSTR_ADD_R32I32, AS_REG(MICRO_ASM386_REG32_EAX), AS_IMM(1)));
    asm_push(&instrs, AS_INSTR(MICRO_ASM386_INSTR_EPILOGUE, AS_IMM(0), AS_IMM(0)));

    micro_asm386_optimize(&instrs);

    munit_assert_size(instrs.size, ==, 4);
    asm_assert_mov(&instrs, 1, MICRO_ASM386_REG32_EAX, MICRO_ASM386_REG32_ECX);
    munit_assert_int((int)asm_at(&instrs, 2)->opcode, ==, (int)MICRO_ASM386_INSTR_INC_R32);
    munit_assert_int((int)asm_at(&instrs, 3)->opcode, ==, (int)MICRO_ASM386_INSTR_EPILOGUE);

    sct_vector_deinit(&instrs);

    return MUNIT_OK;
}

MunitResult test_asm386_optimize_mov_merge(const MunitParameter params[], void *data)
{
    /* mov a, s ; mov d, a with a dead becomes one mov d, s */
    sct_vector_t instrs;
    sct_vector_init(&instrs, sizeof(micro_asm386_instruction_t));

    asm_push(&instrs, AS_INSTR(MICRO_ASM386_INSTR_PRELUDE, AS_IMM(0), AS_IMM(0)));
    asm_push(&instrs, AS_INSTR(MICRO_ASM386_INSTR_MOV_R32R32, AS_REG(MICRO_ASM386_REG32_EAX), AS_REG(MICRO_ASM386_REG32_ECX)));
    asm_push(&instrs, AS_INSTR(MICRO_ASM386_INSTR_MOV_R32R32, AS_REG(MICRO_ASM386_REG32_EDX), AS_REG(MICRO_ASM386_REG32_EAX)));
    asm_push(&instrs, AS_INSTR(MICRO_ASM386_INSTR_EPILOGUE, AS_IMM(0), AS_IMM(0)));

    micro_asm386_optimize(&instrs);

    munit_assert_size(instrs.size, ==, 3);
    asm_assert_mov(&instrs, 1, MICRO_ASM386_REG32_EDX, MICRO_ASM386_REG32_ECX);

    sct_vector_deinit(&instrs);

    return MUNIT_OK;
}

MunitResult test_asm386_optimize_mov_merge_a_live(const MunitParameter params[], void *data)
{
    /* a is read after the pair, so the value of a has to survive */
    sct_vector_t instrs;
    sct_vector_init(&instrs, sizeof(micro_asm386_instruction_t));

    asm_push(&instrs, AS_INSTR(MICRO_ASM386_INSTR_PRELUDE, AS_IMM(0), AS_IMM(0)));
    asm_push(&instrs, AS_INSTR(MICRO_ASM386_INSTR_MOV_R32R32, AS_REG(MICRO_ASM386_REG32_EAX), AS_REG(MICRO_ASM386_REG32_ECX)));
    asm_push(&instrs, AS_INSTR(MICRO_ASM386_INSTR_MOV_R32R32, AS_REG(MICRO_ASM386_REG32_EDX), AS_REG(MICRO_ASM386_REG32_EAX)));
    asm_push(&instrs, AS_INSTR(MICRO_ASM386_INSTR_ADD_R32I32, AS_REG(MICRO_ASM386_REG32_EAX), AS_IMM(2)));
    asm_push(&instrs, AS_INSTR(MICRO_ASM386_INSTR_EPILOGUE, AS_IMM(0), AS_IMM(0)));

    micro_asm386_optimize(&instrs);

    munit_assert_size(instrs.size, ==, 5);
    asm_assert_mov(&instrs, 1, MICRO_ASM386_REG32_EAX, MICRO_ASM386_REG32_ECX);
    asm_assert_mov(&instrs, 2, MICRO_ASM386_REG32_EDX, MICRO_ASM386_REG32_EAX);

    sct_vector_deinit(&instrs);

    return MUNIT_OK;
}

MunitResult test_asm386_optimize_mov_def_stops_scan(const MunitParameter params[], void *data)
{
    /* a is redefined before it is read, the earlier reads do not keep it alive */
    sct_vector_t instrs;
    sct_vector_init(&instrs, sizeof(micro_asm386_instruction_t));

    asm_push(&instrs, AS_INSTR(MICRO_ASM386_INSTR_PRELUDE, AS_IMM(0), AS_IMM(0)));
    asm_push(&instrs, AS_INSTR(MICRO_ASM386_INSTR_MOV_R32R32, AS_REG(MICRO_ASM386_REG32_EAX), AS_REG(MICRO_ASM386_REG32_ECX)));
    asm_push(&instrs, AS_INSTR(MICRO_ASM386_INSTR_MOV_R32R32, AS_REG(MICRO_ASM386_REG32_ECX), AS_REG(MICRO_ASM386_REG32_EAX)));
    asm_push(&instrs, AS_INSTR(MICRO_ASM386_INSTR_MOV_R32I32, AS_REG(MICRO_ASM386_REG32_EAX), AS_IMM(7)));
    asm_push(&instrs, AS_INSTR(MICRO_ASM386_INSTR_ADD_R32I32, AS_REG(MICRO_ASM386_REG32_EDX), AS_IMM(1)));
    asm_push(&instrs, AS_INSTR(MICRO_ASM386_INSTR_EPILOGUE, AS_IMM(0), AS_IMM(0)));

    micro_asm386_optimize(&instrs);

    /* the pair is gone, eax is loaded with 7 right away */
    munit_assert_size(instrs.size, ==, 4);
    munit_assert_int((int)asm_at(&instrs, 1)->opcode, ==, (int)MICRO_ASM386_INSTR_MOV_R32I32);

    sct_vector_deinit(&instrs);

    return MUNIT_OK;
}

MunitResult test_asm386_optimize_mov_dead_in_next_fun(const MunitParameter params[], void *data)
{
    /* the scan of one function must not walk into the next one */
    sct_vector_t instrs;
    sct_vector_init(&instrs, sizeof(micro_asm386_instruction_t));

    asm_push(&instrs, AS_INSTR(MICRO_ASM386_INSTR_PRELUDE, AS_IMM(0), AS_IMM(0)));
    asm_push(&instrs, AS_INSTR(MICRO_ASM386_INSTR_MOV_R32R32, AS_REG(MICRO_ASM386_REG32_EAX), AS_REG(MICRO_ASM386_REG32_ECX)));
    asm_push(&instrs, AS_INSTR(MICRO_ASM386_INSTR_MOV_R32R32, AS_REG(MICRO_ASM386_REG32_ECX), AS_REG(MICRO_ASM386_REG32_EAX)));
    asm_push(&instrs, AS_INSTR(MICRO_ASM386_INSTR_EPILOGUE, AS_IMM(0), AS_IMM(0)));
    asm_push(&instrs, AS_INSTR(MICRO_ASM386_INSTR_PRELUDE, AS_IMM(0), AS_IMM(0)));
    asm_push(&instrs, AS_INSTR(MICRO_ASM386_INSTR_ADD_R32I32, AS_REG(MICRO_ASM386_REG32_EAX), AS_IMM(1)));
    asm_push(&instrs, AS_INSTR(MICRO_ASM386_INSTR_EPILOGUE, AS_IMM(0), AS_IMM(0)));

    micro_asm386_optimize(&instrs);

    munit_assert_size(instrs.size, ==, 5);
    munit_assert_int((int)asm_at(&instrs, 1)->opcode, ==, (int)MICRO_ASM386_INSTR_EPILOGUE);
    munit_assert_int((int)asm_at(&instrs, 3)->opcode, ==, (int)MICRO_ASM386_INSTR_INC_R32);

    sct_vector_deinit(&instrs);

    return MUNIT_OK;
}

MunitResult test_asm386_optimize_swap_with_back_edge(const MunitParameter params[], void *data)
{
    /* a jump behind the pair reaches the loop head, but a swap only copies into
       a, so the second mov goes away even when a is read again there */
    sct_vector_t instrs;
    sct_vector_init(&instrs, sizeof(micro_asm386_instruction_t));

    asm_push(&instrs, AS_INSTR(MICRO_ASM386_INSTR_LBL, AS_LBL("top"), AS_IMM(0)));
    asm_push(&instrs, AS_INSTR(MICRO_ASM386_INSTR_ADD_R32I32, AS_REG(MICRO_ASM386_REG32_EAX), AS_IMM(1)));
    asm_push(&instrs, AS_INSTR(MICRO_ASM386_INSTR_CMP_R32R32, AS_REG(MICRO_ASM386_REG32_ECX), AS_REG(MICRO_ASM386_REG32_EDX)));
    asm_push(&instrs, AS_INSTR(MICRO_ASM386_INSTR_MOV_R32R32, AS_REG(MICRO_ASM386_REG32_EAX), AS_REG(MICRO_ASM386_REG32_ECX)));
    asm_push(&instrs, AS_INSTR(MICRO_ASM386_INSTR_MOV_R32R32, AS_REG(MICRO_ASM386_REG32_ECX), AS_REG(MICRO_ASM386_REG32_EAX)));
    asm_push(&instrs, AS_INSTR(MICRO_ASM386_INSTR_JL_L32, AS_LBL("top"), AS_IMM(0)));
    asm_push(&instrs, AS_INSTR(MICRO_ASM386_INSTR_EPILOGUE, AS_IMM(0), AS_IMM(0)));

    micro_asm386_optimize(&instrs);

    munit_assert_size(instrs.size, ==, 6);
    munit_assert_int((int)asm_at(&instrs, 1)->opcode, ==, (int)MICRO_ASM386_INSTR_INC_R32);
    asm_assert_mov(&instrs, 3, MICRO_ASM386_REG32_EAX, MICRO_ASM386_REG32_ECX);
    munit_assert_int((int)asm_at(&instrs, 4)->opcode, ==, (int)MICRO_ASM386_INSTR_JL_L32);

    sct_vector_deinit(&instrs);

    return MUNIT_OK;
}

MunitResult test_asm386_optimize_merge_with_back_edge(const MunitParameter params[], void *data)
{
    /* the same loop, but the pair copies into another register: the jump hides
       a read of a behind it, so the two movs have to stay */
    sct_vector_t instrs;
    sct_vector_init(&instrs, sizeof(micro_asm386_instruction_t));

    asm_push(&instrs, AS_INSTR(MICRO_ASM386_INSTR_LBL, AS_LBL("top"), AS_IMM(0)));
    asm_push(&instrs, AS_INSTR(MICRO_ASM386_INSTR_ADD_R32I32, AS_REG(MICRO_ASM386_REG32_EAX), AS_IMM(1)));
    asm_push(&instrs, AS_INSTR(MICRO_ASM386_INSTR_CMP_R32R32, AS_REG(MICRO_ASM386_REG32_ECX), AS_REG(MICRO_ASM386_REG32_EDX)));
    asm_push(&instrs, AS_INSTR(MICRO_ASM386_INSTR_MOV_R32R32, AS_REG(MICRO_ASM386_REG32_EAX), AS_REG(MICRO_ASM386_REG32_ECX)));
    asm_push(&instrs, AS_INSTR(MICRO_ASM386_INSTR_MOV_R32R32, AS_REG(MICRO_ASM386_REG32_EDX), AS_REG(MICRO_ASM386_REG32_EAX)));
    asm_push(&instrs, AS_INSTR(MICRO_ASM386_INSTR_JL_L32, AS_LBL("top"), AS_IMM(0)));
    asm_push(&instrs, AS_INSTR(MICRO_ASM386_INSTR_EPILOGUE, AS_IMM(0), AS_IMM(0)));

    micro_asm386_optimize(&instrs);

    munit_assert_size(instrs.size, ==, 7);
    asm_assert_mov(&instrs, 3, MICRO_ASM386_REG32_EAX, MICRO_ASM386_REG32_ECX);
    asm_assert_mov(&instrs, 4, MICRO_ASM386_REG32_EDX, MICRO_ASM386_REG32_EAX);

    sct_vector_deinit(&instrs);

    return MUNIT_OK;
}

MunitResult test_asm386_optimize_mov_live_through_lea(const MunitParameter params[], void *data)
{
    /* a register inside the address of a lea is read, it keeps the pair alive */
    sct_vector_t instrs;
    sct_vector_init(&instrs, sizeof(micro_asm386_instruction_t));

    asm_push(&instrs, AS_INSTR(MICRO_ASM386_INSTR_PRELUDE, AS_IMM(0), AS_IMM(0)));
    asm_push(&instrs, AS_INSTR(MICRO_ASM386_INSTR_MOV_R32R32, AS_REG(MICRO_ASM386_REG32_EAX), AS_REG(MICRO_ASM386_REG32_ECX)));
    asm_push(&instrs, AS_INSTR(MICRO_ASM386_INSTR_MOV_R32R32, AS_REG(MICRO_ASM386_REG32_EDX), AS_REG(MICRO_ASM386_REG32_EAX)));
    asm_push(&instrs, (micro_asm386_instruction_t){
        .opcode = MICRO_ASM386_INSTR_LEA_R32SIB,
        .operand1 = AS_REG(MICRO_ASM386_REG32_EBX),
        .operand2 = { .type = MICRO_ASM386_INSTR_OPERAND_NONE },
        .sib = { .scale = 0, .index = MICRO_ASM386_REG32_NO_INDEX, .base = MICRO_ASM386_REG32_EAX },
    });
    asm_push(&instrs, AS_INSTR(MICRO_ASM386_INSTR_EPILOGUE, AS_IMM(0), AS_IMM(0)));

    micro_asm386_optimize(&instrs);

    munit_assert_size(instrs.size, ==, 5);
    asm_assert_mov(&instrs, 1, MICRO_ASM386_REG32_EAX, MICRO_ASM386_REG32_ECX);
    asm_assert_mov(&instrs, 2, MICRO_ASM386_REG32_EDX, MICRO_ASM386_REG32_EAX);

    sct_vector_deinit(&instrs);

    return MUNIT_OK;
}

MunitResult test_asm386_optimize_mov_immediate_source(const MunitParameter params[], void *data)
{
    /* the source is not a register, there is nothing to merge into */
    sct_vector_t instrs;
    sct_vector_init(&instrs, sizeof(micro_asm386_instruction_t));

    asm_push(&instrs, AS_INSTR(MICRO_ASM386_INSTR_PRELUDE, AS_IMM(0), AS_IMM(0)));
    asm_push(&instrs, AS_INSTR(MICRO_ASM386_INSTR_MOV_R32I32, AS_REG(MICRO_ASM386_REG32_EAX), AS_IMM(5)));
    asm_push(&instrs, AS_INSTR(MICRO_ASM386_INSTR_MOV_R32R32, AS_REG(MICRO_ASM386_REG32_EDX), AS_REG(MICRO_ASM386_REG32_EAX)));
    asm_push(&instrs, AS_INSTR(MICRO_ASM386_INSTR_EPILOGUE, AS_IMM(0), AS_IMM(0)));

    micro_asm386_optimize(&instrs);

    munit_assert_size(instrs.size, ==, 4);

    sct_vector_deinit(&instrs);

    return MUNIT_OK;
}

static MunitTest asm386_tests[] = {
    { "/optimize_swap_dead", test_asm386_optimize_swap_dead, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/optimize_swap_keeps_a", test_asm386_optimize_swap_keeps_a, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/optimize_mov_merge", test_asm386_optimize_mov_merge, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/optimize_mov_merge_a_live", test_asm386_optimize_mov_merge_a_live, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/optimize_mov_def_stops_scan", test_asm386_optimize_mov_def_stops_scan, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/optimize_mov_dead_in_next_fun", test_asm386_optimize_mov_dead_in_next_fun, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/optimize_swap_with_back_edge", test_asm386_optimize_swap_with_back_edge, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/optimize_merge_with_back_edge", test_asm386_optimize_merge_with_back_edge, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/optimize_mov_live_through_lea", test_asm386_optimize_mov_live_through_lea, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { "/optimize_mov_immediate_source", test_asm386_optimize_mov_immediate_source, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite asm386_suite = {
    "/asm386", asm386_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};

#endif