#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>
#include <time.h>

#include "mir-gen.h"

static double now_sec(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9;
}

static void MIR_NO_RETURN mir_error_handler(MIR_error_type_t type, const char *format, ...)
{
    fprintf(stderr, "MIR error %d: ", type);
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputc('\n', stderr);
    exit(1);
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
    size_t source_len = strlen(source);
    char *result = malloc(source_len + strlen("endmodule\n") + 1);
    if (!result) {
        return NULL;
    }

    size_t out_len = 0;
    const char *line = source;
    while (*line) {
        const char *line_end = strchr(line, '\n');
        size_t line_len = line_end ? (size_t)(line_end - line + 1) : strlen(line);
        const char *p = line;
        while (p < line + line_len && (*p == ' ' || *p == '\t')) {
            p++;
        }
        size_t name_len = line_len - (size_t)(p - line);
        if ((name_len >= 7 && !strncmp(p, "main_p:", 7)) ||
            (name_len >= 5 && !strncmp(p, "main:", 5))) {
            break;
        }
        if (!(name_len >= 11 && !strncmp(p, "export main", 11))) {
            memcpy(result + out_len, line, line_len);
            out_len += line_len;
        }
        line += line_len;
        if (!line_end) {
            break;
        }
    }
    strcpy(result + out_len, "endmodule\n");
    return result;
}

static MIR_item_t find_func(MIR_module_t module, const char *name)
{
    for (MIR_item_t item = DLIST_HEAD(MIR_item_t, module->items);
         item != NULL;
         item = DLIST_NEXT(MIR_item_t, item)) {
        if (item->item_type == MIR_func_item && !strcmp(item->u.func->name, name)) {
            return item;
        }
    }
    return NULL;
}

typedef struct {
    MIR_context_t ctx;
    MIR_item_t entry;
    void *addr;
} pipeline_t;

static int build_pipeline(const char *source, const char *entry_name,
                         unsigned optimize_level, pipeline_t *pipeline)
{
    MIR_context_t ctx = MIR_init();
    if (!ctx) {
        return 0;
    }
    MIR_set_error_func(ctx, mir_error_handler);
    MIR_scan_string(ctx, source);

    DLIST(MIR_module_t) *modules = MIR_get_module_list(ctx);
    for (MIR_module_t module = DLIST_HEAD(MIR_module_t, *modules);
         module != NULL;
         module = DLIST_NEXT(MIR_module_t, module)) {
        MIR_load_module(ctx, module);
    }
    MIR_load_external(ctx, "abort", (void *)abort);
    MIR_gen_init(ctx);
    MIR_gen_set_optimize_level(ctx, optimize_level);
    MIR_link(ctx, MIR_set_gen_interface, NULL);

    MIR_item_t entry = NULL;
    modules = MIR_get_module_list(ctx);
    for (MIR_module_t module = DLIST_HEAD(MIR_module_t, *modules);
         module != NULL && entry == NULL;
         module = DLIST_NEXT(MIR_module_t, module)) {
        entry = find_func(module, entry_name);
    }
    if (!entry || !entry->addr) {
        MIR_gen_finish(ctx);
        MIR_finish(ctx);
        return 0;
    }

    pipeline->ctx = ctx;
    pipeline->entry = entry;
    pipeline->addr = entry->addr;
    return 1;
}

static void destroy_pipeline(pipeline_t *pipeline)
{
    if (!pipeline->ctx) {
        return;
    }
    MIR_gen_finish(pipeline->ctx);
    MIR_finish(pipeline->ctx);
    pipeline->ctx = NULL;
}

typedef int64_t (*mir_fn1_t)(int64_t);
typedef int64_t (*mir_fn2_t)(int64_t, int64_t);
typedef void (*mir_fn2p_t)(int32_t *, int64_t);
typedef int64_t (*mir_fn2i_t)(int32_t *, int64_t);
typedef void (*mir_fn4_t)(int32_t *, int32_t *, int32_t *, int64_t);

typedef enum {
    MIR_FN1,
    MIR_FN2,
    MIR_FN2P,
    MIR_FN4,
    MIR_FN2I,
} mir_call_kind_t;

typedef struct {
    mir_call_kind_t kind;
    void *addr;
} mir_callable_t;

static void fill_desc(int32_t *values, long count)
{
    for (long i = 0; i < count; i++) {
        values[i] = (int32_t)(count - i);
    }
}

static int select_callable(const char *name, void *addr, mir_callable_t *callable)
{
    if (!strcmp(name, "fibonacci")) {
        *callable = (mir_callable_t){ .kind = MIR_FN1, .addr = addr };
        return 1;
    }
    if (!strcmp(name, "gcd")) {
        *callable = (mir_callable_t){ .kind = MIR_FN2, .addr = addr };
        return 1;
    }
    if (!strcmp(name, "bubblesort")) {
        *callable = (mir_callable_t){ .kind = MIR_FN2P, .addr = addr };
        return 1;
    }
    if (!strcmp(name, "matrixmul")) {
        *callable = (mir_callable_t){ .kind = MIR_FN4, .addr = addr };
        return 1;
    }
    if (!strcmp(name, "smoke")) {
        *callable = (mir_callable_t){ .kind = MIR_FN2I, .addr = addr };
        return 1;
    }
    return 0;
}

static int64_t invoke_callable(mir_callable_t callable,
                               int32_t *arr, int32_t *A, int32_t *B, int32_t *C)
{
    switch (callable.kind) {
        case MIR_FN1:
            return ((mir_fn1_t)callable.addr)(30);
        case MIR_FN2:
            return ((mir_fn2_t)callable.addr)(48, 18);
        case MIR_FN2P:
            fill_desc(arr, 20);
            ((mir_fn2p_t)callable.addr)(arr, 20);
            return 0;
        case MIR_FN4:
            fill_desc(A, 100);
            fill_desc(B, 100);
            ((mir_fn4_t)callable.addr)(A, B, C, 10);
            return 0;
        case MIR_FN2I:
            fill_desc(arr, 20);
            return ((mir_fn2i_t)callable.addr)(arr, 20);
    }
    return 0;
}

static int validate(const char *name, int64_t result,
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
    if (dot && !strcmp(dot, ".mir")) {
        *dot = 0;
    }
    if (!strcmp(name, "fibonacci")) {
        return "fib";
    }
    if (!strcmp(name, "smoke")) {
        return "checksum";
    }
    if (!strcmp(name, "gcd") || !strcmp(name, "bubblesort") ||
        !strcmp(name, "matrixmul")) {
        return name;
    }
    return NULL;
}

static int run_execution(const char *name, const char *source,
                         const char *entry_name, unsigned optimize_level,
                         long iterations, double *ready_us, double *first_us,
                         double *steady_us)
{
    double ready_start = now_sec();
    pipeline_t pipeline = {0};
    if (!build_pipeline(source, entry_name, optimize_level, &pipeline)) {
        return 0;
    }
    *ready_us = (now_sec() - ready_start) * 1e6;

    mir_callable_t callable;
    if (!select_callable(name, pipeline.addr, &callable)) {
        destroy_pipeline(&pipeline);
        return 0;
    }

    int32_t arr[20], A[100], B[100], C[100];
    int64_t result = 0;
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
        fprintf(stderr, "MIR result check failed for %s\n", name);
        destroy_pipeline(&pipeline);
        return 0;
    }

    double t1 = now_sec();
    for (long i = 0; i < iterations; i++) {
        result = invoke_callable(callable, arr, A, B, C);
    }
    double t2 = now_sec();
    if (!validate(name, result, arr, A, B, C)) {
        fprintf(stderr, "MIR post-run result check failed for %s\n", name);
        destroy_pipeline(&pipeline);
        return 0;
    }
    volatile int64_t sink = result;
    (void)sink;
    *steady_us = (t2 - t1) / (double)iterations * 1e6;
    destroy_pipeline(&pipeline);
    return 1;
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "Usage: bench_mir_lib <file.mir> [iterations] [opt-level]\n");
        return 1;
    }
    const char *path = argv[1];
    long iterations = argc >= 3 ? atol(argv[2]) : 100;
    unsigned optimize_level = argc >= 4 ? (unsigned)atoi(argv[3]) : 2;
    if (iterations <= 0 || optimize_level > 3) {
        fprintf(stderr, "Invalid iterations or optimization level\n");
        return 1;
    }

    char name[64];
    const char *entry_name = entry_for_path(path, name, sizeof(name));
    if (!entry_name) {
        fprintf(stderr, "Unsupported MIR workload: %s\n", path);
        return 1;
    }
    size_t source_size = 0;
    char *file_source = read_file(path, &source_size);
    if (!file_source) {
        return 1;
    }
    (void)source_size;
    char *source = algorithm_source(file_source);
    free(file_source);
    if (!source) {
        return 1;
    }

    long success = 0;
    double compile_ns = 0.0;
    double cleanup_ns = 0.0;
    for (long i = 0; i < iterations; i++) {
        double t0 = now_sec();
        pipeline_t pipeline = {0};
        if (build_pipeline(source, entry_name, optimize_level, &pipeline)) {
            double t1 = now_sec();
            compile_ns += (t1 - t0) * 1e9;
            success++;
            double t2 = now_sec();
            destroy_pipeline(&pipeline);
            cleanup_ns += (now_sec() - t2) * 1e9;
        }
    }
    if (success == 0) {
        fprintf(stderr, "All MIR compilations failed\n");
        free(source);
        return 1;
    }

    double ready_us = -1.0;
    double first_us = -1.0;
    double steady_us = -1.0;
    if (!run_execution(name, source, entry_name, optimize_level, iterations,
                       &ready_us, &first_us, &steady_us)) {
        free(source);
        return 1;
    }

    double compile_us = compile_ns / (double)success / 1000.0;
    double cleanup_us = cleanup_ns / (double)success / 1000.0;
    printf("schema=1 status=ok backend=mir requested=%ld success=%ld opt=%u compile_us=%.3f ready_us=%.3f first_call_us=%.3f steady_us=%.3f cleanup_us=%.3f code_size=0\n",
           iterations, success, optimize_level, compile_us, ready_us,
           first_us, steady_us, cleanup_us);
    free(source);
    return 0;
}
