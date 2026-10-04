#include "../internal.h"

int lowering_tramp(micro_codegen_t *codegen, micro_instruction_t *instr)
{
    micro_codegen386_ext_t *ext = _micro_codegen386_ext(codegen);

    if (!codegen->flags.no_err_outside_fun && ext->in_function) {
        micro_push_err((micro_error_t){
            .err = MICRO_ERROR_TRAMP_INSIDE_FUNCTION,
            .instr = MICRO_INSTR_TRAMP,
        });
        return 1;
    }

    micro_instruction_tramp_t instr_tramp = instr->tramp;

    if (sct_hashmap_get(&ext->idents, instr_tramp.name)) {
        micro_push_err((micro_error_t){
            .err = MICRO_ERROR_TRAMP_REDEFINED,
            .instr = MICRO_INSTR_TRAMP,
        });
        return 1;
    }

    micro_tramp_t *handler = codegen->tramps ? sct_hashmap_get(codegen->tramps, instr_tramp.name) : NULL;
    if (unlikely(!handler || !*handler)) {
        micro_push_err((micro_error_t){
            .err = MICRO_ERROR_UNDEFINED_TRAMP,
            .instr = MICRO_INSTR_TRAMP,
        });
        return 1;
    }

    sct_hashmap_add(&ext->idents, instr_tramp.name, &(micro_codegen386_ident_t){
        .type = MICRO_IDENT_TRAMP,
        .lifetime = -1,
        .tramp = {
            .instr_info = instr_tramp,
            .handler = *handler,
        },
    });

    return 0;
}