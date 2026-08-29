#include <micro/micro.h>

#include <microc/instrgen.h>
#include <microdebug/microdebug.h>

int main()
{
    micro_init();
        sct_vector_t instrs;
        sct_vector_init(&instrs, sizeof(micro_instruction_t));

        sct_vector_t expr;
        sct_vector_init(&expr, sizeof(micro_expr_tok_t));
        micro_make_expr(&expr, "+ 3 4");

        micro_debug_print_expr((micro_expr_tok_t*)expr.data, 0);

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