#include <micro/codegen/386/codegen386.h>
#include "internal.h"

void micro_codegen386_emit(micro_codegen_t *codegen, sct_vector_t *instrs)
{
    codegen->instrs = instrs;
    codegen->pos = 0;
    _micro_codegen386_ext(codegen)->in_function = 0;
}

void micro_codegen386_init(micro_codegen_t *codegen)
{
    sct_vector_init(&codegen->outbuf, sizeof(u8));
    codegen->emit = micro_codegen386_emit;
    micro_codegen386_ext_t *ext = amalloc(sizeof(micro_codegen386_ext_t));
    *ext = (micro_codegen386_ext_t){};
    codegen->ext = ext;
}


void micro_codegen386_deinit(micro_codegen_t *codegen)
{
    sct_vector_deinit(&codegen->outbuf);
    
    free(codegen->ext);
}