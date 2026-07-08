#include "internal.h"

int lowering(micro_codegen_t *codegen)
{
    for (;;) {
        micro_instruction_t *instr = sct_vector_get(codegen->instrs, codegen->pos);

        switch (instr->type) {
            case MICRO_INSTR_SET:
                lowering_set(codegen, instr);
                break;
        }
    }
}
