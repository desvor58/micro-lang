#include "../internal.h"

int lowering_if(micro_codegen_t *codegen, micro_instruction_t *instr)
{
    micro_codegen386_ext_t *ext = _micro_codegen386_ext(codegen);

    if (!ext->in_function) {
        micro_push_err((micro_error_t){
            .err = MICRO_ERROR_IF_OUTSIDE_FUNCTION,
            .line_ref = instr->start_tok->line_ref,
            .chpos_ref = instr->start_tok->chpos_ref
        });
        return 1;
    }

    micro_instruction_if_t instr_if = instr->if_goto;

    size_t expr_size = cond_expr_parse(codegen, instr_if.cond_expr);
    if (!expr_size) {
        return 1;
    }
    
    char *lbl_name = sct_arena_alloc(&ext->arena, sizeof(char) * strlen(ext->curent_function_name) + strlen(instr_if.lbl_name) + 2);
    strcpy(lbl_name, ext->curent_function_name);
    strcat(lbl_name, ".");
    strcat(lbl_name, instr_if.lbl_name);

    micro_codegen386_ident_t *ident = sct_hashmap_get(&ext->idents, instr_if.lbl_name);
    if (!ident) {
        char *name_copy = sct_arena_alloc(&ext->arena, strlen(instr_if.lbl_name) + 1);
        strcpy(name_copy, instr_if.lbl_name);
        sct_vector_push(&ext->goto_unfound_labels, &(micro_codegen386_goto_unfound_lbl_t){
            .name = name_copy,
            .lbl_tok = instr->start_tok,
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

    push_asm_instr(MICRO_ASM386_INSTR_JNZ_L32, { .lbl_name = lbl_name }, {});
    return 0;
}