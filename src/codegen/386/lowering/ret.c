#include "../internal.h"

int lowering_ret(micro_codegen_t *codegen, micro_instruction_t *instr)
{
    micro_codegen386_ext_t *ext = _micro_codegen386_ext(codegen);

    if (!codegen->flags.no_err_outside_fun && !ext->in_function) {
        micro_push_err((micro_error_t){
            .err = MICRO_ERROR_RET_OUTSIDE_FUNCTION,
            .instr = MICRO_INSTR_RET
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

                push_asm_instr(MICRO_ASM386_INSTR_MOV_S32R32, operand_imm(MICRO_SIZE_32, micro_imm_le_gen(free_space)), operand_reg(MICRO_SIZE_32, 0));
            } else {
                storage->type = MICRO_STORAGE_REG;
                storage->reg.reg = free_space;
                storage->reg.size = MICRO_SIZE_32;
            
                push_asm_instr(MICRO_ASM386_INSTR_MOV_R32R32, operand_reg(MICRO_SIZE_32, free_space), operand_reg(MICRO_SIZE_32, 0));
            }

            sct_hashmap_add(&ext->idents, name, ident);
        }
    }

    if (instr_ret.val_expr) {
        expr_info_t expr_info = expr_parse(codegen, (micro_codegen386_storage_t){
            .type = MICRO_STORAGE_REG,
            .reg = {
                .reg = MICRO_ASM386_REG32_EAX,
                .size = MICRO_SIZE_32
            }
        }, instr_ret.val_expr);

        if (!expr_info.size) {
            // micro_push_err((micro_error_t){
            //     .err = MICRO_ERROR_EXPR_PARSE,
            //     .instr = MICRO_INSTR_RET
            // });
            return 1;
        }
    }

    char *end_lbl_name = sct_arena_alloc(ext->arena, sizeof(char) * strlen(ext->curent_function_name) + 1 + 4);
    strcpy(end_lbl_name, ext->curent_function_name);
    strcat(end_lbl_name, ".end");
    push_asm_instr(MICRO_ASM386_INSTR_JMP_L32, operand_lbl(MICRO_SIZE_32, end_lbl_name), operand_none());
    
    return 0;
}