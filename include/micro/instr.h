#ifndef MICRO_INSTR_H
#define MICRO_INSTR_H

#include <SCT/vector.h>

#include <micro/common.h>
#include <micro/expr.h>

typedef struct {
    micro_type_t      type;
    char              reg_name[MICRO_MAX_SYMBOL_SIZE];
    micro_expr_tok_t *val_expr;
} micro_instruction_set_t;

typedef struct {
    micro_type_t      type;
    char              reg_name[MICRO_MAX_SYMBOL_SIZE];
    micro_expr_tok_t *val_expr;
} micro_instruction_drset_t;

typedef struct {
    micro_type_t type;
    char         name[MICRO_MAX_SYMBOL_SIZE];
} micro_instruction_fun_arg_t;

typedef struct {
    char         name[MICRO_MAX_SYMBOL_SIZE];
    sct_vector_t args;
    micro_type_t ret_type;
    sct_vector_t body;
} micro_instruction_fun_t;

typedef struct {
    micro_expr_tok_t *val_expr;
} micro_instruction_ret_t;

typedef struct {
    char         ret_reg_name[MICRO_MAX_SYMBOL_SIZE];
    char         fun_name[MICRO_MAX_SYMBOL_SIZE];
    sct_vector_t arg_exprs;
} micro_instruction_call_t;

typedef struct {
    char name[MICRO_MAX_SYMBOL_SIZE];
} micro_instruction_lbl_t;

typedef struct {
    char lbl[MICRO_MAX_SYMBOL_SIZE];
} micro_instruction_goto_t;

typedef struct {
    micro_expr_tok_t *cond_expr;
    char              lbl_name[MICRO_MAX_SYMBOL_SIZE];
} micro_instruction_if_t;

typedef struct {
    micro_instruction_type_t type;
    union {
        micro_instruction_set_t   set;
        micro_instruction_drset_t drset;
        micro_instruction_fun_t   fun;
        micro_instruction_ret_t   ret;
        micro_instruction_call_t  call;
        micro_instruction_lbl_t   lbl;
        micro_instruction_goto_t  goto_lbl;
        micro_instruction_if_t    if_goto;
    };
} micro_instruction_t;

size_t micro_make_expr(sct_vector_t *expr_dst, const char *str_expr);

/// @param instrs vector of micro_instruction_t - main instruction list
/// @param type type of dereferenced memory
/// @param name name of virtual register for dereferencing
/// @param expr vector of micro_expr_tok_t - init expr. if = 0 then non init (in code set <type> #<name>;)
int micro_instr_gen_set(sct_vector_t *instrs, micro_type_t type, const char *name, sct_vector_t *expr);

/// @param instrs vector of micro_instruction_t - main instruction list
/// @param type type of virtual register
/// @param name name of virtual register
/// @param expr vector of micro_expr_tok_t - init expr. if = 0 then non init (in code set <type> <name>;)
int micro_instr_gen_drset(sct_vector_t *instrs, micro_type_t type, const char *name, sct_vector_t *expr);

/// @param instrs vector of micro_instruction_t - main instruction list
/// @param name name of function
/// @param args vector of micro_instruction_fun_arg_t - arguments of function
/// @param ret_type type which function will return
/// @param body vector of micro_instruction_t - body of function
int micro_instr_gen_fun(sct_vector_t *instrs, const char *name, sct_vector_t *args, micro_type_t ret_type, sct_vector_t *body);

/// @param instrs vector of micro_instruction_t - main instruction list
/// @param expr vector of micro_expr_tok_t - return expr. if = 0 then ret have not expression (in code ret;)
int micro_instr_gen_ret(sct_vector_t *instrs, sct_vector_t *expr);

/// @param instrs vector of micro_instruction_t - main instruction list
/// @param reg_name name of virtual register to save returned value
/// @param fun_name name of function to calling
/// @param args vector of expressions - sct_vector_t of micro_expr_tok_t - arguments for calling
int micro_instr_gen_call(sct_vector_t *instrs, const char *reg_name, const char *fun_name, sct_vector_t *args);

/// @param instrs vector of micro_instruction_t - main instruction list
/// @param name name of label
int micro_instr_gen_lbl(sct_vector_t *instrs, const char *name);

/// @param instrs vector of micro_instruction_t - main instruction list
/// @param name name of label for jumping
int micro_instr_gen_goto(sct_vector_t *instrs, const char *name);

/// @param instrs vector of micro_instruction_t - main instruction list
/// @param cond_expr expression to check
/// @param lbl_name name of label for jumping if cond_expr != 0
int micro_instr_gen_if(sct_vector_t *instrs, sct_vector_t *cond_expr, const char *lbl_name);

#endif