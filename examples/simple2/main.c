#include <micro/micro.h>

#include <microc/instrgen.h>
#include <microdebug/microdebug.h>

#include <sys/mman.h>
#include <unistd.h>

int main()
{
    micro_init();
        sct_vector_t body;
        sct_vector_init(&body, sizeof(micro_instruction_t));

        sct_vector_t expr;
        sct_vector_init(&expr, sizeof(micro_expr_tok_t));
        micro_make_expr(&expr, "+ a b");

        micro_debug_print_expr((micro_expr_tok_t*)expr.data, 0);

        micro_instr_gen_ret(&body, &expr);

        sct_vector_t instrs;
        sct_vector_init(&instrs, sizeof(micro_instruction_t));

        sct_vector_t args;
        sct_vector_init(&args, sizeof(micro_instruction_fun_arg_t));
        sct_vector_push(&args, &(micro_instruction_fun_arg_t){ .name = "a", .type = MICRO_TYPE_I32 });
        sct_vector_push(&args, &(micro_instruction_fun_arg_t){ .name = "b", .type = MICRO_TYPE_I32 });
        micro_instr_gen_fun(&instrs, "my_fun", &args, MICRO_TYPE_I32, &body);

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
        sct_vector_deinit(&body);
        sct_vector_deinit(&expr);

        micro_asm386_emit(&asm_instrs, &outbuf);

        sct_vector_deinit(&asm_instrs);
        sct_arena_deinit(&arena);
    micro_deinit();

    long page_size = sysconf(_SC_PAGESIZE);
    void* exec_mem = mmap(NULL, page_size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

    if (exec_mem == MAP_FAILED) {
        perror("mmap failed");
        sct_vector_deinit(&outbuf);
        return 1;
    }

    memcpy(exec_mem, outbuf.data, outbuf.size);

    i32 (*my_micro_fun)(i32 a, i32 b) = (i32 (*)(i32, i32))exec_mem;

    printf("5 + 4 = %d\n", my_micro_fun(5, 4));

    munmap(exec_mem, page_size);

    sct_vector_deinit(&outbuf);
}