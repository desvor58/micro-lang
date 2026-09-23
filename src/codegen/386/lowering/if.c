#include "../internal.h"

static micro_asm386_instruction_type_t get_jump_instr(micro_expr_tok_t *tok, micro_type_t cond_expr_type)
{
    micro_asm386_instruction_type_t instr = MICRO_ASM386_INSTR_JNZ_L32;

    if (tok->type == MICRO_EXPR_TOK_EXCLAMATION) {
        instr = get_jump_instr(tok + 1, cond_expr_type);
        instr = (micro_asm386_instruction_type_t[]){
            [MICRO_ASM386_INSTR_JZ_L32]   = MICRO_ASM386_INSTR_JNZ_L32,
            [MICRO_ASM386_INSTR_JNZ_L32]  = MICRO_ASM386_INSTR_JZ_L32,

            [MICRO_ASM386_INSTR_JL_L32]   = MICRO_ASM386_INSTR_JNL_L32,
            [MICRO_ASM386_INSTR_JNL_L32]  = MICRO_ASM386_INSTR_JL_L32,
            [MICRO_ASM386_INSTR_JG_L32]   = MICRO_ASM386_INSTR_JNG_L32,
            [MICRO_ASM386_INSTR_JNG_L32]  = MICRO_ASM386_INSTR_JG_L32,
            [MICRO_ASM386_INSTR_JB_L32]   = MICRO_ASM386_INSTR_JNB_L32,
            [MICRO_ASM386_INSTR_JNB_L32]  = MICRO_ASM386_INSTR_JB_L32,
            [MICRO_ASM386_INSTR_JA_L32]   = MICRO_ASM386_INSTR_JNA_L32,
            [MICRO_ASM386_INSTR_JNA_L32]  = MICRO_ASM386_INSTR_JA_L32,
            
            [MICRO_ASM386_INSTR_JLE_L32]   = MICRO_ASM386_INSTR_JNLE_L32,
            [MICRO_ASM386_INSTR_JNLE_L32]  = MICRO_ASM386_INSTR_JLE_L32,
            [MICRO_ASM386_INSTR_JGE_L32]   = MICRO_ASM386_INSTR_JNGE_L32,
            [MICRO_ASM386_INSTR_JNGE_L32]  = MICRO_ASM386_INSTR_JGE_L32,
            [MICRO_ASM386_INSTR_JBE_L32]   = MICRO_ASM386_INSTR_JNBE_L32,
            [MICRO_ASM386_INSTR_JNBE_L32]  = MICRO_ASM386_INSTR_JBE_L32,
            [MICRO_ASM386_INSTR_JAE_L32]   = MICRO_ASM386_INSTR_JNAE_L32,
            [MICRO_ASM386_INSTR_JNAE_L32]  = MICRO_ASM386_INSTR_JAE_L32,
        }[instr];
        return instr;
    } else
    if (tok->type == MICRO_EXPR_TOK_EQ) {
        instr = MICRO_ASM386_INSTR_JZ_L32;
    } else
    if (tok->type == MICRO_EXPR_TOK_LESS) {
        if (micro_type_is_unsigned(cond_expr_type)) {
            instr = MICRO_ASM386_INSTR_JB_L32;
        } else {
            instr = MICRO_ASM386_INSTR_JL_L32;
        }
    } else
    if (tok->type == MICRO_EXPR_TOK_GREAT) {
        if (micro_type_is_unsigned(cond_expr_type)) {
            instr = MICRO_ASM386_INSTR_JA_L32;
        } else {
            instr = MICRO_ASM386_INSTR_JG_L32;
        }
    } else
    if (tok->type == MICRO_EXPR_TOK_LESS_OR_EQ) {
        if (micro_type_is_unsigned(cond_expr_type)) {
            instr = MICRO_ASM386_INSTR_JBE_L32;
        } else {
            instr = MICRO_ASM386_INSTR_JLE_L32;
        }
    } else
    if (tok->type == MICRO_EXPR_TOK_GREAT_OR_EQ) {
        if (micro_type_is_unsigned(cond_expr_type)) {
            instr = MICRO_ASM386_INSTR_JAE_L32;
        } else {
            instr = MICRO_ASM386_INSTR_JGE_L32;
        }
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

    expr_info_t cond_expr_info = { 0, MICRO_TYPE_NULL };
    if (instr_if.cond_expr->type == MICRO_EXPR_TOK_EXCLAMATION) {
        micro_expr_tok_t *tok = instr_if.cond_expr + 1;
        while (tok->type == MICRO_EXPR_TOK_EXCLAMATION) {
            tok = tok + 1;
        }
        cond_expr_info = cond_expr_parse(codegen, tok);
    } else {
        cond_expr_info = cond_expr_parse(codegen, instr_if.cond_expr);
    }
    if (!cond_expr_info.size) {
        micro_push_err((micro_error_t){
            .err = MICRO_ERROR_EXPECTED_EXPRESSION,
            .instr = MICRO_INSTR_IF
        });
        return 1;
    }

    micro_asm386_instruction_type_t asm_instr = get_jump_instr(instr_if.cond_expr, cond_expr_info.type);
    
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

    push_asm_instr(asm_instr, operand_lbl(MICRO_SIZE_32, lbl_name), operand_none());
    return 0;
}