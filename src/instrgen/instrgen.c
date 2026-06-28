#include <micro/instrgen/instrgen.h>

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

void micro_instrgen_init(micro_instrgen_t *instrgen, sct_vector_t *toks)
{
    sct_arena_init(&instrgen->arena);
    instrgen->toks = toks;
    instrgen->pos = 0;
    sct_vector_init(&instrgen->instructions, sizeof(micro_instruction_t));
}

void micro_instrgen_deinit(micro_instrgen_t *instrgen)
{
    sct_vector_deinit(&instrgen->instructions);
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

            default:
                micro_push_err((micro_error_t){
                    .msg = "Unexpected token",
                    .line_ref = tok->line_ref,
                    .chpos_ref = tok->chpos_ref
                });
                return;
        }
    }
}
