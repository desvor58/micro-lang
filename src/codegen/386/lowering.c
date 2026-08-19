#include "internal.h"

int lowering(micro_codegen_t *codegen)
{
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
        }

        codegen->pos++;
    }
    return 0;
}
