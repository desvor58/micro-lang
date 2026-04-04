#include "../../statements.h"

void micro_codegen_386__goto(micro_codegen_t *codegen)
{
    micro_goto_ref_t *ref = malloc(sizeof(micro_goto_ref_t));
    ref->code_ref = codegen->toks_pos;
    ref->outbuf_ref = codegen->outbuf->size;
    sct_list_push_back(get_codegen_386_ext(codegen)->goto_refs, ref);
    for (size_t i = 0; i < 5; i++) {
        sct_string_push_back(codegen->outbuf, 0);
    }
}