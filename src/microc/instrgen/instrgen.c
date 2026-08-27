#include <microc/instrgen.h>

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
