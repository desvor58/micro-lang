#include "../internal.h"

int lowering_fun(micro_codegen_t *codegen, micro_instruction_t *instr)
{
    micro_codegen386_ext_t *ext = _micro_codegen386_ext(codegen);

    micro_instruction_fun_t instr_fun = instr->fun;

    micro_asm386_prelude();
    micro_asm386_put_instructions(&codegen->outbuf);
    size_t sub_imm_addr = micro_asm386_instr_stack.size + 2;

    micro_asm386_subR32I32(MICRO_ASM386_REG32_ESP, micro_imm_le_gen(0));
    micro_asm386_put_instructions(&codegen->outbuf);
    size_t callee_save_addr = micro_asm386_instr_stack.size;

    ext->ebp_top_offset = -4;
    ext->max_stack_offset = 0;
    ext->in_function = 1;
    ext->use_callee_save_regs = 0;
    for (int i = 0; i < 8; i++) {
        ext->used_regs[i] = 0;
    }

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

    micro_imm_le_t imm = micro_imm_le_gen(ext->max_stack_offset);

    for (size_t i = 0; i < 4; i++) {
        memcpy((void*)(micro_asm386_instr_stack.data + sub_imm_addr * codegen->outbuf._item_size + i), imm.bytes, 4);
    }

    if (ext->use_callee_save_regs) {
        micro_asm386_pushR32(3);
        micro_asm386_pushR32(6);
        micro_asm386_pushR32(7);
        micro_asm386_insert_instructions_to_addr(&codegen->outbuf, callee_save_addr);

        micro_asm386_popR32(3);
        micro_asm386_popR32(6);
        micro_asm386_popR32(7);
        micro_asm386_put_instructions(&codegen->outbuf);
    }

    micro_asm386_epilogue();
    micro_asm386_put_instructions(&codegen->outbuf);

    return 0;
}
