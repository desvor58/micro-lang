#include "internal.h"

int lowering(micro_codegen_t *codegen)
{
    micro_codegen386_ext_t *ext = _micro_codegen386_ext(codegen);
    int res = 0;

    while (codegen->pos < codegen->instrs->size) {
        micro_instruction_t *instr = sct_vector_get(codegen->instrs, codegen->pos);

        switch (instr->type) {
            case MICRO_INSTR_FUN:
                res |= lowering_fun(codegen, instr);
                break;

            case MICRO_INSTR_RET:
                res |= lowering_ret(codegen, instr);
                break;

            case MICRO_INSTR_SET:
                res |= lowering_set(codegen, instr);
                break;

            case MICRO_INSTR_CALL:
                res |= lowering_call(codegen, instr);
                break;

            case MICRO_INSTR_LBL:
                res |= lowering_lbl(codegen, instr);
                break;

            case MICRO_INSTR_GOTO:
                res |= lowering_goto(codegen, instr);
                break;

            case MICRO_INSTR_IF:
                res |= lowering_if(codegen, instr);
                break;

            case MICRO_INSTR_DRSET:
                res |= lowering_drset(codegen, instr);
                break;
        }

        sct_hashmap_iter_t ident_it;
        sct_hashmap_iter_init(&ident_it, &ext->idents);
        while (sct_hashmap_iter_next(&ident_it)) {
            micro_codegen386_ident_t *ident = sct_hashmap_iter_value(&ident_it);
            if (ident->lifetime == 0) {
                sct_hashmap_remove(&ext->idents, sct_hashmap_iter_key(&ident_it));
                if (ident->type == MICRO_IDENT_VREG && ident->vreg.storage.type == MICRO_STORAGE_REG) {
                    ext->used_regs[ident->vreg.storage.reg.reg] = 0;
                }
            } else
            if (ident->lifetime > 0) {
                ident->lifetime--;
            }
        }

        codegen->pos++;
    }
    return res;
}
