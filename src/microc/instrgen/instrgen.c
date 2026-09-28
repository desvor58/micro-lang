#include <microc/instrgen.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>

struct {
    char name[4];
    micro_type_t type;
} mc_type_str_parse_tbl[] = {
    { "i8", MICRO_TYPE_I8 },
    { "u8", MICRO_TYPE_U8 },
    { "i16", MICRO_TYPE_I16 },
    { "u16", MICRO_TYPE_U16 },
    { "i32", MICRO_TYPE_I32 },
    { "u32", MICRO_TYPE_U32 },
    { "f32", MICRO_TYPE_F32 },
    { "ptr", MICRO_TYPE_PTR },
};

micro_type_t mc_type_str_parse(const char *str)
{
    for (size_t i = 0; i < sizeof(mc_type_str_parse_tbl) / sizeof(*mc_type_str_parse_tbl); i++) {
        if (!strcmp(str, mc_type_str_parse_tbl[i].name)) {
            return mc_type_str_parse_tbl[i].type;
        }
    }
    return MICRO_TYPE_NULL;
}

u8 op_args_num[] = {
    [MC_TOK_PLUS]        = 2,
    [MC_TOK_MINUS]       = 2,
    [MC_TOK_STAR]        = 2,
    [MC_TOK_SLASH]       = 2,
    [MC_TOK_AMPERSAND]   = 1,
    [MC_TOK_DOLLAR]      = 1,
    [MC_TOK_HASH]        = 1,
    [MC_TOK_APOSTROPHE]  = 1,
    [MC_TOK_TILDE]       = 1,
    [MC_TOK_EQ]          = 2,
    [MC_TOK_EXCLAMATION] = 1,
    [MC_TOK_GREAT]       = 2,
    [MC_TOK_LESS]        = 2,
    [MC_TOK_GREAT_OR_EQ] = 2,
    [MC_TOK_LESS_OR_EQ]  = 2,
};

size_t mc_scroll_expr(sct_vector_t *toks, size_t pos)
{
    mc_token_t *tok = sct_vector_get(toks, pos);
    if (!tok) {
        mc_token_t *err_tok = sct_vector_get(toks, toks->size - 1);
        micro_push_err((micro_error_t){
            .err = MICRO_ERROR_EXPECTED_EXPRESSION,
        });
        return 0;
    }
    if (_micro_expr_is_lit(tok->type) || tok->type == MC_TOK_IDENT) {
        return 1;
    }
    if (_micro_expr_is_op(tok->type)) {
        u8 num = op_args_num[tok->type];
        size_t offset = 1;
        while (num) {
            size_t doffset = mc_scroll_expr(toks, pos + offset);
            if (!doffset) return 0;
            offset += doffset;
            num--;
        }
        return offset;
    }
    micro_push_err((micro_error_t){
        .err = MICRO_ERROR_EXPECTED_EXPRESSION,
    });
    return 0;
}

static int mc_hints_error(micro_instruction_type_t instr_type)
{
    micro_push_err((micro_error_t){
        .err = MICRO_ERROR_UNEXPECTED_TOKEN,
        .instr = instr_type,
    });
    return 0;
}

static int mc_hint_bool(const mc_token_t *tok, size_t *value)
{
    if (tok->type == MC_TOK_IDENT) {
        if (!strcmp(tok->val, "true")) {
            *value = 1;
            return 1;
        }
        if (!strcmp(tok->val, "false")) {
            *value = 0;
            return 1;
        }
    }
    if (tok->type == MC_TOK_LIT_INT) {
        if (!strcmp(tok->val, "0")) {
            *value = 0;
            return 1;
        }
        if (!strcmp(tok->val, "1")) {
            *value = 1;
            return 1;
        }
    }
    return 0;
}

int mc_instrgen_parse_hints(mc_instrgen_t *instrgen, micro_instruction_hints_t *hints, micro_instruction_type_t instr_type)
{
    if (!instrgen || !hints) {
        return 0;
    }

    *hints = (micro_instruction_hints_t){
        .lifetime = -1,
        .forced_stack = 0,
        .lazy_init = 0,
    };

    mc_token_t *tok = sct_vector_get(instrgen->toks, instrgen->pos);
    if (!tok || tok->type != MC_TOK_LBRACE) {
        return 1;
    }

    instrgen->pos++;
    tok = sct_vector_get(instrgen->toks, instrgen->pos);
    if (tok && tok->type == MC_TOK_RBRACE) {
        instrgen->pos++;
        return 1;
    }

    for (;;) {
        mc_token_t *name_tok = sct_vector_get(instrgen->toks, instrgen->pos++);
        if (!name_tok || name_tok->type != MC_TOK_IDENT) {
            return mc_hints_error(instr_type);
        }

        mc_token_t *colon_tok = sct_vector_get(instrgen->toks, instrgen->pos++);
        if (!colon_tok || colon_tok->type != MC_TOK_COLON) {
            return mc_hints_error(instr_type);
        }

        mc_token_t *value_tok = sct_vector_get(instrgen->toks, instrgen->pos++);
        if (!value_tok) {
            return mc_hints_error(instr_type);
        }

        if (!strcmp(name_tok->val, "livetime") || !strcmp(name_tok->val, "lifetime")) {
            char *end;
            long long value;
            if (value_tok->type != MC_TOK_LIT_INT) {
                return mc_hints_error(instr_type);
            }
            errno = 0;
            value = strtoll(value_tok->val, &end, 10);
            if (errno == ERANGE || end == value_tok->val || *end || value < PTRDIFF_MIN || value > PTRDIFF_MAX) {
                return mc_hints_error(instr_type);
            }
            hints->lifetime = (ptrdiff_t)value;
        } else {
            size_t bool_value;
            if (!strcmp(name_tok->val, "forced_stack")) {
                if (!mc_hint_bool(value_tok, &bool_value)) {
                    return mc_hints_error(instr_type);
                }
                hints->forced_stack = bool_value;
            } else
            if (!strcmp(name_tok->val, "lazy_init")) {
                if (!mc_hint_bool(value_tok, &bool_value)) {
                    return mc_hints_error(instr_type);
                }
                hints->lazy_init = bool_value;
            } else {
                return mc_hints_error(instr_type);
            }
        }

        tok = sct_vector_get(instrgen->toks, instrgen->pos);
        if (tok && tok->type == MC_TOK_RBRACE) {
            instrgen->pos++;
            return 1;
        }
        if (!tok || tok->type != MC_TOK_COMA) {
            return mc_hints_error(instr_type);
        }
        instrgen->pos++;
    }
}

void mc_instrgen_init(mc_instrgen_t *instrgen, sct_vector_t *toks)
{
    sct_arena_init(&instrgen->arena);
    instrgen->toks = toks;
    instrgen->pos = 0;
    sct_vector_init(&instrgen->instructions, sizeof(micro_instruction_t));
    instrgen->code_in_function = 0;
}

static void instrs_deinit(mc_instrgen_t *instrgen, sct_vector_t *instrs)
{
    for (size_t i = 0; i < instrs->size; i++) {
        micro_instruction_t *instr = sct_vector_get(instrs, i);

        if (instr->type == MICRO_INSTR_FUN) {
            sct_vector_deinit(&instr->fun.args);
            instrs_deinit(instrgen, &instr->fun.body);
        }
    }
    sct_vector_deinit(instrs);
}

void mc_instrgen_deinit(mc_instrgen_t *instrgen)
{
    instrs_deinit(instrgen, &instrgen->instructions);
    sct_arena_deinit(&instrgen->arena);
}

void mc_instrgen_gen(mc_instrgen_t *instrgen)
{
    for (instrgen->pos = 0; instrgen->pos < instrgen->toks->size; instrgen->pos++) {
        mc_token_t *tok = sct_vector_get(instrgen->toks, instrgen->pos);
        if (!tok) {
            return;
        }
        switch (tok->type) {
            case MC_TOK_KW_SET:
                mc_instrgen_parse_set(instrgen);
                break;

            case MC_TOK_KW_FUN:
                mc_instrgen_parse_fun(instrgen);
                break;

            case MC_TOK_KW_RET:
                mc_instrgen_parse_ret(instrgen);
                break;

            case MC_TOK_KW_CALL:
                mc_instrgen_parse_call(instrgen);
                break;

            case MC_TOK_IDENT:
                mc_instrgen_parse_lbl(instrgen);
                break;

            case MC_TOK_KW_GOTO:
                mc_instrgen_parse_goto(instrgen);
                break;

            case MC_TOK_KW_IF:
                mc_instrgen_parse_if(instrgen);
                break;

            case MC_TOK_KW_END:
                if (!instrgen->code_in_function) {
                    micro_push_err((micro_error_t){
                        .err = MICRO_ERROR_UNEXPECTED_END_KW,
                    });
                }
                return;

            default:
                puts(mc_token_type2str[tok->type]);
                micro_push_err((micro_error_t){
                    .err = MICRO_ERROR_UNEXPECTED_TOKEN,
                });
                return;
        }
    }
}
