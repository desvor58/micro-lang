#include "../internal.h"

int lowering_set(micro_codegen_t *codegen, micro_instruction_t *instr)
{
    if (!_micro_codegen386_ext(codegen)->in_function) {
        micro_push_err((micro_error_t){
            .msg = "'set' instruction can be only in function body",
            .line_ref = instr->start_tok->line_ref,
            .chpos_ref = instr->start_tok->chpos_ref
        });
        return 1;
    }

    micro_instruction_set_t instr_set = instr->set;
}
