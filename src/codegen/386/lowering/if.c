#include "../internal.h"

static size_t get_jump_instr(micro_expr_tok_t *tok)
{
    micro_asm386_instruction_type_t instr = MICRO_ASM386_INSTR_JNZ_L32;

    if (tok->type == MICRO_EXPR_TOK_EXCLAMATION) {
        instr = get_jump_instr(tok + 1);
        instr = (micro_asm386_instruction_type_t[]){
            [MICRO_ASM386_INSTR_JZ_L32]  = MICRO_ASM386_INSTR_JNZ_L32,
            [MICRO_ASM386_INSTR_JNZ_L32] = MICRO_ASM386_INSTR_JZ_L32,
            [MICRO_ASM386_INSTR_JL_L32]  = MICRO_ASM386_INSTR_JNL_L32,
            [MICRO_ASM386_INSTR_JNL_L32] = MICRO_ASM386_INSTR_JL_L32,
            [MICRO_ASM386_INSTR_JG_L32]  = MICRO_ASM386_INSTR_JNG_L32,
            [MICRO_ASM386_INSTR_JNG_L32] = MICRO_ASM386_INSTR_JG_L32,
        }[instr];
        return instr;
    } else
    if (tok->type == MICRO_EXPR_TOK_EQ) {
        instr = MICRO_ASM386_INSTR_JZ_L32;
    } else
    if (tok->type == MICRO_EXPR_TOK_LESS) {
        instr = MICRO_ASM386_INSTR_JL_L32;
    } else
    if (tok->type == MICRO_EXPR_TOK_GREAT) {
        instr = MICRO_ASM386_INSTR_JG_L32;
    }
    return instr;
}

int lowering_if(micro_codegen_t *codegen, micro_instruction_t *instr)
{
    micro_codegen386_ext_t *ext = _micro_codegen386_ext(codegen);

    if (!codegen->flags.no_err_outside_fun && !ext->in_function) {
        micro_push_err((micro_error_t){
            .err = MICRO_ERROR_IF_OUTSIDE_FUNCTION,
            .instr = MICRO_INSTR_IF
        });
        return 1;
    }

    micro_instruction_if_t instr_if = instr->if_goto;

    micro_asm386_instruction_type_t asm_instr = get_jump_instr(instr_if.cond_expr);

    size_t cond_expr_size = 0;
    if (instr_if.cond_expr->type == MICRO_EXPR_TOK_EXCLAMATION) {
        micro_expr_tok_t *tok = instr_if.cond_expr + 1;
        while (tok->type == MICRO_EXPR_TOK_EXCLAMATION) {
            tok = tok + 1;
        }
        cond_expr_size = cond_expr_parse(codegen, tok);
    } else {
        cond_expr_size = cond_expr_parse(codegen, instr_if.cond_expr);
    }
    if (!cond_expr_size) {
        micro_push_err((micro_error_t){
            .err = MICRO_ERROR_EXPECTED_EXPRESSION,
            .instr = MICRO_INSTR_IF
        });
        return 1;
    }
    
    char *lbl_name = sct_arena_alloc(ext->arena, sizeof(char) * strlen(ext->curent_function_name) + strlen(instr_if.lbl_name) + 2);
    strcpy(lbl_name, ext->curent_function_name);
    strcat(lbl_name, ".");
    strcat(lbl_name, instr_if.lbl_name);

    micro_codegen386_ident_t *ident = sct_hashmap_get(&ext->idents, instr_if.lbl_name);
    if (!ident) {
        char *name_copy = sct_arena_alloc(ext->arena, strlen(instr_if.lbl_name) + 1);
        strcpy(name_copy, instr_if.lbl_name);
        sct_vector_push(&ext->goto_unfound_labels, &(micro_codegen386_goto_unfound_lbl_t){
            .name = name_copy,
        });
    } else
    if (ident->type != MICRO_IDENT_LBL) {
        micro_push_err((micro_error_t){
            .err = MICRO_ERROR_IDENT_NOT_LBL,
            .instr = MICRO_INSTR_IF
        });
        return 1;
    }

    push_asm_instr(asm_instr, { .lbl_name = lbl_name }, {});
    return 0;
}