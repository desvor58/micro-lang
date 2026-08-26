#include <micro/micro.h>

int main()
{
    micro_init();
        sct_vector_t instrs;
        sct_vector_init(&instrs, sizeof(micro_instruction_t));

        sct_vector_t expr;
        sct_vector_init(&expr, sizeof(micro_token_t));

        sct_vector_

        micro_instrgen_gen_set(&instrs, MICRO_TYPE_I32, "test_vreg", &expr);
    micro_deinit();
}