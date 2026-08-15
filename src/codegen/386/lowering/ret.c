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

    if (ext->used_regs[0]) {
        int free_space = get_last_free_space(codegen);

        for (size_t i = 0; i < ext->idents.keys.size; i++) {
            char *name = *(char**)sct_vector_get(&ext->idents.keys, i);
            micro_codegen386_ident_t *ident = sct_hashmap_get(&ext->idents, name);
            if (ident->type != MICRO_IDENT_VREG || ident->vreg.storage.reg.reg != MICRO_ASM386_REG32_EAX) {
                continue;
            }

            micro_codegen386_storage_t *storage = &ident->vreg.storage;
            if (free_space < 0) {
                storage->type = MICRO_STORAGE_STACK;
                storage->stack.ebp_offset = free_space;

                push_asm_instr(MICRO_ASM386_INSTR_MOV_S32R32, { .imm = micro_imm_le_gen(free_space) }, { .reg = 0 });
            } else {
                storage->type = MICRO_STORAGE_REG;
                storage->reg.reg = free_space;
                storage->reg.size = MICRO_SIZE_32;
            
                push_asm_instr(MICRO_ASM386_INSTR_MOV_R32R32, { .reg = free_space }, { .reg = 0 });
            }

            sct_hashmap_add(&ext->idents, name, ident);
        }
    }

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