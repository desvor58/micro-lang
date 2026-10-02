#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

#include <llvm-c/Core.h>
#include <llvm-c/IRReader.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/Target.h>

static double now_sec(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9;
}

static char *read_file(const char *path, size_t *size_out)
{
    FILE *file = fopen(path, "rb");
    if (!file) {
        fprintf(stderr, "Cannot open %s\n", path);
        return NULL;
    }
    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return NULL;
    }
    long size = ftell(file);
    if (size <= 0 || fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        return NULL;
    }
    char *data = malloc((size_t)size + 1);
    if (!data) {
        fclose(file);
        return NULL;
    }
    size_t read = fread(data, 1, (size_t)size, file);
    fclose(file);
    data[read] = 0;
    *size_out = read;
    return data;
}

static char *algorithm_source(const char *source)
{
    const char *main_start = strstr(source, "\ndefine i32 @main()");
    if (!main_start) {
        size_t size = strlen(source) + 1;
        char *copy = malloc(size);
        if (copy) {
            memcpy(copy, source, size);
        }
        return copy;
    }
    const char *main_end = strstr(main_start, "\n}");
    if (!main_end) {
        return NULL;
    }
    main_end += 2;
    size_t prefix = (size_t)(main_start - source);
    size_t suffix = strlen(main_end);
    char *copy = malloc(prefix + suffix + 1);
    if (!copy) {
        return NULL;
    }
    memcpy(copy, source, prefix);
    memcpy(copy + prefix, main_end, suffix + 1);
    return copy;
}

typedef struct {
    LLVMContextRef ctx;
    LLVMModuleRef mod;
    LLVMExecutionEngineRef engine;
    void *addr;
} pipeline_t;

static int build_pipeline(const char *source, const char *entry_name,
                         pipeline_t *pipeline)
{
    LLVMContextRef ctx = LLVMContextCreate();
    LLVMMemoryBufferRef buffer = LLVMCreateMemoryBufferWithMemoryRangeCopy(
        source, strlen(source), "bench");
    LLVMModuleRef mod = NULL;
    LLVMBool parsed = LLVMParseIRInContext(ctx, buffer, &mod, NULL);
    if (parsed || !mod) {
        if (mod) {
            LLVMDisposeModule(mod);
        }
        LLVMContextDispose(ctx);
        return 0;
    }

    LLVMExecutionEngineRef engine = NULL;
    char *error = NULL;
    if (LLVMCreateMCJITCompilerForModule(&engine, mod, NULL, 0, &error)) {
        fprintf(stderr, "LLVM MCJIT setup failed: %s\n", error ? error : "unknown");
        LLVMDisposeMessage(error);
        LLVMDisposeModule(mod);
        LLVMContextDispose(ctx);
        return 0;
    }
    if (error) {
        LLVMDisposeMessage(error);
    }
    uint64_t address = LLVMGetFunctionAddress(engine, entry_name);
    if (!address) {
        LLVMDisposeExecutionEngine(engine);
        LLVMContextDispose(ctx);
        return 0;
    }

    pipeline->ctx = ctx;
    pipeline->mod = mod;
    pipeline->engine = engine;
    pipeline->addr = (void *)(uintptr_t)address;
    return 1;
}

static void destroy_pipeline(pipeline_t *pipeline)
{
    if (!pipeline->ctx) {
        return;
    }
    LLVMDisposeExecutionEngine(pipeline->engine);
    LLVMContextDispose(pipeline->ctx);
    pipeline->ctx = NULL;
}

typedef int32_t (*llvm_fn1_t)(int32_t);
typedef int32_t (*llvm_fn2_t)(int32_t, int32_t);
typedef void (*llvm_fn2p_t)(int32_t *, int32_t);
typedef int32_t (*llvm_fn2i_t)(int32_t *, int32_t);
typedef void (*llvm_fn4_t)(int32_t *, int32_t *, int32_t *, int32_t);

typedef enum {
    LLVM_FN1,
    LLVM_FN2,
    LLVM_FN2P,
    LLVM_FN4,
    LLVM_FN2I,
} llvm_call_kind_t;

typedef struct {
    llvm_call_kind_t kind;
    void *addr;
} llvm_callable_t;

static void fill_desc(int32_t *values, long count)
{
    for (long i = 0; i < count; i++) {
        values[i] = (int32_t)(count - i);
    }
}

static int select_callable(const char *name, void *addr, llvm_callable_t *callable)
{
    if (!strcmp(name, "fibonacci")) {
        *callable = (llvm_callable_t){ .kind = LLVM_FN1, .addr = addr };
        return 1;
    }
    if (!strcmp(name, "gcd")) {
        *callable = (llvm_callable_t){ .kind = LLVM_FN2, .addr = addr };
        return 1;
    }
    if (!strcmp(name, "bubblesort")) {
        *callable = (llvm_callable_t){ .kind = LLVM_FN2P, .addr = addr };
        return 1;
    }
    if (!strcmp(name, "matrixmul")) {
        *callable = (llvm_callable_t){ .kind = LLVM_FN4, .addr = addr };
        return 1;
    }
    if (!strcmp(name, "smoke")) {
        *callable = (llvm_callable_t){ .kind = LLVM_FN2I, .addr = addr };
        return 1;
    }
    return 0;
}

static int32_t invoke_callable(llvm_callable_t callable,
                               int32_t *arr, int32_t *A, int32_t *B, int32_t *C)
{
    switch (callable.kind) {
        case LLVM_FN1:
            return ((llvm_fn1_t)callable.addr)(30);
        case LLVM_FN2:
            return ((llvm_fn2_t)callable.addr)(48, 18);
        case LLVM_FN2P:
            fill_desc(arr, 20);
            ((llvm_fn2p_t)callable.addr)(arr, 20);
            return 0;
        case LLVM_FN4:
            fill_desc(A, 100);
            fill_desc(B, 100);
            ((llvm_fn4_t)callable.addr)(A, B, C, 10);
            return 0;
        case LLVM_FN2I:
            fill_desc(arr, 20);
            return ((llvm_fn2i_t)callable.addr)(arr, 20);
    }
    return 0;
}

static int validate(const char *name, int32_t result,
                    const int32_t *arr, const int32_t *A,
                    const int32_t *B, const int32_t *C)
{
    if (!strcmp(name, "fibonacci")) {
        return result == 832040;
    }
    if (!strcmp(name, "gcd")) {
        return result == 6;
    }
    if (!strcmp(name, "smoke")) {
        return result == 1330;
    }
    if (!strcmp(name, "bubblesort")) {
        for (int i = 0; i < 20; i++) {
            if (arr[i] != i + 1) {
                return 0;
            }
        }
        return 1;
    }
    if (!strcmp(name, "matrixmul")) {
        for (int i = 0; i < 10; i++) {
            for (int j = 0; j < 10; j++) {
                int32_t expected = 0;
                for (int k = 0; k < 10; k++) {
                    expected += A[i * 10 + k] * B[k * 10 + j];
                }
                if (C[i * 10 + j] != expected) {
                    return 0;
                }
            }
        }
        return 1;
    }
    return 0;
}

static const char *entry_for_path(const char *path, char *name, size_t name_size)
{
    const char *base = strrchr(path, '/');
    base = base ? base + 1 : path;
    snprintf(name, name_size, "%s", base);
    char *dot = strrchr(name, '.');
    if (dot && !strcmp(dot, ".ll")) {
        *dot = 0;
    }
    if (!strcmp(name, "smoke")) {
        return "checksum";
    }
    if (!strcmp(name, "fibonacci") || !strcmp(name, "gcd") ||
        !strcmp(name, "bubblesort") || !strcmp(name, "matrixmul")) {
        return name;
    }
    return NULL;
}

static int run_execution(const char *name, const char *source,
                         const char *entry_name, long iterations,
                         double *ready_us, double *first_us, double *steady_us)
{
    double ready_start = now_sec();
    pipeline_t pipeline = {0};
    if (!build_pipeline(source, entry_name, &pipeline)) {
        return 0;
    }
    *ready_us = (now_sec() - ready_start) * 1e6;
    llvm_callable_t callable;
    if (!select_callable(name, pipeline.addr, &callable)) {
        destroy_pipeline(&pipeline);
        return 0;
    }

    int32_t arr[20], A[100], B[100], C[100];
    int32_t result = 0;
    memset(arr, 0xa5, sizeof(arr));
    memset(A, 0xa5, sizeof(A));
    memset(B, 0xa5, sizeof(B));
    memset(C, 0xa5, sizeof(C));

    double t0 = now_sec();
    result = invoke_callable(callable, arr, A, B, C);
    *first_us = (now_sec() - t0) * 1e6;
    for (int i = 0; i < 2; i++) {
        result = invoke_callable(callable, arr, A, B, C);
    }
    if (!validate(name, result, arr, A, B, C)) {
        fprintf(stderr, "LLVM result check failed for %s\n", name);
        destroy_pipeline(&pipeline);
        return 0;
    }

    double t1 = now_sec();
    for (long i = 0; i < iterations; i++) {
        result = invoke_callable(callable, arr, A, B, C);
    }
    double t2 = now_sec();
    if (!validate(name, result, arr, A, B, C)) {
        fprintf(stderr, "LLVM post-run result check failed for %s\n", name);
        destroy_pipeline(&pipeline);
        return 0;
    }
    volatile int32_t sink = result;
    (void)sink;
    *steady_us = (t2 - t1) / (double)iterations * 1e6;
    destroy_pipeline(&pipeline);
    return 1;
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "Usage: bench_llvm_lib <file.ll> [iterations]\n");
        return 1;
    }
    const char *path = argv[1];
    long iterations = argc >= 3 ? atol(argv[2]) : 100;
    if (iterations <= 0) {
        fprintf(stderr, "Invalid iteration count\n");
        return 1;
    }

    char name[64];
    const char *entry_name = entry_for_path(path, name, sizeof(name));
    if (!entry_name) {
        fprintf(stderr, "Unsupported LLVM workload: %s\n", path);
        return 1;
    }
    size_t file_size = 0;
    char *file_source = read_file(path, &file_size);
    if (!file_source) {
        return 1;
    }
    (void)file_size;
    char *source = algorithm_source(file_source);
    free(file_source);
    if (!source) {
        return 1;
    }

    LLVMInitializeX86TargetInfo();
    LLVMInitializeX86Target();
    LLVMInitializeX86TargetMC();
    LLVMInitializeX86AsmPrinter();

    long success = 0;
    double compile_ns = 0.0;
    double cleanup_ns = 0.0;
    for (long i = 0; i < iterations; i++) {
        double t0 = now_sec();
        pipeline_t pipeline = {0};
        if (build_pipeline(source, entry_name, &pipeline)) {
            double t1 = now_sec();
            compile_ns += (t1 - t0) * 1e9;
            success++;
            double t2 = now_sec();
            destroy_pipeline(&pipeline);
            cleanup_ns += (now_sec() - t2) * 1e9;
        }
    }
    if (success == 0) {
        fprintf(stderr, "All LLVM compilations failed\n");
        free(source);
        return 1;
    }

    double ready_us = -1.0;
    double first_us = -1.0;
    double steady_us = -1.0;
    if (!run_execution(name, source, entry_name, iterations, &ready_us,
                       &first_us, &steady_us)) {
        free(source);
        return 1;
    }

    double compile_us = compile_ns / (double)success / 1000.0;
    double cleanup_us = cleanup_ns / (double)success / 1000.0;
    printf("schema=1 status=ok backend=llvm requested=%ld success=%ld opt=default compile_us=%.3f ready_us=%.3f first_call_us=%.3f steady_us=%.3f cleanup_us=%.3f code_size=0\n",
           iterations, success, compile_us, ready_us, first_us, steady_us, cleanup_us);
    free(source);
    return 0;
}
