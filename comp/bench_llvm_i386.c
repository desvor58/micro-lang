#define _POSIX_C_SOURCE 199309L
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

extern int32_t fibonacci(int32_t);
extern int32_t gcd(int32_t, int32_t);
extern void bubblesort(int32_t *, int32_t);
extern void matrixmul(int32_t *, int32_t *, int32_t *, int32_t);
extern int32_t checksum(int32_t *, int32_t);

static double now_sec(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9;
}

static void fill_desc(int32_t *values, long count)
{
    for (long i = 0; i < count; i++) {
        values[i] = (int32_t)(count - i);
    }
}

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

static int select_callable(const char *name, llvm_callable_t *callable)
{
    if (!strcmp(name, "fibonacci")) {
        *callable = (llvm_callable_t){ LLVM_FN1, (void *)fibonacci };
        return 1;
    }
    if (!strcmp(name, "gcd")) {
        *callable = (llvm_callable_t){ LLVM_FN2, (void *)gcd };
        return 1;
    }
    if (!strcmp(name, "bubblesort")) {
        *callable = (llvm_callable_t){ LLVM_FN2P, (void *)bubblesort };
        return 1;
    }
    if (!strcmp(name, "matrixmul")) {
        *callable = (llvm_callable_t){ LLVM_FN4, (void *)matrixmul };
        return 1;
    }
    if (!strcmp(name, "smoke")) {
        *callable = (llvm_callable_t){ LLVM_FN2I, (void *)checksum };
        return 1;
    }
    return 0;
}

static int32_t invoke_callable(llvm_callable_t callable,
                               int32_t *arr, int32_t *A, int32_t *B, int32_t *C)
{
    switch (callable.kind) {
        case LLVM_FN1:
            return ((int32_t (*)(int32_t))callable.addr)(30);
        case LLVM_FN2:
            return ((int32_t (*)(int32_t, int32_t))callable.addr)(48, 18);
        case LLVM_FN2P:
            fill_desc(arr, 20);
            ((void (*)(int32_t *, int32_t))callable.addr)(arr, 20);
            return 0;
        case LLVM_FN4:
            fill_desc(A, 100);
            fill_desc(B, 100);
            ((void (*)(int32_t *, int32_t *, int32_t *, int32_t))callable.addr)(A, B, C, 10);
            return 0;
        case LLVM_FN2I:
            fill_desc(arr, 20);
            return ((int32_t (*)(int32_t *, int32_t))callable.addr)(arr, 20);
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

int main(int argc, char **argv)
{
    if (argc < 2 || argc > 4) {
        fprintf(stderr, "Usage: bench_llvm_i386 <fibonacci|bubblesort|matrixmul|gcd|smoke> [iterations] [O0|O2]\n");
        return 1;
    }
    const char *name = argv[1];
    long iterations = argc >= 3 ? atol(argv[2]) : 100;
    const char *opt = argc >= 4 ? argv[3] : "O2";
    if (iterations <= 0 || (strcmp(opt, "O0") && strcmp(opt, "O2"))) {
        fprintf(stderr, "Invalid iterations or optimization level\n");
        return 1;
    }

    llvm_callable_t callable;
    if (!select_callable(name, &callable)) {
        fprintf(stderr, "Unsupported LLVM workload: %s\n", name);
        return 1;
    }

    int32_t arr[20], A[100], B[100], C[100];
    int32_t result = 0;
    memset(arr, 0xa5, sizeof(arr));
    memset(A, 0xa5, sizeof(A));
    memset(B, 0xa5, sizeof(B));
    memset(C, 0xa5, sizeof(C));

    double first_start = now_sec();
    result = invoke_callable(callable, arr, A, B, C);
    double first_us = (now_sec() - first_start) * 1e6;
    for (int i = 0; i < 2; i++) {
        result = invoke_callable(callable, arr, A, B, C);
    }
    if (!validate(name, result, arr, A, B, C)) {
        fprintf(stderr, "LLVM i386 result check failed for %s\n", name);
        return 1;
    }

    double steady_start = now_sec();
    for (long i = 0; i < iterations; i++) {
        result = invoke_callable(callable, arr, A, B, C);
    }
    double steady_us = (now_sec() - steady_start) * 1e6 / (double)iterations;
    if (!validate(name, result, arr, A, B, C)) {
        fprintf(stderr, "LLVM i386 post-run result check failed for %s\n", name);
        return 1;
    }

    volatile int32_t sink = result;
    (void)sink;
    printf("schema=1 status=ok backend=llvm-i386 target=i386 workload=%s requested=%ld success=%ld opt=%s compile_us=-1 ready_us=-1 first_call_us=%.3f steady_us=%.3f cleanup_us=0 code_size=0\n",
           name, iterations, iterations, opt, first_us, steady_us);
    return 0;
}
