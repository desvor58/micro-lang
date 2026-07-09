#include "../internal.h"

int lowering_fun(micro_codegen_t *codegen, micro_instruction_t *instr)
{
    micro_instruction_fun_t instr_fun = instr->fun;

    micro_asm386_prelude();

    sct_vector_t *instrs_save = codegen->instrs;
    size_t pos_save = codegen->pos;

    codegen->emit(codegen, &instr_fun.body);

    codegen->instrs = instrs_save;
    codegen->pos = pos_save;

    micro_asm386_epilogue();

    micro_asm386_put_instructions(&codegen->outbuf);

    return 0;
}
