#include "../internal.h"

int lowering_fun(micro_codegen_t *codegen, micro_instruction_t *instr)
{
    micro_codegen386_ext_t *ext = _micro_codegen386_ext(codegen);

    micro_instruction_fun_t instr_fun = instr->fun;

    micro_asm386_prelude();
    micro_asm386_put_instructions(&codegen->outbuf);

    ext->ebp_top_offset = -4;
    ext->in_function = 1;

    micro_codegen386_ident_fun_t fun = {
        .instr_info = instr_fun,
        .address = codegen->outbuf.size
    };

    sct_hashmap_add(&ext->idents, instr_fun.name, &(micro_codegen386_ident_t){
        .type = MICRO_IDENT_FUN,
        .fun  = fun,
    });

    sct_vector_t *instrs_save = codegen->instrs;
    size_t pos_save = codegen->pos;
        codegen->emit(codegen, &instr_fun.body);
    codegen->instrs = instrs_save;
    codegen->pos = pos_save;

    micro_asm386_epilogue();
    micro_asm386_put_instructions(&codegen->outbuf);

    return 0;
}
