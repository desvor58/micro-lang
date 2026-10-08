#define _DEFAULT_SOURCE   /* mmap / MAP_ANONYMOUS (not in POSIX.1-1993) */
#define _POSIX_C_SOURCE 199309L
/*************************************************
 *  bench_micro_lib.c
 *  Library benchmark: build IR via micro_instr_gen_* API
 *  (micro/instr.h, included via micro.h), then compile
 *  through codegen → asm optimization → assembling.
 *
 *  Does NOT use microc internals (no mc_tokenize/mc_instrgen).
 *
 *  Usage: bench_micro_lib <fibonacci|bubblesort|matrixmul|gcd|smoke|hints> [iterations] [plain|hinted]
 *
 *  Prints one key/value record.  The 32-bit build measures API compilation,
 *  VM metadata time, executable allocation/copy, first call and steady calls.
 *  The 64-bit build reports compile stages and leaves execution fields absent.
 ************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <sys/mman.h>
#include <unistd.h>
#include <micro/micro.h>

static double now_sec(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9;
}

typedef struct {
    long irgen;
    long vmopt;
    long codegen;
    long asmopt;
    long emit;
    long cleanup;
} stage_times_t;

/*
 * Program handle: top-level instruction vector + all auxiliary vectors.
 *
 * micro_instr_gen_* store RAW POINTERS into expr/args/body vectors
 * (shallow copy), so every vector handed to the API must stay alive
 * until codegen finishes. `keep` tracks them for teardown.
 */
typedef struct {
    sct_vector_t instrs;  /* micro_instruction_t — top level */
    sct_vector_t keep;    /* sct_vector_t* — vectors owned by this program */
} prog_t;

static prog_t *prog_new(void)
{
    prog_t *p = malloc(sizeof(prog_t));
    sct_vector_init(&p->instrs, sizeof(micro_instruction_t));
    sct_vector_init(&p->keep, sizeof(sct_vector_t *));
    return p;
}

/* allocate an owned vector; caller sets element size via elem_size */
static sct_vector_t *prog_vec(prog_t *p, size_t elem_size)
{
    sct_vector_t *v = malloc(sizeof(sct_vector_t));
    sct_vector_init(v, elem_size);
    sct_vector_push(&p->keep, &v);
    return v;
}

/* expression vector built via micro_make_expr (library API) */
static sct_vector_t *prog_expr(prog_t *p, const char *str)
{
    sct_vector_t *v = prog_vec(p, sizeof(micro_expr_tok_t));
    size_t errs_before = micro_err_stk_size;
    micro_make_expr(v, str);
    if (micro_err_stk_size != errs_before) {
        fprintf(stderr, "micro_make_expr failed for \"%s\" (%zu errors)\n",
                str, micro_err_stk_size - errs_before);
    }
    return v;
}

/*
 * call arguments: micro_instr_gen_call expects sct_vector_t of
 * micro_expr_tok_t* — one pointer per argument (see mc_instrgen:
 * src/microc/instrgen/statements/call.c), each pointing to the start
 * of a self-delimiting prefix expression.
 */
static sct_vector_t *prog_call_args(prog_t *p, const char **strs, size_t count)
{
    sct_vector_t *v = prog_vec(p, sizeof(micro_expr_tok_t *));
    for (size_t i = 0; i < count; i++) {
        sct_vector_t *e = prog_expr(p, strs[i]);
        micro_expr_tok_t *tok = (micro_expr_tok_t *)e->data;
        sct_vector_push(v, &tok);
    }
    return v;
}

static void prog_free(prog_t *p)
{
    for (size_t i = 0; i < p->keep.size; i++) {
        sct_vector_t *v = *(sct_vector_t **)sct_vector_get(&p->keep, i);
        sct_vector_deinit(v);
        free(v);
    }
    sct_vector_deinit(&p->keep);
    sct_vector_deinit(&p->instrs);
    free(p);
}

static micro_instruction_hints_t vm_default_hints(void)
{
    return (micro_instruction_hints_t){
        .lifetime = -1,
        .forced_stack = 0,
        .lazy_init = 0,
    };
}

#define micro_instr_gen_set(instrs, type, name, expr) \
    micro_instr_gen_set((instrs), (type), (name), (expr), vm_default_hints())
#define micro_instr_gen_drset(instrs, type, name, expr) \
    micro_instr_gen_drset((instrs), (type), (name), (expr), vm_default_hints())
#define micro_instr_gen_fun(instrs, name, args, ret_type, body) \
    micro_instr_gen_fun((instrs), (name), (args), (ret_type), (body), vm_default_hints())
#define micro_instr_gen_ret(instrs, expr) \
    micro_instr_gen_ret((instrs), (expr), vm_default_hints())
#define micro_instr_gen_call(instrs, reg_name, fun_name, args) \
    micro_instr_gen_call((instrs), (reg_name), (fun_name), (args), vm_default_hints())
#define micro_instr_gen_lbl(instrs, name) \
    micro_instr_gen_lbl((instrs), (name), vm_default_hints())
#define micro_instr_gen_goto(instrs, name) \
    micro_instr_gen_goto((instrs), (name), vm_default_hints())
#define micro_instr_gen_if(instrs, cond_expr, lbl_name) \
    micro_instr_gen_if((instrs), (cond_expr), (lbl_name), vm_default_hints())

/* same meaning as a source hint: alive for this many further instructions */
static int prog_set_lifetime(sct_vector_t *instrs, micro_type_t type,
                             const char *name, sct_vector_t *expr,
                             ptrdiff_t lifetime)
{
    micro_instruction_hints_t hints = {
        .lifetime = (ptrdiff_t)instrs->size + lifetime,
        .forced_stack = 0,
        .lazy_init = 0,
    };
    return (micro_instr_gen_set)(instrs, type, name, expr, hints);
}

static size_t vm_expr_arity(micro_expr_tok_type_t type)
{
    switch (type) {
        case MICRO_EXPR_TOK_PLUS:
        case MICRO_EXPR_TOK_MINUS:
        case MICRO_EXPR_TOK_STAR:
        case MICRO_EXPR_TOK_SLASH:
        case MICRO_EXPR_TOK_EQ:
        case MICRO_EXPR_TOK_GREAT:
        case MICRO_EXPR_TOK_LESS:
        case MICRO_EXPR_TOK_GREAT_OR_EQ:
        case MICRO_EXPR_TOK_LESS_OR_EQ:
            return 2;
        case MICRO_EXPR_TOK_AMPERSAND:
        case MICRO_EXPR_TOK_DOLLAR:
        case MICRO_EXPR_TOK_HASH:
        case MICRO_EXPR_TOK_APOSTROPHE:
        case MICRO_EXPR_TOK_TILDE:
        case MICRO_EXPR_TOK_EXCLAMATION:
            return 1;
        default:
            return 0;
    }
}

static size_t vm_expr_visit(micro_expr_tok_t *expr, const char *name, size_t index, size_t *last_use)
{
    if (!expr) {
        return 0;
    }
    if (expr->type == MICRO_EXPR_TOK_IDENT) {
        if (!strcmp(expr->val, name)) {
            *last_use = index;
        }
        return 1;
    }
    if (_micro_expr_is_lit(expr->type)) {
        return 1;
    }

    size_t arity = vm_expr_arity(expr->type);
    if (!arity) {
        return 1;
    }

    size_t size = 1;
    for (size_t i = 0; i < arity; i++) {
        size_t child_size = vm_expr_visit(expr + size, name, index, last_use);
        if (!child_size) {
            return 0;
        }
        size += child_size;
    }
    return size;
}

static void vm_instruction_uses(micro_instruction_t *instr, const char *name, size_t index, size_t *last_use)
{
    switch (instr->type) {
        case MICRO_INSTR_SET:
            vm_expr_visit(instr->set.val_expr, name, index, last_use);
            break;
        case MICRO_INSTR_DRSET:
            vm_expr_visit(instr->drset.val_expr, name, index, last_use);
            break;
        case MICRO_INSTR_RET:
            vm_expr_visit(instr->ret.val_expr, name, index, last_use);
            break;
        case MICRO_INSTR_IF:
            vm_expr_visit(instr->if_goto.cond_expr, name, index, last_use);
            break;
        case MICRO_INSTR_CALL:
            for (size_t i = 0; i < instr->call.arg_exprs.size; i++) {
                micro_expr_tok_t **arg = sct_vector_get(&instr->call.arg_exprs, i);
                if (arg && *arg) {
                    vm_expr_visit(*arg, name, index, last_use);
                }
            }
            break;
        default:
            break;
    }
}

static void vm_hint_body(sct_vector_t *body)
{
    for (size_t i = 0; i < body->size; i++) {
        micro_instruction_t *instr = sct_vector_get(body, i);
        if (instr->type != MICRO_INSTR_SET && instr->type != MICRO_INSTR_DRSET) {
            continue;
        }

        const char *name = instr->type == MICRO_INSTR_SET
            ? instr->set.reg_name
            : instr->drset.reg_name;
        size_t last_use = i;
        for (size_t j = i; j < body->size; j++) {
            micro_instruction_t *use_instr = sct_vector_get(body, j);
            vm_instruction_uses(use_instr, name, j, &last_use);
        }
        instr->hints.lifetime = (ptrdiff_t)last_use;
    }
}

static void vm_hint_program(prog_t *p)
{
    for (size_t i = 0; i < p->instrs.size; i++) {
        micro_instruction_t *instr = sct_vector_get(&p->instrs, i);
        if (instr->type == MICRO_INSTR_FUN) {
            vm_hint_body(&instr->fun.body);
        }
    }
}

/* ---- program builders ---- */

static prog_t *build_fibonacci(void)
{
    prog_t *p = prog_new();
    sct_vector_t *args = prog_vec(p, sizeof(micro_instruction_fun_arg_t));
    sct_vector_t *body = prog_vec(p, sizeof(micro_instruction_t));

    sct_vector_push(args, &(micro_instruction_fun_arg_t){ .type = MICRO_TYPE_I32, .name = "n" });

    micro_instr_gen_set(body, MICRO_TYPE_I32, "cmp", prog_expr(p, "<= n 1"));
    micro_instr_gen_if(body, prog_expr(p, "cmp"), "base");

    micro_instr_gen_set(body, MICRO_TYPE_I32, "n1", prog_expr(p, "- n 1"));
    micro_instr_gen_set(body, MICRO_TYPE_I32, "a", NULL);
    micro_instr_gen_call(body, "a", "fibonacci", prog_call_args(p, (const char *[]){"n1"}, 1));

    micro_instr_gen_set(body, MICRO_TYPE_I32, "n2", prog_expr(p, "- n 2"));
    micro_instr_gen_set(body, MICRO_TYPE_I32, "b", NULL);
    micro_instr_gen_call(body, "b", "fibonacci", prog_call_args(p, (const char *[]){"n2"}, 1));

    micro_instr_gen_set(body, MICRO_TYPE_I32, "result", prog_expr(p, "+ a b"));
    micro_instr_gen_ret(body, prog_expr(p, "result"));

    micro_instr_gen_lbl(body, "base");
    micro_instr_gen_ret(body, prog_expr(p, "n"));

    micro_instr_gen_fun(&p->instrs, "fibonacci", args, MICRO_TYPE_I32, body);
    return p;
}

static prog_t *build_gcd(void)
{
    prog_t *p = prog_new();
    sct_vector_t *args = prog_vec(p, sizeof(micro_instruction_fun_arg_t));
    sct_vector_t *body = prog_vec(p, sizeof(micro_instruction_t));

    sct_vector_push(args, &(micro_instruction_fun_arg_t){ .type = MICRO_TYPE_I32, .name = "a" });
    sct_vector_push(args, &(micro_instruction_fun_arg_t){ .type = MICRO_TYPE_I32, .name = "b" });

    micro_instr_gen_set(body, MICRO_TYPE_I32, "bz", prog_expr(p, "= b 0"));
    micro_instr_gen_if(body, prog_expr(p, "bz"), "done");

    micro_instr_gen_set(body, MICRO_TYPE_I32, "q", prog_expr(p, "/ a b"));
    micro_instr_gen_set(body, MICRO_TYPE_I32, "remv", prog_expr(p, "* q b"));
    micro_instr_gen_set(body, MICRO_TYPE_I32, "rem", prog_expr(p, "- a remv"));

    micro_instr_gen_set(body, MICRO_TYPE_I32, "result", NULL);
    micro_instr_gen_call(body, "result", "gcd", prog_call_args(p, (const char *[]){"b", "rem"}, 2));
    micro_instr_gen_ret(body, prog_expr(p, "result"));

    micro_instr_gen_lbl(body, "done");
    micro_instr_gen_ret(body, prog_expr(p, "a"));

    micro_instr_gen_fun(&p->instrs, "gcd", args, MICRO_TYPE_I32, body);
    return p;
}

static prog_t *build_bubblesort(void)
{
    prog_t *p = prog_new();
    sct_vector_t *args = prog_vec(p, sizeof(micro_instruction_fun_arg_t));
    sct_vector_t *body = prog_vec(p, sizeof(micro_instruction_t));

    sct_vector_push(args, &(micro_instruction_fun_arg_t){ .type = MICRO_TYPE_PTR, .name = "arr" });
    sct_vector_push(args, &(micro_instruction_fun_arg_t){ .type = MICRO_TYPE_I32, .name = "n" });

    micro_instr_gen_set(body, MICRO_TYPE_I32, "i", prog_expr(p, "0"));

    micro_instr_gen_lbl(body, "outer_loop");
    micro_instr_gen_if(body, prog_expr(p, ">= i - n 1"), "done");

    micro_instr_gen_set(body, MICRO_TYPE_I32, "j", prog_expr(p, "0"));

    micro_instr_gen_lbl(body, "inner_loop");
    micro_instr_gen_if(body, prog_expr(p, ">= j - - n i 1"), "inner_done");

    prog_set_lifetime(body, MICRO_TYPE_I32, "addr_j", prog_expr(p, "+ arr * j 4"), 6);
    prog_set_lifetime(body, MICRO_TYPE_I32, "addr_j1", prog_expr(p, "+ arr * + j 1 4"), 6);
    prog_set_lifetime(body, MICRO_TYPE_I32, "val_j", prog_expr(p, "$ addr_j"), 4);
    prog_set_lifetime(body, MICRO_TYPE_I32, "val_j1", prog_expr(p, "$ addr_j1"), 3);
    micro_instr_gen_if(body, prog_expr(p, "<= val_j val_j1"), "no_swap");

    micro_instr_gen_drset(body, MICRO_TYPE_I32, "addr_j", prog_expr(p, "val_j1"));
    micro_instr_gen_drset(body, MICRO_TYPE_I32, "addr_j1", prog_expr(p, "val_j"));

    micro_instr_gen_lbl(body, "no_swap");
    micro_instr_gen_set(body, MICRO_TYPE_I32, "j", prog_expr(p, "+ j 1"));
    micro_instr_gen_goto(body, "inner_loop");

    micro_instr_gen_lbl(body, "inner_done");
    micro_instr_gen_set(body, MICRO_TYPE_I32, "i", prog_expr(p, "+ i 1"));
    micro_instr_gen_goto(body, "outer_loop");

    micro_instr_gen_lbl(body, "done");
    micro_instr_gen_ret(body, NULL);

    micro_instr_gen_fun(&p->instrs, "bubblesort", args, MICRO_TYPE_NULL, body);
    return p;
}

static prog_t *build_matrixmul(void)
{
    prog_t *p = prog_new();
    sct_vector_t *args = prog_vec(p, sizeof(micro_instruction_fun_arg_t));
    sct_vector_t *body = prog_vec(p, sizeof(micro_instruction_t));

    sct_vector_push(args, &(micro_instruction_fun_arg_t){ .type = MICRO_TYPE_PTR, .name = "A" });
    sct_vector_push(args, &(micro_instruction_fun_arg_t){ .type = MICRO_TYPE_PTR, .name = "B" });
    sct_vector_push(args, &(micro_instruction_fun_arg_t){ .type = MICRO_TYPE_PTR, .name = "C" });
    sct_vector_push(args, &(micro_instruction_fun_arg_t){ .type = MICRO_TYPE_I32, .name = "N" });

    micro_instr_gen_set(body, MICRO_TYPE_I32, "i", prog_expr(p, "0"));

    micro_instr_gen_lbl(body, "row_loop");
    micro_instr_gen_set(body, MICRO_TYPE_I32, "done_cmp", prog_expr(p, ">= i N"));
    micro_instr_gen_if(body, prog_expr(p, "done_cmp"), "done");

    micro_instr_gen_set(body, MICRO_TYPE_I32, "j", prog_expr(p, "0"));

    micro_instr_gen_lbl(body, "col_loop");
    micro_instr_gen_set(body, MICRO_TYPE_I32, "col_cmp", prog_expr(p, ">= j N"));
    micro_instr_gen_if(body, prog_expr(p, "col_cmp"), "next_row");

    micro_instr_gen_set(body, MICRO_TYPE_I32, "sum", prog_expr(p, "0"));
    micro_instr_gen_set(body, MICRO_TYPE_I32, "k", prog_expr(p, "0"));

    micro_instr_gen_set(body, MICRO_TYPE_I32, "row_idx", prog_expr(p, "* i N"));
    micro_instr_gen_set(body, MICRO_TYPE_I32, "row_off", prog_expr(p, "* row_idx 4"));

    micro_instr_gen_lbl(body, "dot_loop");
    micro_instr_gen_set(body, MICRO_TYPE_I32, "dot_cmp", prog_expr(p, ">= k N"));
    micro_instr_gen_if(body, prog_expr(p, "dot_cmp"), "store");

    micro_instr_gen_set(body, MICRO_TYPE_I32, "col_off", prog_expr(p, "* k 4"));
    micro_instr_gen_set(body, MICRO_TYPE_I32, "tmp1", prog_expr(p, "+ A row_off"));
    micro_instr_gen_set(body, MICRO_TYPE_I32, "addr_a", prog_expr(p, "+ tmp1 col_off"));
    micro_instr_gen_set(body, MICRO_TYPE_I32, "k_row", prog_expr(p, "* k N"));
    micro_instr_gen_set(body, MICRO_TYPE_I32, "k_row_off", prog_expr(p, "* k_row 4"));
    micro_instr_gen_set(body, MICRO_TYPE_I32, "tmp2", prog_expr(p, "+ B k_row_off"));
    micro_instr_gen_set(body, MICRO_TYPE_I32, "j_off", prog_expr(p, "* j 4"));
    micro_instr_gen_set(body, MICRO_TYPE_I32, "addr_b", prog_expr(p, "+ tmp2 j_off"));

    micro_instr_gen_set(body, MICRO_TYPE_I32, "va", prog_expr(p, "$ addr_a"));
    micro_instr_gen_set(body, MICRO_TYPE_I32, "vb", prog_expr(p, "$ addr_b"));
    micro_instr_gen_set(body, MICRO_TYPE_I32, "prod", prog_expr(p, "* va vb"));
    micro_instr_gen_set(body, MICRO_TYPE_I32, "sum", prog_expr(p, "+ sum prod"));

    micro_instr_gen_set(body, MICRO_TYPE_I32, "k", prog_expr(p, "+ k 1"));
    micro_instr_gen_goto(body, "dot_loop");

    micro_instr_gen_lbl(body, "store");
    micro_instr_gen_set(body, MICRO_TYPE_I32, "j_off", prog_expr(p, "* j 4"));
    micro_instr_gen_set(body, MICRO_TYPE_I32, "tmp3", prog_expr(p, "+ C row_off"));
    micro_instr_gen_set(body, MICRO_TYPE_I32, "addr_c", prog_expr(p, "+ tmp3 j_off"));
    micro_instr_gen_drset(body, MICRO_TYPE_I32, "addr_c", prog_expr(p, "sum"));

    micro_instr_gen_set(body, MICRO_TYPE_I32, "j", prog_expr(p, "+ j 1"));
    micro_instr_gen_goto(body, "col_loop");

    micro_instr_gen_lbl(body, "next_row");
    micro_instr_gen_set(body, MICRO_TYPE_I32, "i", prog_expr(p, "+ i 1"));
    micro_instr_gen_goto(body, "row_loop");

    micro_instr_gen_lbl(body, "done");
    micro_instr_gen_ret(body, NULL);

    micro_instr_gen_fun(&p->instrs, "matrixmul", args, MICRO_TYPE_NULL, body);
    return p;
}

static prog_t *build_hints(void)
{
    prog_t *p = prog_new();
    sct_vector_t *body = prog_vec(p, sizeof(micro_instruction_t));

    micro_instr_gen_set(body, MICRO_TYPE_I32, "sum", prog_expr(p, "0"));
    micro_instr_gen_set(body, MICRO_TYPE_I32, "i", prog_expr(p, "0"));

    micro_instr_gen_lbl(body, "loop");
    micro_instr_gen_set(body, MICRO_TYPE_I32, "limit", prog_expr(p, "100"));
    micro_instr_gen_set(body, MICRO_TYPE_I32, "done_cmp", prog_expr(p, ">= i limit"));
    micro_instr_gen_if(body, prog_expr(p, "done_cmp"), "done");

    micro_instr_gen_set(body, MICRO_TYPE_I32, "sum", prog_expr(p, "+ sum i"));
    micro_instr_gen_set(body, MICRO_TYPE_I32, "i", prog_expr(p, "+ i 1"));
    micro_instr_gen_goto(body, "loop");

    micro_instr_gen_lbl(body, "done");
    micro_instr_gen_ret(body, prog_expr(p, "sum"));

    micro_instr_gen_fun(&p->instrs, "hints", prog_vec(p, sizeof(micro_instruction_fun_arg_t)), MICRO_TYPE_I32, body);
    return p;
}

static prog_t *build_smoke(void)
{
    prog_t *p = prog_new();
    sct_vector_t *args = prog_vec(p, sizeof(micro_instruction_fun_arg_t));
    sct_vector_t *body = prog_vec(p, sizeof(micro_instruction_t));

    sct_vector_push(args, &(micro_instruction_fun_arg_t){ .type = MICRO_TYPE_PTR, .name = "data" });
    sct_vector_push(args, &(micro_instruction_fun_arg_t){ .type = MICRO_TYPE_I32, .name = "n" });

    micro_instr_gen_set(body, MICRO_TYPE_I32, "acc", prog_expr(p, "0"));
    micro_instr_gen_set(body, MICRO_TYPE_I32, "i", prog_expr(p, "0"));
    micro_instr_gen_set(body, MICRO_TYPE_I32, "addr", prog_expr(p, "data"));

    micro_instr_gen_lbl(body, "loop");
    micro_instr_gen_if(body, prog_expr(p, ">= i n"), "done");

    micro_instr_gen_set(body, MICRO_TYPE_I32, "word", prog_expr(p, "$ addr"));
    micro_instr_gen_set(body, MICRO_TYPE_I32, "acc", prog_expr(p, "+ acc * i word"));

    micro_instr_gen_set(body, MICRO_TYPE_I32, "addr", prog_expr(p, "+ addr 4"));
    micro_instr_gen_set(body, MICRO_TYPE_I32, "i", prog_expr(p, "+ i 1"));
    micro_instr_gen_goto(body, "loop");

    micro_instr_gen_lbl(body, "done");
    micro_instr_gen_ret(body, prog_expr(p, "acc"));

    micro_instr_gen_fun(&p->instrs, "checksum", args, MICRO_TYPE_I32, body);
    return p;
}

/* ---- compile pipeline with stage timing ---- */
static int compile_prog(prog_t *p, sct_vector_t *outbuf, stage_times_t *st)
{
    double t;

    sct_vector_t asm_instrs;
    sct_vector_init(&asm_instrs, sizeof(micro_asm386_instruction_t));
    sct_arena_t arena;
    sct_arena_init(&arena);

    micro_codegen_t cg;
    micro_codegen_flags_t flags = {0};

    t = now_sec();
    micro_codegen386_init(&cg, flags, &asm_instrs, &arena, NULL);
    cg.emit(&cg, &p->instrs);
    st->codegen += (long)((now_sec() - t) * 1e9);

    if (micro_err_stk_size) {
        fprintf(stderr, "Codegen failed (%zu errors)\n", micro_err_stk_size);
        micro_codegen386_deinit(&cg);
        sct_arena_deinit(&arena);
        sct_vector_deinit(&asm_instrs);
        return 1;
    }

    t = now_sec();
    micro_asm386_optimize(&asm_instrs);
    st->asmopt += (long)((now_sec() - t) * 1e9);

    t = now_sec();
    micro_asm386_emit(&asm_instrs, outbuf);
    st->emit += (long)((now_sec() - t) * 1e9);

    t = now_sec();
    micro_codegen386_deinit(&cg);
    sct_arena_deinit(&arena);
    sct_vector_deinit(&asm_instrs);
    st->cleanup += (long)((now_sec() - t) * 1e9);

    return 0;
}

/* ---- execution phase (32-bit builds only) -------------------------------
 *
 * The compiled blob is raw x86-32 cdecl machine code — the same format
 * micro_runner loads — so only a 32-bit process can call it directly.
 * The workloads mirror the .ll/.mir mains; buffer setup happens inside
 * the timed span so every call does the same work as one MIR/LLVM main().
 */

typedef int32_t (*exec_fn0_t)(void);
typedef int32_t (*exec_fn1_t)(int32_t);
typedef int32_t (*exec_fn2_t)(int32_t, int32_t);
typedef void (*exec_fn2p_t)(int32_t *, int32_t);
typedef int32_t (*exec_fn2i_t)(int32_t *, int32_t);
typedef void (*exec_fn4_t)(int32_t *, int32_t *, int32_t *, int32_t);

typedef enum {
    EXEC_FN0,
    EXEC_FN1,
    EXEC_FN2,
    EXEC_FN2P,
    EXEC_FN4,
    EXEC_FN2I,
} exec_kind_t;

typedef struct {
    exec_kind_t kind;
    void *code;
} callable_t;

typedef struct {
    double ready_us;
    double load_us;
    double first_call_us;
    double steady_us;
    size_t code_size;
} exec_stats_t;

static void fill_desc(int32_t *w, long n)
{
    for (long k = 0; k < n; k++) w[k] = (int32_t)(n - k);
}

static int selectable_callable(const char *prog, void *code, callable_t *out)
{
    if (!strcmp(prog, "hints")) {
        *out = (callable_t){ .kind = EXEC_FN0, .code = code };
        return 1;
    }
    if (!strcmp(prog, "fibonacci")) {
        *out = (callable_t){ .kind = EXEC_FN1, .code = code };
        return 1;
    }
    if (!strcmp(prog, "gcd")) {
        *out = (callable_t){ .kind = EXEC_FN2, .code = code };
        return 1;
    }
    if (!strcmp(prog, "bubblesort")) {
        *out = (callable_t){ .kind = EXEC_FN2P, .code = code };
        return 1;
    }
    if (!strcmp(prog, "matrixmul")) {
        *out = (callable_t){ .kind = EXEC_FN4, .code = code };
        return 1;
    }
    if (!strcmp(prog, "smoke")) {
        *out = (callable_t){ .kind = EXEC_FN2I, .code = code };
        return 1;
    }
    return 0;
}

#if defined(__GNUC__)
__attribute__((noinline, optimize("O0")))
#endif
static int32_t invoke_callable(callable_t fn, int32_t *arr, int32_t *A, int32_t *B, int32_t *C)
{
    switch (fn.kind) {
        case EXEC_FN0: return ((exec_fn0_t)fn.code)();
        case EXEC_FN1: return ((exec_fn1_t)fn.code)(30);
        case EXEC_FN2: return ((exec_fn2_t)fn.code)(48, 18);
        case EXEC_FN2P:
            fill_desc(arr, 20);
            ((exec_fn2p_t)fn.code)(arr, 20);
            return 0;
        case EXEC_FN4:
            fill_desc(A, 100);
            fill_desc(B, 100);
            ((exec_fn4_t)fn.code)(A, B, C, 10);
            return 0;
        case EXEC_FN2I:
            fill_desc(arr, 20);
            return ((exec_fn2i_t)fn.code)(arr, 20);
    }
    return 0;
}

static int validate_workload(const char *prog, int32_t last,
                             const int32_t *arr, const int32_t *A,
                             const int32_t *B, const int32_t *C)
{
    if (!strcmp(prog, "hints")) {
        return last == 4950;
    }
    if (!strcmp(prog, "fibonacci")) {
        return last == 832040;
    }
    if (!strcmp(prog, "gcd")) {
        return last == 6;
    }
    if (!strcmp(prog, "smoke")) {
        return last == 1330;
    }
    if (!strcmp(prog, "bubblesort")) {
        for (int i = 0; i < 20; i++) {
            if (arr[i] != i + 1) {
                return 0;
            }
        }
        return 1;
    }
    if (!strcmp(prog, "matrixmul")) {
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

static int run_exec(const char *prog, prog_t *(*builder)(void), long iterations,
                    int hinted, exec_stats_t *stats)
{
    double ready_start = now_sec();

    micro_err_stk_size = 0;
    prog_t *p = builder();
    if (hinted && !strcmp(prog, "hints") && !micro_err_stk_size) {
        vm_hint_program(p);
    }
    if (micro_err_stk_size) {
        fprintf(stderr, "exec: IR build failed (%zu errors)\n", micro_err_stk_size);
        prog_free(p);
        return 0;
    }

    sct_vector_t outbuf;
    sct_vector_init(&outbuf, sizeof(u8));
    stage_times_t wst = {0};
    int rc = compile_prog(p, &outbuf, &wst);
    prog_free(p);
    if (rc != 0 || outbuf.size == 0) {
        fprintf(stderr, "exec: compile failed\n");
        sct_vector_deinit(&outbuf);
        return 0;
    }

    double load_start = now_sec();
    long page = sysconf(_SC_PAGESIZE);
    if (page <= 0) {
        page = 4096;
    }
    size_t map_sz = ((size_t)outbuf.size + (size_t)page - 1) & ~((size_t)page - 1);
    void *code = mmap(NULL, map_sz, PROT_READ | PROT_WRITE | PROT_EXEC,
                      MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (code == MAP_FAILED) {
        fprintf(stderr, "exec: mmap RWX failed\n");
        sct_vector_deinit(&outbuf);
        return 0;
    }
    memcpy(code, outbuf.data, outbuf.size);
    stats->code_size = outbuf.size;
    stats->load_us = (now_sec() - load_start) * 1e6;
    sct_vector_deinit(&outbuf);

    callable_t fn;
    if (!selectable_callable(prog, code, &fn)) {
        munmap(code, map_sz);
        return 0;
    }
    stats->ready_us = (now_sec() - ready_start) * 1e6;

    int32_t arr[20], A[100], B[100], C[100];
    int32_t last = 0;
    memset(arr, 0xa5, sizeof(arr));
    memset(A, 0xa5, sizeof(A));
    memset(B, 0xa5, sizeof(B));
    memset(C, 0xa5, sizeof(C));

    double first_start = now_sec();
    last = invoke_callable(fn, arr, A, B, C);
    stats->first_call_us = (now_sec() - first_start) * 1e6;

    for (int i = 0; i < 2; i++) {
        last = invoke_callable(fn, arr, A, B, C);
    }
    if (!validate_workload(prog, last, arr, A, B, C)) {
        fprintf(stderr, "exec: result check failed for %s\n", prog);
        munmap(code, map_sz);
        return 0;
    }

    double steady_start = now_sec();
    for (long i = 0; i < iterations; i++) {
        last = invoke_callable(fn, arr, A, B, C);
    }
    double steady_end = now_sec();
    if (!validate_workload(prog, last, arr, A, B, C)) {
        fprintf(stderr, "exec: post-run result check failed for %s\n", prog);
        munmap(code, map_sz);
        return 0;
    }

    volatile int32_t sink = last;
    (void)sink;
    stats->steady_us = (steady_end - steady_start) / (double)iterations * 1e6;
    munmap(code, map_sz);
    return 1;
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "Usage: bench_micro_lib <fibonacci|bubblesort|matrixmul|gcd|smoke|hints> [iterations] [plain|hinted]\n");
        return 1;
    }

    const char *prog = argv[1];
    long iterations = argc >= 3 ? atol(argv[2]) : 100;
    const char *mode = argc >= 4 ? argv[3] : "plain";
    if (iterations <= 0) iterations = 100;
    if (strcmp(mode, "plain") && strcmp(mode, "hinted")) {
        fprintf(stderr, "Unknown mode: %s\n", mode);
        return 1;
    }
    int hinted = !strcmp(mode, "hinted");

    prog_t *(*builder)(void) = NULL;
    if      (!strcmp(prog, "fibonacci"))  builder = build_fibonacci;
    else if (!strcmp(prog, "gcd"))        builder = build_gcd;
    else if (!strcmp(prog, "bubblesort")) builder = build_bubblesort;
    else if (!strcmp(prog, "matrixmul"))  builder = build_matrixmul;
    else if (!strcmp(prog, "smoke"))      builder = build_smoke;
    else if (!strcmp(prog, "hints"))      builder = build_hints;
    else { fprintf(stderr, "Unknown program: %s\n", prog); return 1; }

    micro_init();

    /* warm up */
    for (long i = 0; i < 3; i++) {
        micro_err_stk_size = 0;
        prog_t *p = builder();
        if (hinted && !strcmp(prog, "hints") && !micro_err_stk_size) {
            vm_hint_program(p);
        }
        sct_vector_t tmp; sct_vector_init(&tmp, sizeof(u8));
        stage_times_t wst = {0};
        compile_prog(p, &tmp, &wst);
        sct_vector_deinit(&tmp);
        prog_free(p);
    }

    /* benchmark */
    stage_times_t st = {0};
    int success = 0;
    size_t code_size = 0;

    for (long i = 0; i < iterations; i++) {
        /* irgen: build IR through micro_instr_gen_* */
        double t0 = now_sec();
        micro_err_stk_size = 0;
        prog_t *p = builder();
        st.irgen += (long)((now_sec() - t0) * 1e9);

        if (hinted && !strcmp(prog, "hints") && !micro_err_stk_size) {
            double tv = now_sec();
            vm_hint_program(p);
            st.vmopt += (long)((now_sec() - tv) * 1e9);
        }

        if (micro_err_stk_size) {
            fprintf(stderr, "IR build failed (%zu errors)\n", micro_err_stk_size);
            prog_free(p);
            continue;
        }

        sct_vector_t outbuf;
        sct_vector_init(&outbuf, sizeof(u8));

        if (compile_prog(p, &outbuf, &st) == 0) {
            success++;
            code_size = outbuf.size;
        }

        sct_vector_deinit(&outbuf);

        double tf = now_sec();
        prog_free(p);
        st.cleanup += (long)((now_sec() - tf) * 1e9);
    }

    if (success == 0) {
        fprintf(stderr, "All compilations failed\n");
        micro_deinit();
        return 1;
    }

    double n = (double)success;
    double total_us    = ((double)st.irgen + st.vmopt + st.codegen + st.asmopt + st.emit + st.cleanup) / n / 1000.0;
    double irgen_us    = (double)st.irgen   / n / 1000.0;
    double codegen_us  = (double)st.codegen / n / 1000.0;
    double asmopt_us   = (double)st.asmopt  / n / 1000.0;
    double emit_us     = (double)st.emit    / n / 1000.0;
    double cleanup_us  = (double)st.cleanup / n / 1000.0;

    double vmopt_us = (double)st.vmopt / n / 1000.0;
    exec_stats_t exec = {
        .ready_us = -1.0,
        .load_us = -1.0,
        .first_call_us = -1.0,
        .steady_us = -1.0,
        .code_size = 0,
    };
    int exec_ok = 0;
    if (sizeof(void *) == 4) {
        exec_ok = run_exec(prog, builder, iterations, hinted, &exec);
        if (!exec_ok) {
            micro_deinit();
            return 1;
        }
        code_size = exec.code_size;
    }

    micro_deinit();

    printf("schema=1 status=ok backend=micro target=x86-32 mode=%s requested=%ld success=%ld exec_ok=%d "
           "compile_us=%.3f total_us=%.3f irgen_us=%.3f codegen_us=%.3f asmopt_us=%.3f "
           "emit_us=%.3f cleanup_us=%.3f vmopt_us=%.3f ready_us=%.3f load_us=%.3f "
           "first_call_us=%.3f steady_us=%.3f code_size=%zu\n",
           hinted ? "hinted" : "plain", iterations, success, exec_ok,
           total_us, total_us, irgen_us, codegen_us, asmopt_us, emit_us, cleanup_us,
           vmopt_us, exec.ready_us, exec.load_us, exec.first_call_us, exec.steady_us, code_size);
    return 0;
}
