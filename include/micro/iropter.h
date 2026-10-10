#ifndef MICRO_IROPTER_H
#define MICRO_IROPTER_H

#include <SCT/arena_hashmap.h>
#include <SCT/arena_vector.h>
#include <micro/common.h>
#include <micro/instr.h>

#define MICRO_IROPTER_CALL_PRICES_BUCKETS_NUM 64

/* how much bigger a translation unit is allowed to become while inlining */
#define MICRO_IROPTER_GROWTH_NUM 2

/* the top level list is never changed by the pass, so a function can be
   reached through a pointer and its body stays readable while it grows */
typedef struct {
    micro_instruction_fun_t *fun;
} micro_iropter_fun_info_t;

typedef struct {
    sct_vector_t *instrs;  /* list the position belongs to, it changes while inlining */
    size_t        call_pos;
    u32           price;
} micro_iropter_call_info_t;

typedef struct {
    sct_vector_t       *instrs;
    size_t              pos;
    sct_arena_hashmap_t fun_infos;
    sct_arena_vector_t  call_prices[MICRO_IROPTER_CALL_PRICES_BUCKETS_NUM];
    sct_arena_t         arena;
    sct_vector_t       *instr_save;
    size_t              instr_save_pos;
    size_t              budget;        /* instructions the pass may add */
    size_t              inlined_instrs;
    u32                 inlined_calls;
} micro_iropter_t;

void micro_iropter_init(micro_iropter_t *iropter, sct_vector_t *instrs);

/* the pass allocates the expressions of the inlined code in its own arena,
   so it has to be called after the code generator is done with the list */
void micro_iropter_deinit(micro_iropter_t *iropter);

void micro_iropter_inlining_pass(micro_iropter_t *iropter);

#endif