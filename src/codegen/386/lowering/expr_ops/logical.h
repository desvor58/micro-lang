#ifndef MICRO_CODEGEN386_EXPR_OPS_LOGICAL_H
#define MICRO_CODEGEN386_EXPR_OPS_LOGICAL_H

#include "ampersand.h"
#include "pipe.h"

static expr_info_t op_logical_handler(micro_codegen_t *codegen, micro_codegen386_storage_t dst, micro_expr_tok_t *start, const op_tbls_t *op_tbls)
{
    micro_codegen386_ext_t *ext = _micro_codegen386_ext(codegen);

    expr_info_t first_info = expr_parse(codegen, dst, start + 1);
    if (!first_info.size) return (expr_info_t){ 0, MICRO_TYPE_NULL };

    op_bool_to_dst(codegen, dst, micro_type_to_size[first_info.type]);

    int free_space = get_last_free_space(codegen);
    micro_codegen386_storage_t tmp;
    if (free_space < 0) {
        tmp.type = MICRO_STORAGE_STACK;
        tmp.stack.ebp_offset = free_space;
        ext->max_stack_offset -= 4;
        ext->ebp_top_offset -= 4;
    } else {
        tmp.type = MICRO_STORAGE_REG;
        tmp.reg.reg = free_space;
        tmp.reg.size = MICRO_SIZE_32;
    }

    expr_info_t second_info = expr_parse(codegen, tmp, start + 1 + first_info.size);
    if (!second_info.size) return (expr_info_t){ 0, MICRO_TYPE_NULL };

    op_bool_to_dst(codegen, tmp, micro_type_to_size[second_info.type]);
    op_tmp_to_dst(codegen, op_tbls, dst, tmp);

    if (tmp.type == MICRO_STORAGE_REG) {
        ext->used_regs[tmp.reg.reg] = 0;
    }

    return (expr_info_t){ 1 + first_info.size + second_info.size, first_info.type };
}

expr_info_t op_andand_handler(micro_codegen_t *codegen, micro_codegen386_storage_t dst, micro_expr_tok_t *start)
{
    if (unlikely(start->type != MICRO_EXPR_TOK_ANDAND)) {
        return (expr_info_t){ 0, MICRO_TYPE_NULL };
    }
    return op_logical_handler(codegen, dst, start, &ampersand_op_tbls);
}

expr_info_t op_oror_handler(micro_codegen_t *codegen, micro_codegen386_storage_t dst, micro_expr_tok_t *start)
{
    if (unlikely(start->type != MICRO_EXPR_TOK_OROR)) {
        return (expr_info_t){ 0, MICRO_TYPE_NULL };
    }
    return op_logical_handler(codegen, dst, start, &pipe_op_tbls);
}

#endif
