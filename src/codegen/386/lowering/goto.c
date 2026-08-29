#include "../internal.h"

int lowering_goto(micro_codegen_t *codegen, micro_instruction_t *instr)
{
    micro_codegen386_ext_t *ext = _micro_codegen386_ext(codegen);

    if (!codegen->flags.no_err_outside_fun && !ext->in_function) {
        micro_push_err((micro_error_t){
            .err = MICRO_ERROR_GOTO_OUTSIDE_FUNCTION,
            .instr = MICRO_INSTR_GOTO
        });
        return 1;
    }

    micro_instruction_goto_t instr_goto = instr->goto_lbl;

    char *lbl_name = sct_arena_alloc(ext->arena, sizeof(char) * strlen(ext->curent_function_name) + strlen(instr_goto.lbl) + 2);
    strcpy(lbl_name, ext->curent_function_name);
    strcat(lbl_name, ".");
    strcat(lbl_name, instr_goto.lbl);

    micro_codegen386_ident_t *ident = sct_hashmap_get(&ext->idents, instr_goto.lbl);
    if (!ident) {
        char *name_copy = sct_arena_alloc(ext->arena, strlen(instr_goto.lbl) + 1);
        strcpy(name_copy, instr_goto.lbl);
        sct_vector_push(&ext->goto_unfound_labels, &(micro_codegen386_goto_unfound_lbl_t){
            .name = name_copy,
        });
    } else
    if (ident->type != MICRO_IDENT_LBL) {
        micro_push_err((micro_error_t){
            .err = MICRO_ERROR_IDENT_NOT_LBL,
            .instr = MICRO_INSTR_GOTO
        });
        return 1;
    }

    push_asm_instr(MICRO_ASM386_INSTR_JMP_L32, { .lbl_name = lbl_name }, {});
    return 0;
}