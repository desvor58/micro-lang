#include "../internal.h"

int lowering_goto(micro_codegen_t *codegen, micro_instruction_t *instr)
{
    micro_codegen386_ext_t *ext = _micro_codegen386_ext(codegen);

    if (!ext->in_function) {
        micro_push_err((micro_error_t){
            .err = MICRO_ERROR_GOTO_OUTSIDE_FUNCTION,
            .line_ref = instr->start_tok->line_ref,
            .chpos_ref = instr->start_tok->chpos_ref
        });
        return 1;
    }

    micro_instruction_goto_t instr_goto = instr->goto_lbl;

    char *lbl_name = sct_arena_alloc(&ext->arena, sizeof(char) * strlen(ext->curent_function_name) + strlen(instr_goto.lbl) + 2);
    strcpy(lbl_name, ext->curent_function_name);
    strcat(lbl_name, ".");
    strcat(lbl_name, instr_goto.lbl);

    micro_codegen386_ident_t *ident = sct_hashmap_get(&ext->idents, instr_goto.lbl);
    if (!ident) {
        sct_vector_push(&ext->goto_unfound_labels, &(micro_codegen386_goto_unfound_lbl_t){
            .name = instr_goto.lbl,
            .lbl_tok = instr->start_tok++,
        });
    } else
    if (ident->type != MICRO_IDENT_LBL) {
        micro_push_err((micro_error_t){
            .err = MICRO_ERROR_IDENT_NOT_LBL,
            .line_ref = instr->start_tok[1].line_ref,
            .chpos_ref = instr->start_tok[1].chpos_ref
        });
        return 1;
    }

    push_asm_instr(MICRO_ASM386_INSTR_JMP_L32, { .lbl_name = lbl_name }, {});
    return 0;
}