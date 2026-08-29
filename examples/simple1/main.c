#include <micro/micro.h>

#include <microc/instrgen.h>

void print_tok(mc_token_t tok)
{
    printf("%lu:%lu type:%s, val:%s\n",
                       tok.line_ref,
                       tok.chpos_ref,
                       mc_token_type2str[tok.type],
                       tok.val);
}

char *str_type[] = {
    [MICRO_TYPE_NULL] = "null",
    [MICRO_TYPE_I8]   = "i8",
    [MICRO_TYPE_U8]   = "u8",
    [MICRO_TYPE_I16]  = "i16",
    [MICRO_TYPE_U16]  = "u16",
    [MICRO_TYPE_I32]  = "i32",
    [MICRO_TYPE_U32]  = "u32",
    [MICRO_TYPE_F32]  = "f32",
    [MICRO_TYPE_PTR]  = "ptr"
};

u8 _op_args_num[] = {
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

size_t print_expr(micro_expr_tok_t *start, size_t tab)
{
    if (!start) return 0;
    for (size_t i = 0; i < tab; i++) {
        putchar(' ');
    }
    print_tok(*(mc_token_t*)start);
    if (_micro_expr_is_lit(start->type) || start->type == MICRO_EXPR_TOK_IDENT) {
        return 1;
    }
    if (_micro_expr_is_op(start->type)) {
        u8 num = _op_args_num[start->type];
        size_t offset = 0;
        while (num) {
            offset += print_expr(start + offset + 1, tab);
            num--;
        }
        return offset;
    }
    puts("print_expr error!");
    return 0;
}


int main()
{
    micro_init();
        sct_vector_t instrs;
        sct_vector_init(&instrs, sizeof(micro_instruction_t));

        sct_vector_t expr;
        sct_vector_init(&expr, sizeof(micro_expr_tok_t));
        micro_make_expr(&expr, "+ 3 4");

        print_expr((micro_expr_tok_t*)expr.data, 0);

        micro_instr_gen_set(&instrs, MICRO_TYPE_I32, "test_var", &expr);

        sct_vector_t asm_instrs;
        sct_vector_init(&asm_instrs, sizeof(micro_asm386_instruction_t));

        sct_vector_t outbuf;
        sct_vector_init(&outbuf, sizeof(u8));

        sct_arena_t arena;
        sct_arena_init(&arena);

        micro_codegen_flags_t codegen_flags = {
            .no_err_outside_fun = 1,
        };

        micro_codegen_t codegen;
        micro_codegen386_init(&codegen, codegen_flags, &asm_instrs, &arena);
            codegen.emit(&codegen, &instrs);
        micro_codegen386_deinit(&codegen);

        sct_vector_deinit(&instrs);
        sct_vector_deinit(&expr);

        micro_asm386_emit(&asm_instrs, &outbuf);

        sct_vector_deinit(&asm_instrs);
        sct_arena_deinit(&arena);
    micro_deinit();

    for (size_t i = 0; i < outbuf.size; i++) {
        printf("%x ", *(u8*)sct_vector_get(&outbuf, i));
    }
}