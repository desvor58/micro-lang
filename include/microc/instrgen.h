#ifndef MICROC_INSTRGEN_H
#define MICROC_INSTRGEN_H

#include <SCT/arena.h>

#include <micro/common.h>
#include <micro/expr.h>
#include <micro/instr.h>
#include <microc/lexer.h>

micro_type_t micro_type_str_parse(const char *str);

size_t micro_scroll_expr(sct_vector_t *toks, size_t i);

typedef struct {
    sct_arena_t   arena;
    sct_vector_t *toks;
    size_t        pos;
    sct_vector_t  instructions;
    int           code_in_function;
} micro_instrgen_t;

void micro_instrgen_init(micro_instrgen_t *instrgen, sct_vector_t *toks);

void micro_instrgen_deinit(micro_instrgen_t *instrgen);

void micro_instrgen_gen(micro_instrgen_t *instrgen);

void micro_instrgen_parse_set(micro_instrgen_t *instrgen);
void micro_instrgen_parse_fun(micro_instrgen_t *instrgen);
void micro_instrgen_parse_ret(micro_instrgen_t *instrgen);
void micro_instrgen_parse_call(micro_instrgen_t *instrgen);
void micro_instrgen_parse_lbl(micro_instrgen_t *instrgen);
void micro_instrgen_parse_goto(micro_instrgen_t *instrgen);
void micro_instrgen_parse_if(micro_instrgen_t *instrgen);

#endif