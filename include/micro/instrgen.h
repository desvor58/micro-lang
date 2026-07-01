#ifndef MICRO_INSTRGEN_COMMON_H
#define MICRO_INSTRGEN_COMMON_H

#include <SCT/arena.h>
#include <micro/common.h>
#include <micro/lexer.h>

#define _micro_tok_is_op(tok_type) ((tok_type) == MICRO_TOK_PLUS ||  \
                                    (tok_type) == MICRO_TOK_MINUS ||  \
                                    (tok_type) == MICRO_TOK_STAR ||  \
                                    (tok_type) == MICRO_TOK_SLASH ||  \
                                    (tok_type) == MICRO_TOK_AMPERSAND ||  \
                                    (tok_type) == MICRO_TOK_DOLLAR ||  \
                                    (tok_type) == MICRO_TOK_HASH ||  \
                                    (tok_type) == MICRO_TOK_APOSTROPHE ||  \
                                    (tok_type) == MICRO_TOK_TILDE ||  \
                                    (tok_type) == MICRO_TOK_EQ ||  \
                                    (tok_type) == MICRO_TOK_EXCLAMATION ||  \
                                    (tok_type) == MICRO_TOK_NOT_EQ ||  \
                                    (tok_type) == MICRO_TOK_GREAT ||  \
                                    (tok_type) == MICRO_TOK_LESS ||  \
                                    (tok_type) == MICRO_TOK_GREAT_OR_EQ ||  \
                                    (tok_type) == MICRO_TOK_LESS_OR_EQ)

#define _micro_tok_is_lit(tok_type) ((tok_type) == MICRO_TOK_LIT_FLOAT ||  \
                                     (tok_type) == MICRO_TOK_LIT_INT ||  \
                                     (tok_type) == MICRO_TOK_LIT_STR)

#define _micro_tok_is_expr_start(tok_type) (_micro_tok_is_op(tok_type) || _micro_tok_is_lit(tok_type) || tok_type == MICRO_TOK_IDENT)

micro_type_t micro_type_str_parse(const char *str);

size_t micro_scroll_expr(sct_vector_t *toks, size_t i);

typedef enum {
    MICRO_INSTR_SET,
    MICRO_INSTR_DRSET,  // DeRef&SET
    MICRO_INSTR_FUN,
    MICRO_INSTR_RET,
    MICRO_INSTR_CALL,
    MICRO_INSTR_LBL,
    MICRO_INSTR_GOTO,
    MICRO_INSTR_IF,
} micro_instruction_type_t;

typedef struct {
    micro_type_t   type;
    char           reg_name[MICRO_MAX_SYMBOL_SIZE];
    micro_token_t *val_expr;
} micro_instruction_set_t;

typedef struct {
    micro_type_t   type;
    char           reg_name[MICRO_MAX_SYMBOL_SIZE];
    micro_token_t *val_expr;
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
    micro_token_t *val_expr;
} micro_instruction_ret_t;

typedef struct {
    char ret_reg_name[MICRO_MAX_SYMBOL_SIZE];
    char fun_name[MICRO_MAX_SYMBOL_SIZE];
    sct_vector_t arg_exprs;
} micro_instruction_call_t;

typedef struct {
    char name[MICRO_MAX_SYMBOL_SIZE];
} micro_instruction_lbl_t;

typedef struct {
    micro_instruction_type_t type;
    union {
        micro_instruction_set_t   set;
        micro_instruction_drset_t drset;
        micro_instruction_fun_t   fun;
        micro_instruction_ret_t   ret;
        micro_instruction_call_t  call;
    };
} micro_instruction_t;

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

#endif