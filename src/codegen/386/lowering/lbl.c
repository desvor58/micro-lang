#include "../internal.h"

int lowering_lbl(micro_codegen_t *codegen, micro_instruction_t *instr)
{
    micro_codegen386_ext_t *ext = _micro_codegen386_ext(codegen);

    if (!codegen->flags.no_err_outside_fun && !ext->in_function) {
        micro_push_err((micro_error_t){
            .err = MICRO_ERROR_LBL_OUTSIDE_FUNCTION,
            .instr = MICRO_INSTR_LBL
        });
        return 1;
    }

    micro_instruction_lbl_t instr_lbl = instr->lbl;

    char *lbl_name = sct_arena_alloc(ext->arena, sizeof(char) * strlen(ext->curent_function_name) + strlen(instr_lbl.name) + 2);
    strcpy(lbl_name, ext->curent_function_name);
    strcat(lbl_name, ".");
    strcat(lbl_name, instr_lbl.name);

    push_asm_instr(MICRO_ASM386_INSTR_LBL, { .lbl_name = lbl_name }, {});

    micro_codegen386_ident_t ident = {
        .type = MICRO_IDENT_LBL,
    };
    strcpy(ident.lbl.name, lbl_name);

    sct_hashmap_add(&ext->idents, instr_lbl.name, &ident);

    for (size_t i = 0; i < ext->goto_unfound_labels.size; i++) {
        micro_codegen386_goto_unfound_lbl_t *lbl = sct_vector_get(&ext->goto_unfound_labels, i);

        if (!strcmp(lbl->name, instr_lbl.name)) {
            sct_vector_erase(&ext->goto_unfound_labels, i);
        }
    }

    return 0;
}