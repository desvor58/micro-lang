#include "internal.h"

typedef int (*lowering_handler_t)(micro_codegen_t*, micro_instruction_t*);

int lowering(micro_codegen_t *codegen)
{
    micro_codegen386_ext_t *ext = _micro_codegen386_ext(codegen);
    int res = 0;

    static const lowering_handler_t handlers[] = {
        [MICRO_INSTR_SET]   = lowering_set,
        [MICRO_INSTR_DRSET] = lowering_drset,
        [MICRO_INSTR_FUN]   = lowering_fun,
        [MICRO_INSTR_TRAMP] = lowering_tramp,
        [MICRO_INSTR_RET]   = lowering_ret,
        [MICRO_INSTR_CALL]  = lowering_call,
        [MICRO_INSTR_LBL]   = lowering_lbl,
        [MICRO_INSTR_GOTO]  = lowering_goto,
        [MICRO_INSTR_IF]    = lowering_if,
    };

    while (codegen->pos < codegen->instrs->size) {
        micro_instruction_t *instr = sct_vector_get(codegen->instrs, codegen->pos);

        if (unlikely(instr->type >= sizeof(handlers) / sizeof(handlers[0]) || !handlers[instr->type])) {
            micro_push_err((micro_error_t){
                .err = MICRO_ERROR_UNEXPECTED_TOKEN,
                .instr = instr->type,
            });
        } else {
            res |= handlers[instr->type](codegen, instr);
        }

        ext->dead_idents.size = 0;
        sct_hashmap_iter_t ident_it;
        sct_hashmap_iter_init(&ident_it, &ext->idents);
        while (sct_hashmap_iter_next(&ident_it)) {
            micro_codegen386_ident_t *ident = sct_hashmap_iter_value(&ident_it);
            if (ident->lifetime >= 0 && (size_t)ident->lifetime <= codegen->pos) {
                if (ident->type == MICRO_IDENT_VREG && ident->vreg.storage.type == MICRO_STORAGE_REG) {
                    ext->used_regs[ident->vreg.storage.reg.reg] = 0;
                }
                const char *key = sct_hashmap_iter_key(&ident_it);
                sct_vector_push(&ext->dead_idents, &key);
            }
        }

        for (size_t i = 0; i < ext->dead_idents.size; i++) {
            sct_hashmap_remove(&ext->idents, *(const char**)sct_vector_get(&ext->dead_idents, i));
        }

        codegen->pos++;
    }
    return res;
}
