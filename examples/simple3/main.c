#include <micro/micro.h>

#include <microc/lexer.h>
#include <microc/instrgen.h>

#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

static const char source[] =
    "fun fib\n"
    "    i32 n\n"
    "    ret i32\n"
    "start\n"
    "    if <= n 1 : base;\n"
    "    set i32 n1 - n 1;\n"
    "    set i32 f1;\n"
    "    call f1 fib n1;\n"
    "    set i32 n2 - n 2;\n"
    "    set i32 f2;\n"
    "    call f2 fib n2;\n"
    "    ret + f1 f2;\n"
    "base:\n"
    "    ret n;\n"
    "end\n";

int main()
{
    micro_init();

    sct_vector_t toks;
    sct_vector_init(&toks, sizeof(mc_token_t));
    mc_tokenize(source, sizeof(source) - 1, &toks);

    mc_instrgen_t instrgen;
    mc_instrgen_init(&instrgen, &toks);
    mc_instrgen_gen(&instrgen);

    sct_vector_t asm_instrs;
    sct_vector_init(&asm_instrs, sizeof(micro_asm386_instruction_t));

    sct_vector_t outbuf;
    sct_vector_init(&outbuf, sizeof(u8));

    sct_arena_t arena;
    sct_arena_init(&arena);

    micro_codegen_t codegen;
    micro_codegen386_init(&codegen, (micro_codegen_flags_t){0}, &asm_instrs, &arena);
    codegen.emit(&codegen, &instrgen.instructions);
    micro_codegen386_deinit(&codegen);

    mc_instrgen_deinit(&instrgen);
    sct_vector_deinit(&toks);
    sct_arena_deinit(&arena);
    micro_deinit();

    if (micro_err_stk_size) {
        puts("compilation failed");
        return 1;
    }

    micro_asm386_optimize(&asm_instrs);
    micro_asm386_emit(&asm_instrs, &outbuf);

    sct_vector_deinit(&asm_instrs);

    long page_size = sysconf(_SC_PAGESIZE);
    void *exec_mem = mmap(NULL, page_size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

    if (exec_mem == MAP_FAILED) {
        perror("mmap failed");
        sct_vector_deinit(&outbuf);
        return 1;
    }

    memcpy(exec_mem, outbuf.data, outbuf.size);

    i32 (*fib)(i32 n) = (i32 (*)(i32))exec_mem;

    for (i32 n = 0; n < 10; n++) {
        printf("fib(%d) = %d\n", n, fib(n));
    }

    munmap(exec_mem, page_size);

    sct_vector_deinit(&outbuf);

    return 0;
}
