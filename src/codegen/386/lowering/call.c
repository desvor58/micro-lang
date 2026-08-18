#include "../internal.h"

int lowering_call(micro_codegen_t *codegen, micro_instruction_t *instr)
{
    micro_codegen386_ext_t *ext = _micro_codegen386_ext(codegen);

    if (!ext->in_function) {
        micro_push_err((micro_error_t){
            .msg = "'call' instruction can be only in function body",
            .line_ref = instr->start_tok->line_ref,
            .chpos_ref = instr->start_tok->chpos_ref
        });
        return 1;
    }

    micro_instruction_call_t instr_call = instr->call;

    micro_codegen386_ident_fun_t *fun = sct_hashmap_get(&ext->idents, instr_call.fun_name);
    
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
            .msg = "Too few arguments for calling",
            .line_ref = instr->start_tok->line_ref,
            .chpos_ref = instr->start_tok->chpos_ref
        });
        return 1;
    }
    if (instr_call.arg_exprs.size > fun->instr_info.args.size) {
        micro_push_err((micro_error_t){
            .msg = "Too much arguments for calling",
            .line_ref = instr->start_tok->line_ref,
            .chpos_ref = instr->start_tok->chpos_ref
        });
        return 1;
    }

    for (int i = instr_call.arg_exprs.size - 1; i > 0; i++) {
        micro_instruction_fun_arg_t *arg = sct_vector_get(&fun->instr_info.args, i);
        arg_dst.reg.size = micro_type_to_size[arg->type];

        micro_token_t **arg_start_tok = sct_vector_get(&instr_call.arg_exprs, i);
        int expr_size = expr_parse(codegen, arg_dst, *arg_start_tok);
        
        push_asm_instr(MICRO_ASM386_INSTR_PUSH_R32, { .reg = arg_dst.reg.reg }, {});
    }

    return 0;
}
