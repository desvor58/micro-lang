#include <micro/micro.h>

#include <microc/lexer.h>
#include <microc/instrgen.h>

#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

static const char source[] =
    "tramp native_abs\n"
    "    i32 value\n"
    "    ret i32\n"
    "end\n"
    "\n"
    "fun absolute\n"
    "    i32 value\n"
    "    ret i32\n"
    "start\n"
    "    set i32 result;\n"
    "    call result native_abs - 0 value;\n"
    "    ret result;\n"
    "end\n";

static i32 native_abs(const micro_tramp_frame_t *frame)
{
    printf("native_abs: args_num=%lu, ret_type=%d, value=%d\n",
           (unsigned long)frame->args_num, (int)frame->ret_type, frame->args[0]);
    return abs(frame->args[0]);
}

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

    sct_hashmap_t tramps;
    sct_hashmap_init(&tramps, sizeof(micro_tramp_t));
    micro_tramp_t handler = native_abs;
    sct_hashmap_add(&tramps, "native_abs", &handler);

    micro_codegen_t codegen;
    micro_codegen386_init(&codegen, (micro_codegen_flags_t){0}, &asm_instrs, &arena, &tramps);
    codegen.emit(&codegen, &instrgen.instructions);
    micro_codegen386_deinit(&codegen);

    int failed = micro_err_stk_size != 0;

    mc_instrgen_deinit(&instrgen);
    sct_vector_deinit(&toks);
    sct_hashmap_deinit(&tramps);
    micro_deinit();

    if (failed) {
        puts("compilation failed");
        sct_arena_deinit(&arena);
        return 1;
    }

    /* the label names live in the arena, so it has to outlive the assembler */
    micro_asm386_optimize(&asm_instrs);
    micro_asm386_emit(&asm_instrs, &outbuf);

    sct_arena_deinit(&arena);
    sct_vector_deinit(&asm_instrs);

    long page_size = sysconf(_SC_PAGESIZE);
    void *exec_mem = mmap(NULL, page_size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

    if (exec_mem == MAP_FAILED) {
        perror("mmap failed");
        sct_vector_deinit(&outbuf);
        return 1;
    }

    memcpy(exec_mem, outbuf.data, outbuf.size);

    i32 (*absolute)(i32) = (i32 (*)(i32))exec_mem;

    for (i32 value = -9; value <= 9; value += 9) {
        printf("absolute(%d) = %d\n", value, absolute(value));
    }

    munmap(exec_mem, page_size);

    sct_vector_deinit(&outbuf);

    return 0;
}