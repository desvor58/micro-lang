#include <micro/iropter.h>

#define INL_SEARCHED_FUNS_NUM 64

static size_t expr_arity(micro_expr_tok_type_t type)
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

static size_t expr_size(const micro_expr_tok_t *expr)
{
    size_t arity = expr_arity(expr->type);
    if (!arity) {
        return 1;
    }

    size_t size = 1;
    const micro_expr_tok_t *arg = expr + 1;
    for (size_t i = 0; i < arity; i++) {
        size_t arg_size = expr_size(arg);
        if (!arg_size) {
            return 0;
        }
        size += arg_size;
        arg += arg_size;
    }
    return size;
}

typedef struct {
    const char *from;
    char        to[MICRO_MAX_SYMBOL_SIZE];
} inl_name_t;

typedef struct {
    micro_iropter_t *iropter;
    char             prefix[16];
    size_t           short_names;
    sct_vector_t     names;
} inl_ctx_t;

static micro_instruction_hints_t inl_hints(void)
{
    return (micro_instruction_hints_t){
        .lifetime = -1,
        .forced_stack = 0,
        .lazy_init = 0,
    };
}

static void inl_name(inl_ctx_t *ctx, const char *name, char *dst)
{
    for (size_t i = 0; i < ctx->names.size; i++) {
        inl_name_t *pair = sct_vector_get(&ctx->names, i);
        if (!strcmp(pair->from, name)) {
            strcpy(dst, pair->to);
            return;
        }
    }

    if (strlen(ctx->prefix) + strlen(name) < MICRO_MAX_SYMBOL_SIZE) {
        snprintf(dst, MICRO_MAX_SYMBOL_SIZE, "%s%s", ctx->prefix, name);
    } else {
        snprintf(dst, MICRO_MAX_SYMBOL_SIZE, "t%zu", ctx->short_names++);
    }

    inl_name_t pair = { .from = name };
    strcpy(pair.to, dst);
    sct_vector_push(&ctx->names, &pair);
}

static size_t inl_expr_copy(inl_ctx_t *ctx, const micro_expr_tok_t *src, micro_expr_tok_t *dst)
{
    dst[0] = src[0];

    if (src[0].type == MICRO_EXPR_TOK_IDENT) {
        inl_name(ctx, src[0].val, dst[0].val);
        return 1;
    }

    size_t arity = expr_arity(src[0].type);
    if (!arity) {
        return 1;
    }

    size_t written = 1;
    const micro_expr_tok_t *arg = src + 1;
    for (size_t i = 0; i < arity; i++) {
        written += inl_expr_copy(ctx, arg, dst + written);
        arg += expr_size(arg);
    }
    return written;
}

static micro_expr_tok_t *inl_expr_clone(inl_ctx_t *ctx, const micro_expr_tok_t *src)
{
    if (!src) {
        return NULL;
    }

    size_t size = expr_size(src);
    if (!size) {
        return NULL;
    }

    micro_expr_tok_t *dst = sct_arena_alloc(&ctx->iropter->arena, size * sizeof(micro_expr_tok_t));
    inl_expr_copy(ctx, src, dst);
    return dst;
}

static sct_vector_t inl_arg_exprs(inl_ctx_t *ctx, sct_vector_t *src)
{
    sct_vector_t dst = {0};
    dst._item_size = sizeof(micro_expr_tok_t *);
    dst.size = src->size;
    dst.cap = src->size * dst._item_size;
    if (!dst.cap) {
        return dst;
    }

    dst.data = sct_arena_alloc(&ctx->iropter->arena, dst.cap);
    for (size_t i = 0; i < src->size; i++) {
        micro_expr_tok_t *arg = *(micro_expr_tok_t **)sct_vector_get(src, i);
        micro_expr_tok_t *copy = inl_expr_clone(ctx, arg);
        sct_vector_set(&dst, i, &copy);
    }
    return dst;
}

static int inl_body_supported(sct_vector_t *body)
{
    for (size_t i = 0; i < body->size; i++) {
        micro_instruction_t *instr = sct_vector_get(body, i);
        if (instr->type == MICRO_INSTR_FUN || instr->type == MICRO_INSTR_TRAMP) {
            return 0;
        }
    }
    return 1;
}

static int inl_reaches(micro_iropter_t *iropter, const char *from, const char *to)
{
    char queue[INL_SEARCHED_FUNS_NUM][MICRO_MAX_SYMBOL_SIZE];
    size_t head = 0;
    size_t tail = 0;

    strncpy(queue[tail++], from, MICRO_MAX_SYMBOL_SIZE - 1);
    queue[tail - 1][MICRO_MAX_SYMBOL_SIZE - 1] = '\0';

    while (head < tail) {
        char name[MICRO_MAX_SYMBOL_SIZE];
        strncpy(name, queue[head++], MICRO_MAX_SYMBOL_SIZE - 1);
        name[MICRO_MAX_SYMBOL_SIZE - 1] = '\0';

        micro_iropter_fun_info_t *fun_info = sct_arena_hashmap_get(&iropter->fun_infos, name);
        if (!fun_info) {
            continue;
        }

        for (size_t i = 0; i < fun_info->fun->body.size; i++) {
            micro_instruction_t *instr = sct_vector_get(&fun_info->fun->body, i);
            if (instr->type != MICRO_INSTR_CALL) {
                continue;
            }

            const char *callee = instr->call.fun_name;
            if (!strcmp(callee, to)) {
                return 1;
            }

            int seen = 0;
            for (size_t j = 0; j < tail; j++) {
                if (!strcmp(queue[j], callee)) {
                    seen = 1;
                    break;
                }
            }
            if (seen || tail >= INL_SEARCHED_FUNS_NUM) {
                continue;
            }

            strncpy(queue[tail], callee, MICRO_MAX_SYMBOL_SIZE - 1);
            queue[tail][MICRO_MAX_SYMBOL_SIZE - 1] = '\0';
            tail++;
        }
    }
    return 0;
}

u32 calc_call_price(micro_iropter_t *iropter, micro_instruction_call_t call_instr)
{
    i64 price = 20;

    micro_iropter_fun_info_t *fun_info = sct_arena_hashmap_get(&iropter->fun_infos, call_instr.fun_name);

    if (call_instr.arg_exprs.size < 3) {
        price += 10;
    }

    price -= (i64)(fun_info ? fun_info->fun->body.size / 2 : 0);
    if (price < 0) return 0;
    else if (price >= MICRO_IROPTER_CALL_PRICES_BUCKETS_NUM) return 63;

    return (u32)price;
}

static int inl_call_inlinable(micro_iropter_t *iropter, micro_iropter_call_info_t *call_info)
{
    if (!call_info->instrs || call_info->call_pos >= call_info->instrs->size) {
        return 0;
    }

    micro_instruction_t *instr = sct_vector_get(call_info->instrs, call_info->call_pos);
    if (!instr || instr->type != MICRO_INSTR_CALL) {
        return 0;
    }

    micro_iropter_fun_info_t *fun_info = sct_arena_hashmap_get(&iropter->fun_infos, instr->call.fun_name);
    if (!fun_info || !inl_body_supported(&fun_info->fun->body)) {
        return 0;
    }

    if (instr->call.arg_exprs.size != fun_info->fun->args.size) {
        return 0;
    }

    return !inl_reaches(iropter, instr->call.fun_name, instr->call.fun_name);
}

static void inl_add_call(micro_iropter_t *iropter, sct_vector_t *instrs, size_t call_pos, micro_instruction_t *instr)
{
    u32 price = calc_call_price(iropter, instr->call);

    sct_arena_vector_t *bucket = &iropter->call_prices[price];

    if (!bucket->data) {
        sct_arena_vector_init(bucket, &iropter->arena, sizeof(micro_iropter_call_info_t));
    }
    sct_arena_vector_push(bucket, &(micro_iropter_call_info_t){
        .instrs = instrs,
        .call_pos = call_pos,
        .price = price,
    });
}
static void inl_shift_positions(micro_iropter_t *iropter, sct_vector_t *instrs, size_t from, ptrdiff_t delta)
{
    for (u32 b = 0; b < MICRO_IROPTER_CALL_PRICES_BUCKETS_NUM; b++) {
        sct_arena_vector_t *bucket = &iropter->call_prices[b];
        for (size_t i = 0; i < bucket->size; i++) {
            micro_iropter_call_info_t *call_info = sct_arena_vector_get(bucket, i);
            if (call_info->instrs == instrs && call_info->call_pos > from) {
                call_info->call_pos += delta;
            }
        }
    }
}

static void inl_collect_range(micro_iropter_t *iropter, sct_vector_t *instrs, size_t from, size_t count)
{
    for (size_t i = 0; i < count; i++) {
        size_t pos = from + i;
        if (pos >= instrs->size) {
            break;
        }
        micro_instruction_t *instr = sct_vector_get(instrs, pos);
        if (instr->type == MICRO_INSTR_CALL) {
            inl_add_call(iropter, instrs, pos, instr);
        }
    }
}

static size_t inl_inline_call(micro_iropter_t *iropter, micro_iropter_call_info_t *call_info)
{
    sct_vector_t *instrs = call_info->instrs;
    size_t call_pos = call_info->call_pos;

    micro_instruction_t *call_instr = sct_vector_get(instrs, call_pos);
    if (call_instr->type != MICRO_INSTR_CALL) {
        return 0;
    }

    micro_instruction_call_t call = call_instr->call;
    micro_iropter_fun_info_t *fun_info = sct_arena_hashmap_get(&iropter->fun_infos, call.fun_name);
    if (!fun_info) {
        return 0;
    }

    inl_ctx_t ctx = { .iropter = iropter, .short_names = 0 };
    sct_vector_init(&ctx.names, sizeof(inl_name_t));
    snprintf(ctx.prefix, sizeof(ctx.prefix), "i%u.", iropter->inlined_calls);

    sct_vector_t chunk;
    sct_vector_init(&chunk, sizeof(micro_instruction_t));

    for (size_t i = 0; i < call.arg_exprs.size; i++) {
        micro_instruction_fun_arg_t *arg = sct_vector_get(&fun_info->fun->args, i);
        micro_expr_tok_t *arg_expr = *(micro_expr_tok_t **)sct_vector_get(&call.arg_exprs, i);

        char tmp[MICRO_MAX_SYMBOL_SIZE];
        inl_name(&ctx, arg->name, tmp);

        micro_instruction_t set_instr = {
            .type = MICRO_INSTR_SET,
            .hints = inl_hints(),
        };
        set_instr.set.type = arg->type;
        strcpy(set_instr.set.reg_name, tmp);
        set_instr.set.val_expr = arg_expr;
        sct_vector_push(&chunk, &set_instr);
    }

    for (size_t i = 0; i < fun_info->fun->body.size; i++) {
        micro_instruction_t *src = sct_vector_get(&fun_info->fun->body, i);
        micro_instruction_t copy = *src;
        copy.hints = inl_hints();

        switch (src->type) {
            case MICRO_INSTR_SET:
                inl_name(&ctx, src->set.reg_name, copy.set.reg_name);
                copy.set.val_expr = inl_expr_clone(&ctx, src->set.val_expr);
                break;

            case MICRO_INSTR_DRSET:
                inl_name(&ctx, src->drset.reg_name, copy.drset.reg_name);
                copy.drset.val_expr = inl_expr_clone(&ctx, src->drset.val_expr);
                break;

            case MICRO_INSTR_CALL:
                inl_name(&ctx, src->call.ret_reg_name, copy.call.ret_reg_name);
                copy.call.arg_exprs = inl_arg_exprs(&ctx, &src->call.arg_exprs);
                break;

            case MICRO_INSTR_RET:
                copy.ret.val_expr = inl_expr_clone(&ctx, src->ret.val_expr);
                break;

            case MICRO_INSTR_IF:
                inl_name(&ctx, src->if_goto.lbl_name, copy.if_goto.lbl_name);
                copy.if_goto.cond_expr = inl_expr_clone(&ctx, src->if_goto.cond_expr);
                break;

            case MICRO_INSTR_LBL:
                inl_name(&ctx, src->lbl.name, copy.lbl.name);
                break;

            case MICRO_INSTR_GOTO:
                inl_name(&ctx, src->goto_lbl.lbl, copy.goto_lbl.lbl);
                break;

            default:
                sct_vector_deinit(&chunk);
                sct_vector_deinit(&ctx.names);
                return 0;
        }
        sct_vector_push(&chunk, &copy);
    }

    char end_lbl[MICRO_MAX_SYMBOL_SIZE];
    snprintf(end_lbl, sizeof(end_lbl), "i%u", iropter->inlined_calls);

    micro_instruction_t leave = {
        .type = MICRO_INSTR_GOTO,
        .hints = inl_hints(),
    };
    strcpy(leave.goto_lbl.lbl, end_lbl);

    int keep_result = call.ret_reg_name[0] && call.ret_reg_name[0] != '_';

    for (size_t i = 0; i < chunk.size; i++) {
        micro_instruction_t *instr = sct_vector_get(&chunk, i);
        if (instr->type != MICRO_INSTR_RET) {
            continue;
        }

        micro_expr_tok_t *val_expr = instr->ret.val_expr;

        if (keep_result && val_expr) {
            micro_instruction_t set_instr = {
                .type = MICRO_INSTR_SET,
                .hints = inl_hints(),
            };
            set_instr.set.type = fun_info->fun->ret_type;
            strcpy(set_instr.set.reg_name, call.ret_reg_name);
            set_instr.set.val_expr = val_expr;
            sct_vector_set(&chunk, i, &set_instr);
            sct_vector_insert(&chunk, i + 1, &leave);
            i++;
        } else {
            sct_vector_set(&chunk, i, &leave);
        }
    }

    micro_instruction_t end_instr = {
        .type = MICRO_INSTR_LBL,
        .hints = inl_hints(),
    };
    strcpy(end_instr.lbl.name, end_lbl);
    sct_vector_push(&chunk, &end_instr);

    sct_vector_erase(instrs, call_pos);
    for (size_t i = 0; i < chunk.size; i++) {
        sct_vector_insert(instrs, call_pos + i, sct_vector_get(&chunk, i));
    }

    size_t chunk_size = chunk.size;

    sct_vector_deinit(&chunk);
    sct_vector_deinit(&ctx.names);

    iropter->inlined_calls++;
    iropter->inlined_instrs += chunk_size;

    return chunk_size;
}

static void inl_collect(micro_iropter_t *iropter)
{
    micro_instruction_t *instr = sct_vector_get(iropter->instrs, iropter->pos);

    if (instr->type == MICRO_INSTR_FUN) {
        sct_arena_hashmap_add(&iropter->fun_infos, instr->fun.name,
                              &(micro_iropter_fun_info_t){ .fun = &instr->fun });
    }

    if (instr->type == MICRO_INSTR_CALL) {
        if (!sct_arena_hashmap_contains(&iropter->fun_infos, instr->call.fun_name)) {
            micro_push_err((micro_error_t){
                .err = MICRO_ERROR_UNDEFINED_FUN,
                .instr = MICRO_INSTR_CALL,
            });
        } else {
            inl_add_call(iropter, iropter->instrs, iropter->pos, instr);
        }
    }
}

static void inl_inline(micro_iropter_t *iropter)
{
    for (;;) {
        micro_iropter_call_info_t *target = NULL;
        u32 target_bucket = 0;
        size_t target_index = 0;

        for (u32 b = 0; b < MICRO_IROPTER_CALL_PRICES_BUCKETS_NUM && !target; b++) {
            sct_arena_vector_t *bucket = &iropter->call_prices[b];
            for (size_t i = 0; i < bucket->size; i++) {
                micro_iropter_call_info_t *call_info = sct_arena_vector_get(bucket, i);
                if (!inl_call_inlinable(iropter, call_info)) {
                    continue;
                }
                target = call_info;
                target_bucket = b;
                target_index = i;
                break;
            }
        }

        if (!target) {
            break;
        }

        micro_iropter_fun_info_t *fun_info = sct_arena_hashmap_get(&iropter->fun_infos,
            ((micro_instruction_t *)sct_vector_get(target->instrs, target->call_pos))->call.fun_name);
        size_t estimate = fun_info->fun->body.size + fun_info->fun->args.size + 2;
        if (iropter->inlined_instrs + estimate > iropter->budget) {
            break;
        }

        /* erasing moves the rest of the bucket over the entry, keep a copy */
        micro_iropter_call_info_t entry = *target;
        sct_arena_vector_erase(&iropter->call_prices[target_bucket], target_index);

        size_t chunk_size = inl_inline_call(iropter, &entry);
        if (!chunk_size) {
            continue;
        }

        inl_shift_positions(iropter, entry.instrs, entry.call_pos, (ptrdiff_t)chunk_size - 1);
        inl_collect_range(iropter, entry.instrs, entry.call_pos, chunk_size);
    }
}

void micro_iropter_inlining_pass(micro_iropter_t *iropter)
{
    iropter->pos = iropter->instr_save_pos;

    for (;;) {
        int descended = 0;

        for (; iropter->pos < iropter->instrs->size; iropter->pos++) {
            inl_collect(iropter);

            micro_instruction_t *instr = sct_vector_get(iropter->instrs, iropter->pos);
            if (instr->type != MICRO_INSTR_FUN) {
                continue;
            }

            iropter->instr_save_pos = iropter->pos + 1;
            iropter->instr_save = iropter->instrs;
            iropter->instrs = &instr->fun.body;
            iropter->pos = 0;
            descended = 1;
            break;
        }

        if (descended) {
            continue;
        }

        if (!iropter->instr_save) {
            break;
        }
        iropter->instrs = iropter->instr_save;
        iropter->instr_save = 0;
        iropter->pos = iropter->instr_save_pos;
        iropter->instr_save_pos = 0;
    }

    inl_inline(iropter);
}