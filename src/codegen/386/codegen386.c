#include <micro/codegen/386/codegen386.h>
#include "internal.h"

void micro_codegen386_emit(micro_codegen_t *codegen, sct_vector_t *instrs)
{
    codegen->instrs = instrs;
    codegen->pos = 0;

    lowering(codegen);
}

void micro_codegen386_init(micro_codegen_t *codegen)
{
    sct_vector_init(&codegen->outbuf, sizeof(u8));
    micro_asm386_init();
    codegen->emit = micro_codegen386_emit;
    micro_codegen386_ext_t *ext = amalloc(sizeof(micro_codegen386_ext_t));
    *ext = (micro_codegen386_ext_t){};
    codegen->ext = ext;
    ext->in_function = 0;
    ext->ebp_top_offset = -4;
    sct_hashmap_init(&ext->idents, sizeof(micro_codegen386_ident_t));
    memset(ext->used_regs, 0, sizeof(ext->used_regs));
}

void micro_codegen386_deinit(micro_codegen_t *codegen)
{
    sct_vector_deinit(&codegen->outbuf);
    micro_asm386_deinit();

    sct_hashmap_deinit(&_micro_codegen386_ext(codegen)->idents);
    
    free(codegen->ext);
}