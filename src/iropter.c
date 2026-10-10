#include <micro/iropter.h>

static size_t iropter_instrs_num(sct_vector_t *instrs)
{
    size_t num = instrs->size;

    for (size_t i = 0; i < instrs->size; i++) {
        micro_instruction_t *instr = sct_vector_get(instrs, i);
        if (instr->type == MICRO_INSTR_FUN) {
            num += iropter_instrs_num(&instr->fun.body);
        }
    }
    return num;
}

void micro_iropter_init(micro_iropter_t *iropter, sct_vector_t *instrs)
{
    iropter->instrs = instrs;
    sct_arena_init(&iropter->arena);
    sct_arena_hashmap_init(&iropter->fun_infos, &iropter->arena, sizeof(micro_iropter_fun_info_t));
    memset(iropter->call_prices, 0, sizeof(sct_arena_vector_t) * 64);
    iropter->instr_save = 0;
    iropter->instr_save_pos = 0;
    iropter->inlined_calls = 0;
    iropter->inlined_instrs = 0;
    iropter->budget = iropter_instrs_num(instrs) * MICRO_IROPTER_GROWTH_NUM;
}

void micro_iropter_deinit(micro_iropter_t *iropter)
{
    for (u32 i = 0; i < MICRO_IROPTER_CALL_PRICES_BUCKETS_NUM; i++) {
        if (iropter->call_prices[i].data) {
            sct_arena_vector_deinit(&iropter->call_prices[i]);
        }
    }
    sct_arena_deinit(&iropter->arena);
}