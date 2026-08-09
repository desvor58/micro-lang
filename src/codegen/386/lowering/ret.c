#include "../internal.h"

int lowering_ret(micro_codegen_t *codegen, micro_instruction_t *instr)
{
    micro_codegen386_ext_t *ext = _micro_codegen386_ext(codegen);

    if (!ext->in_function) {
        micro_push_err((micro_error_t){
            .msg = "'ret' instruction can be only in function body",
            .line_ref = instr->start_tok->line_ref,
            .chpos_ref = instr->start_tok->chpos_ref
        });
        return 1;
    }

    micro_instruction_ret_t instr_ret = instr->ret;

    int expr_parse_offset = expr_parse(codegen, (micro_codegen386_storage_t){
        .type = MICRO_STORAGE_REG,
        .reg = {
            .reg = MICRO_ASM386_REG32_EAX,
            .size = MICRO_SIZE_32
        }
    }, instr_ret.val_expr);

    if (!expr_parse_offset) {
        micro_push_err((micro_error_t){
            .msg = "expression parse error",
            .line_ref = instr_ret.val_expr->line_ref,
            .chpos_ref = instr_ret.val_expr->chpos_ref
        });
        return 1;
    }
    return 0;
}