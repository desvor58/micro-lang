#include <micro/instrgen.h>

struct {
    char name[4];
    micro_type_t type;
} micro_type_str_parse_tbl[] = {
    { "i8", MICRO_TYPE_I8 },
    { "u8", MICRO_TYPE_U8 },
    { "i16", MICRO_TYPE_I16 },
    { "u16", MICRO_TYPE_U16 },
    { "i32", MICRO_TYPE_I32 },
    { "u32", MICRO_TYPE_U32 },
    { "f32", MICRO_TYPE_F32 },
    { "ptr", MICRO_TYPE_PTR },
};

micro_type_t micro_type_str_parse(const char *str)
{
    for (size_t i = 0; i < sizeof(micro_type_str_parse_tbl) / sizeof(*micro_type_str_parse_tbl); i++) {
        if (!strcmp(str, micro_type_str_parse_tbl[i].name)) {
            return micro_type_str_parse_tbl[i].type;
        }
    }
    return MICRO_TYPE_NULL;
}

u8 op_args_num[] = {
    [MICRO_TOK_PLUS]        = 2,
    [MICRO_TOK_MINUS]       = 2,
    [MICRO_TOK_STAR]        = 2,
    [MICRO_TOK_SLASH]       = 2,
    [MICRO_TOK_AMPERSAND]   = 1,
    [MICRO_TOK_DOLLAR]      = 1,
    [MICRO_TOK_HASH]        = 1,
    [MICRO_TOK_APOSTROPHE]  = 1,
    [MICRO_TOK_TILDE]       = 1,
    [MICRO_TOK_EQ]          = 2,
    [MICRO_TOK_EXCLAMATION] = 1,
    [MICRO_TOK_NOT_EQ]      = 2,
    [MICRO_TOK_GREAT]       = 2,
    [MICRO_TOK_LESS]        = 2,
    [MICRO_TOK_GREAT_OR_EQ] = 2,
    [MICRO_TOK_LESS_OR_EQ]  = 2,
};

size_t micro_scroll_expr(sct_vector_t *toks, size_t pos)
{
    micro_token_t *tok = sct_vector_get(toks, pos);
    if (!tok) {
        micro_token_t *err_tok = sct_vector_get(toks, toks->size - 1);
        micro_push_err((micro_error_t){
            .err = MICRO_ERROR_EXPECTED_EXPRESSION,
            .line_ref = err_tok->line_ref,
            .chpos_ref = err_tok->chpos_ref
        });
        return 0;
    }
    if (_micro_tok_is_lit(tok->type) || tok->type == MICRO_TOK_IDENT) {
        return 1;
    }
    if (_micro_tok_is_op(tok->type)) {
        u8 num = op_args_num[tok->type];
        size_t offset = 1;
        while (num) {
            size_t doffset = micro_scroll_expr(toks, pos + offset);
            if (!doffset) return 0;
            offset += doffset;
            num--;
        }
        return offset;
    }
    micro_push_err((micro_error_t){
        .err = MICRO_ERROR_EXPECTED_EXPRESSION,
        .line_ref = tok->line_ref,
        .chpos_ref = tok->chpos_ref
    });
    return 0;
}

void micro_instrgen_init(micro_instrgen_t *instrgen, sct_vector_t *toks)
{
    sct_arena_init(&instrgen->arena);
    instrgen->toks = toks;
    instrgen->pos = 0;
    sct_vector_init(&instrgen->instructions, sizeof(micro_instruction_t));
    instrgen->code_in_function = 0;
}

static void instrs_deinit(micro_instrgen_t *instrgen, sct_vector_t *instrs)
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

void micro_instrgen_deinit(micro_instrgen_t *instrgen)
{
    instrs_deinit(instrgen, &instrgen->instructions);
    sct_arena_deinit(&instrgen->arena);
}

void micro_instrgen_gen(micro_instrgen_t *instrgen)
{
    for (instrgen->pos = 0; instrgen->pos < instrgen->toks->size; instrgen->pos++) {
        micro_token_t *tok = sct_vector_get(instrgen->toks, instrgen->pos);
        if (!tok) {
            return;
        }
        switch (tok->type) {
            case MICRO_TOK_KW_SET:
                micro_instrgen_parse_set(instrgen);
                break;

            case MICRO_TOK_KW_FUN:
                micro_instrgen_parse_fun(instrgen);
                break;

            case MICRO_TOK_KW_RET:
                micro_instrgen_parse_ret(instrgen);
                break;

            case MICRO_TOK_KW_CALL:
                micro_instrgen_parse_call(instrgen);
                break;

            case MICRO_TOK_IDENT:
                micro_instrgen_parse_lbl(instrgen);
                break;

            case MICRO_TOK_KW_GOTO:
                micro_instrgen_parse_goto(instrgen);
                break;

            case MICRO_TOK_KW_END:
                if (!instrgen->code_in_function) {
                    micro_push_err((micro_error_t){
                        .err = MICRO_ERROR_UNEXPECTED_END_KW,
                        .line_ref = tok->line_ref,
                        .chpos_ref = tok->chpos_ref
                    });
                }
                return;

            default:
                puts(micro_token_type2str[tok->type]);
                micro_push_err((micro_error_t){
                    .err = MICRO_ERROR_UNEXPECTED_TOKEN,
                    .line_ref = tok->line_ref,
                    .chpos_ref = tok->chpos_ref
                });
                return;
        }
    }
}
