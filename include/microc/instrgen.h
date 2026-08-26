#ifndef MICROC_INSTRGEN_H
#define MICROC_INSTRGEN_H

#include <SCT/arena.h>

#include <micro/common.h>
#include <micro/expr.h>
#include <micro/instr.h>
#include <microc/lexer.h>

micro_type_t mc_type_str_parse(const char *str);

size_t mc_scroll_expr(sct_vector_t *toks, size_t i);

typedef struct {
    sct_arena_t   arena;
    sct_vector_t *toks;
    size_t        pos;
    sct_vector_t  instructions;
    int           code_in_function;
} mc_instrgen_t;

void mc_instrgen_init(mc_instrgen_t *instrgen, sct_vector_t *toks);

void mc_instrgen_deinit(mc_instrgen_t *instrgen);

void mc_instrgen_gen(mc_instrgen_t *instrgen);

void mc_instrgen_parse_set(mc_instrgen_t *instrgen);
void mc_instrgen_parse_fun(mc_instrgen_t *instrgen);
void mc_instrgen_parse_ret(mc_instrgen_t *instrgen);
void mc_instrgen_parse_call(mc_instrgen_t *instrgen);
void mc_instrgen_parse_lbl(mc_instrgen_t *instrgen);
void mc_instrgen_parse_goto(mc_instrgen_t *instrgen);
void mc_instrgen_parse_if(mc_instrgen_t *instrgen);

#endif