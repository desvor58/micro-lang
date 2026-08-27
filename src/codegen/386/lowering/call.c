#include "../internal.h"

int lowering_call(micro_codegen_t *codegen, micro_instruction_t *instr)
{
    micro_codegen386_ext_t *ext = _micro_codegen386_ext(codegen);

    if (!ext->in_function) {
        micro_push_err((micro_error_t){
            .err = MICRO_ERROR_CALL_OUTSIDE_FUNCTION,
            .instr = MICRO_INSTR_CALL
        });
        return 1;
    }

    micro_instruction_call_t instr_call = instr->call;

    micro_codegen386_ident_t *ident = sct_hashmap_get(&ext->idents, instr_call.fun_name);
    if (!ident || ident->type != MICRO_IDENT_FUN) {
        micro_push_err((micro_error_t){
            .err = MICRO_ERROR_UNDEFINED_FUN,
            .instr = MICRO_INSTR_CALL
        });
        return 1;
    }
    micro_codegen386_ident_fun_t *fun = &ident->fun;

    size_t stack_cleanup_offset = 0;

    for (size_t i = 0; i < 3; i++) {
        if (ext->used_regs[i]) {
            push_asm_instr(MICRO_ASM386_INSTR_PUSH_R32, { .reg = i }, {});
            stack_cleanup_offset += 4;
        }
    }
    
    int free_space = get_last_free_space(codegen);
    micro_codegen386_storage_t arg_dst = {
        .type = MICRO_STORAGE_REG
    };
    if (free_space < 0) {
        push_asm_instr(MICRO_ASM386_INSTR_MOV_S32R32, { .imm = micro_imm_le_gen(ext->ebp_top_offset) }, { .reg = MICRO_ASM386_REG32_EAX });
        ext->max_stack_offset -= 4;

        arg_dst.reg.reg = 0;
    } else {
        arg_dst.reg.reg = free_space;
    }

    if (instr_call.arg_exprs.size < fun->instr_info.args.size) {
        micro_push_err((micro_error_t){
            .err = MICRO_ERROR_TOO_FEW_ARGS,
            .instr = MICRO_INSTR_CALL
        });
        return 1;
    }
    if (instr_call.arg_exprs.size > fun->instr_info.args.size) {
        micro_push_err((micro_error_t){
            .err = MICRO_ERROR_TOO_MANY_ARGS,
            .instr = MICRO_INSTR_CALL
        });
        return 1;
    }

    for (int i = instr_call.arg_exprs.size - 1; i >= 0; i--) {
        micro_instruction_fun_arg_t *arg = sct_vector_get(&fun->instr_info.args, i);
        arg_dst.reg.size = micro_type_to_size[arg->type];

        micro_expr_tok_t **arg_start_tok = sct_vector_get(&instr_call.arg_exprs, i);
        int expr_size = expr_parse(codegen, arg_dst, *arg_start_tok);
        
        push_asm_instr(MICRO_ASM386_INSTR_PUSH_R32, { .reg = arg_dst.reg.reg }, {});
        stack_cleanup_offset += 4;
    }

    push_asm_instr(MICRO_ASM386_INSTR_CALL_L32, { .lbl_name = fun->instr_info.name }, {});

    push_asm_instr(MICRO_ASM386_INSTR_ADD_R32I32, { .reg = MICRO_ASM386_REG32_ESP }, { .imm = micro_imm_le_gen(stack_cleanup_offset) });

    micro_codegen386_ident_t *res_ident = sct_hashmap_get(&ext->idents, instr_call.ret_reg_name);
    if (!res_ident) {
        micro_push_err((micro_error_t){
            .err = MICRO_ERROR_UNDEFINED_IDENT,
            .instr = MICRO_INSTR_CALL
        });
        return 1;
    }
    if (res_ident->type != MICRO_IDENT_VREG) {
        micro_push_err((micro_error_t){
            .err = MICRO_ERROR_EXPECTED_VREG_RESULT,
            .instr = MICRO_INSTR_CALL
        });
        return 1;
    }
    if (res_ident->vreg.type != fun->instr_info.ret_type) {
        micro_push_err((micro_error_t){
            .err = MICRO_ERROR_RESULT_TYPE_MISMATCH,
            .instr = MICRO_INSTR_CALL
        });
        return 1;
    }

    switch (res_ident->vreg.storage.type) {
        case MICRO_STORAGE_DATASEC: return 1;
        case MICRO_STORAGE_STACK:
            push_asm_instr(MICRO_ASM386_INSTR_MOV_S32R32, { .imm = micro_imm_le_gen(res_ident->vreg.storage.stack.ebp_offset) }, { .reg = MICRO_ASM386_REG32_EAX });
            break;

        case MICRO_STORAGE_REG:
            push_asm_instr(MICRO_ASM386_INSTR_MOV_R32R32, { .reg = res_ident->vreg.storage.reg.reg }, { .reg = MICRO_ASM386_REG32_EAX });
            break;
    }

    return 0;
}
