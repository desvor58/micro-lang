#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
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

typedef struct {
    MIR_context_t ctx;
    MIR_item_t func;
    void *addr;
} vm_pipeline_t;

static int build_vm_pipeline(unsigned optimize_level, vm_pipeline_t *pipeline)
{
    MIR_context_t ctx = MIR_init();
    if (!ctx) {
        return 0;
    }
    MIR_set_error_func(ctx, mir_error_handler);

    MIR_module_t module = MIR_new_module(ctx, "vm");
    MIR_type_t result_type = MIR_T_I64;
    MIR_item_t func = MIR_new_func(ctx, "vm_sum", 1, &result_type, 0);
    MIR_reg_t sum = MIR_new_func_reg(ctx, func->u.func, MIR_T_I64, "sum");
    MIR_reg_t i = MIR_new_func_reg(ctx, func->u.func, MIR_T_I64, "i");
    MIR_label_t loop = MIR_new_label(ctx);
    MIR_label_t done = MIR_new_label(ctx);

    MIR_append_insn(ctx, func, MIR_new_insn(ctx, MIR_MOV,
                                             MIR_new_reg_op(ctx, sum),
                                             MIR_new_int_op(ctx, 0)));
    MIR_append_insn(ctx, func, MIR_new_insn(ctx, MIR_MOV,
                                             MIR_new_reg_op(ctx, i),
                                             MIR_new_int_op(ctx, 0)));
    MIR_append_insn(ctx, func, loop);
    MIR_append_insn(ctx, func, MIR_new_insn(ctx, MIR_BGE,
                                             MIR_new_label_op(ctx, done),
                                             MIR_new_reg_op(ctx, i),
                                             MIR_new_int_op(ctx, 100)));
    MIR_append_insn(ctx, func, MIR_new_insn(ctx, MIR_ADD,
                                             MIR_new_reg_op(ctx, sum),
                                             MIR_new_reg_op(ctx, sum),
                                             MIR_new_reg_op(ctx, i)));
    MIR_append_insn(ctx, func, MIR_new_insn(ctx, MIR_ADD,
                                             MIR_new_reg_op(ctx, i),
                                             MIR_new_reg_op(ctx, i),
                                             MIR_new_int_op(ctx, 1)));
    MIR_append_insn(ctx, func, MIR_new_insn(ctx, MIR_JMP,
                                             MIR_new_label_op(ctx, loop)));
    MIR_append_insn(ctx, func, done);
    MIR_append_insn(ctx, func, MIR_new_ret_insn(ctx, 1,
                                                 MIR_new_reg_op(ctx, sum)));
    MIR_finish_func(ctx);
    MIR_finish_module(ctx);

    MIR_load_module(ctx, module);
    MIR_gen_init(ctx);
    MIR_gen_set_optimize_level(ctx, optimize_level);
    MIR_link(ctx, MIR_set_gen_interface, NULL);
    if (!func->addr) {
        MIR_gen_finish(ctx);
        MIR_finish(ctx);
        return 0;
    }

    pipeline->ctx = ctx;
    pipeline->func = func;
    pipeline->addr = func->addr;
    return 1;
}

static void destroy_vm_pipeline(vm_pipeline_t *pipeline)
{
    if (!pipeline->ctx) {
        return;
    }
    MIR_gen_finish(pipeline->ctx);
    MIR_finish(pipeline->ctx);
    pipeline->ctx = NULL;
}

static int run_execution(unsigned optimize_level, long iterations,
                         double *ready_us, double *first_us, double *steady_us)
{
    double ready_start = now_sec();
    vm_pipeline_t pipeline = {0};
    if (!build_vm_pipeline(optimize_level, &pipeline)) {
        return 0;
    }
    *ready_us = (now_sec() - ready_start) * 1e6;
    typedef int64_t (*vm_fn_t)(void);
    vm_fn_t fn = (vm_fn_t)pipeline.addr;
    volatile int64_t sink = fn();
    (void)sink;

    double t0 = now_sec();
    int64_t result = fn();
    *first_us = (now_sec() - t0) * 1e6;
    for (int i = 0; i < 2; i++) {
        result = fn();
    }
    if (result != 4950) {
        fprintf(stderr, "MIR API result check failed: %lld\n", (long long)result);
        destroy_vm_pipeline(&pipeline);
        return 0;
    }

    double t1 = now_sec();
    for (long i = 0; i < iterations; i++) {
        result = fn();
    }
    double t2 = now_sec();
    sink = result;
    (void)sink;
    *steady_us = (t2 - t1) / (double)iterations * 1e6;
    destroy_vm_pipeline(&pipeline);
    return 1;
}

int main(int argc, char **argv)
{
    long iterations = argc >= 2 ? atol(argv[1]) : 100;
    unsigned optimize_level = argc >= 3 ? (unsigned)atoi(argv[2]) : 2;
    if (iterations <= 0 || optimize_level > 3) {
        fprintf(stderr, "Usage: bench_mir_api [iterations] [opt-level]\n");
        return 1;
    }

    long success = 0;
    double compile_ns = 0.0;
    double cleanup_ns = 0.0;
    for (long i = 0; i < iterations; i++) {
        double t0 = now_sec();
        vm_pipeline_t pipeline = {0};
        if (build_vm_pipeline(optimize_level, &pipeline)) {
            double t1 = now_sec();
            compile_ns += (t1 - t0) * 1e9;
            success++;
            double t2 = now_sec();
            destroy_vm_pipeline(&pipeline);
            cleanup_ns += (now_sec() - t2) * 1e9;
        }
    }
    if (success == 0) {
        fprintf(stderr, "All MIR API compilations failed\n");
        return 1;
    }

    double ready_us = -1.0;
    double first_us = -1.0;
    double steady_us = -1.0;
    if (!run_execution(optimize_level, iterations, &ready_us, &first_us, &steady_us)) {
        return 1;
    }

    double compile_us = compile_ns / (double)success / 1000.0;
    double cleanup_us = cleanup_ns / (double)success / 1000.0;
    printf("schema=1 status=ok backend=mir-api target=x86-64 workload=vm_sum requested=%ld success=%ld opt=%u compile_us=%.3f ready_us=%.3f first_call_us=%.3f steady_us=%.3f cleanup_us=%.3f code_size=0\n",
           iterations, success, optimize_level, compile_us, ready_us,
           first_us, steady_us, cleanup_us);
    return 0;
}
