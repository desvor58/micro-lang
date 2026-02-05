#include "../../statements.h"

void micro_codegen_386__goto()
{
    micro_goto_ref_t *ref = malloc(sizeof(micro_goto_ref_t));
    ref->code_ref = micro_pos;
    ref->outbuf_ref = micro_outbuf->size;
    sct_list_push_back(micro_goto_refs, ref);
    for (size_t i = 0; i < 5; i++) {
        sct_string_push_back(micro_outbuf, 0);
    }
}