#ifndef MICRO_INSTRGEN_COMMON_H
#define MICRO_INSTRGEN_COMMON_H

#include <SCT/arena.h>
#include <micro/common.h>
#include <micro/lexer.h>

#define _micro_tok_is_op(tok_type) ((tok_type) == MICRO_TOK_PLUS ||  \
                                    (tok_type) == MICRO_TOK_MINUS ||  \
                                    (tok_type) == MICRO_TOK_STAR ||  \
                                    (tok_type) == MICRO_TOK_SLASH ||  \
                                    (tok_type) == MICRO_TOK_DOT ||  \
                                    (tok_type) == MICRO_TOK_COMA ||  \
                                    (tok_type) == MICRO_TOK_COLON ||  \
                                    (tok_type) == MICRO_TOK_SEMICOLON ||  \
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

#define _micro_tok_is_expr_start(tok_type) (_micro_tok_is_op(tok_type) || _micro_tok_is_lit(tok_type))

micro_type_t micro_type_str_parse(const char *str);

typedef enum {
    MICRO_INSTR_SET,
    MICRO_INSTR_FUN,
    MICRO_INSTR_RET,
    MICRO_INSTR_GOTO,
    MICRO_INSTR_IF,
    MICRO_INSTR_CALL,
} micro_instruction_type_t;

typedef struct {
    micro_type_t type;
    char reg_name[MICRO_MAX_SYMBOL_SIZE];
    micro_token_t *val_expr;
} micro_instruction_set_t;

typedef struct {
    char name[MICRO_MAX_SYMBOL_SIZE];
    micro_type_t ret_type;
} micro_instruction_fun_t;

typedef struct {
    micro_instruction_type_t type;
    union {
        micro_instruction_set_t set;
        micro_instruction_fun_t fun;
    };
} micro_instruction_t;

typedef struct {
    sct_arena_t   arena;
    sct_vector_t *toks;
    size_t        pos;
    sct_vector_t  instructions;
} micro_instrgen_t;

void micro_instrgen_init(micro_instrgen_t *instrgen, sct_vector_t *toks);

void micro_instrgen_deinit(micro_instrgen_t *instrgen);

void micro_instrgen_gen(micro_instrgen_t *instrgen);

void micro_instrgen_parse_set(micro_instrgen_t *instrgen);

#endif