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
    codegen->emit = micro_codegen386_emit;
    micro_codegen386_ext_t *ext = amalloc(sizeof(micro_codegen386_ext_t));
    *ext = (micro_codegen386_ext_t){};
    codegen->ext = ext;
    sct_arena_init(&ext->arena);
    sct_vector_init(&codegen->asm_instrs, sizeof(micro_asm386_instruction_t));
    ext->in_function = 0;
    ext->use_callee_save_regs = 0;
    ext->ebp_top_offset = -4;
    ext->max_stack_offset = 0;
    ext->curent_function_name = "";
    sct_vector_init(&ext->goto_unfound_labels, sizeof(micro_codegen386_goto_unfound_lbl_t));
    sct_hashmap_init(&ext->idents, sizeof(micro_codegen386_ident_t));
    memset(ext->used_regs, 0, sizeof(ext->used_regs));
}

void micro_codegen386_deinit(micro_codegen_t *codegen)
{
    sct_vector_deinit(&codegen->outbuf);

    sct_vector_deinit(&codegen->asm_instrs);

    sct_hashmap_deinit(&_micro_codegen386_ext(codegen)->idents);

    sct_arena_deinit(&_micro_codegen386_ext(codegen)->arena);
    
    free(codegen->ext);
}