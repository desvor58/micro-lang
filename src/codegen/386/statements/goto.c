#include <micro/codegen/386/statements.h>

void micro_codegen_386__goto(micro_codegen_t *codegen)
{
    i32 o = asm386_jmpL32(micro_imm_le_gen(0));
    micro_defer_addr_ref_t *ref = malloc(sizeof(micro_defer_addr_ref_t));
    ref->lbl_code_ref = codegen->toks_pos + 1;
    ref->outbuf_ref = codegen->outbuf->size + o;
    ref->lbl_name = codegen->toks->toks[codegen->toks_pos + 1].val;
    sct_list_push_back(get_codegen_386_ext(codegen)->defer_addr_refs, ref);
    asm_put_instructions(codegen);
}
