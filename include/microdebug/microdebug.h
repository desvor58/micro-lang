/*************************************************
 *              micro-debug library
 *
 * Optional debug/print helpers for the micro
 * toolchain. Built separately as libmicro-debug.a
 * and is not required by libmicro.
 ************************************************/

#ifndef MICRODEBUG_H
#define MICRODEBUG_H

#include <SCT/vector.h>

#include <micro/common.h>
#include <micro/expr.h>
#include <micro/instr.h>
#include <micro/asm/asm386.h>
#include <microc/lexer.h>

/* human-readable names of micro_error_type_t values */
extern const char *micro_debug_err_str[];

/* human-readable names of micro_instruction_type_t values */
extern const char *micro_debug_instr_str[];

/* human-readable names of micro_type_t values */
extern const char *micro_debug_str_type[];

/* number of operands each expression operator takes */
extern u8 micro_debug_op_args_num[];

/* print a single compiler error to stdout, like microc does */
void micro_debug_put_err(const char *file, micro_error_t err);

/* print a single token to stdout */
void micro_debug_put_tok(mc_token_t tok);

/* recursively print an expression tree to stdout.
 * @return total number of expression tokens consumed */
size_t micro_debug_print_expr(micro_expr_tok_t *start, size_t tab);

/* recursively print a vector of micro_instruction_t to stdout */
void micro_debug_print_instructions(sct_vector_t *instrs, size_t tab);

/* print a vector of micro_asm386_instruction_t as pseudo-assembly */
void micro_debug_put_asm(sct_vector_t *asm_instrs);

#endif