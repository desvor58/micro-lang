#include "../codegen_386.h"

void micro_codegen_386_micro_instruction_parse(micro_codegen_t *codegen)
{
    if (codegen->toks->toks[codegen->toks_pos].type == MICRO_TT_KW_VAR) {
        if (get_codegen_386_ext(codegen)->code_in_function) {
            micro_codegen_386__var(codegen);
        } else {
            micro_codegen_386__static_var(codegen);
        }
    } else
    if (codegen->toks->toks[codegen->toks_pos].type == MICRO_TT_KW_SET) {
        micro_codegen_386__set(codegen);
    } else
    if (codegen->toks->toks[codegen->toks_pos].type == MICRO_TT_KW_FUN) {
        micro_codegen_386__fun(codegen);
    } else
    if (codegen->toks->toks[codegen->toks_pos].type == MICRO_TT_KW_CALL) {
        micro_codegen_386__call(codegen);
    } else
    if (codegen->toks->toks[codegen->toks_pos].type == MICRO_TT_KW_GOTO) {
        micro_codegen_386__goto(codegen);
    } else
    if (codegen->toks->toks[codegen->toks_pos].type == MICRO_TT_IDENT) {
        if (codegen->toks_pos == codegen->toks->size - 1) {
            micro_push_err((micro_error_t){
                .msg = "Expected ':' for the label",
                .line_ref = codegen->toks->toks[codegen->toks_pos].line_ref,
                .chpos_ref = codegen->toks->toks[codegen->toks_pos].chpos_ref
            });
            return;
        }
        if (codegen->toks->toks[codegen->toks_pos + 1].type == MICRO_TT_COLON) {
            micro_codegen_386_lbl_info_t lbl_info;
            strcpy(lbl_info.name, codegen->toks->toks[codegen->toks_pos].val);
            lbl_info.offset = codegen->outbuf->size;
            micro_codegen_386_ident_info_t *ident_info = malloc(sizeof(micro_codegen_386_ident_info_t));
            ident_info->type = IT_LBL;
            ident_info->lbl_info = lbl_info;
            sct_hashmap_set(get_codegen_386_ext(codegen)->idents, codegen->toks->toks[codegen->toks_pos].val, ident_info);
        }
    }
}

void micro_codegen_386(micro_codegen_t *codegen) {
    for (codegen->toks_pos = 0; codegen->toks_pos < codegen->toks->size; codegen->toks_pos++) {
        micro_codegen_386_micro_instruction_parse(codegen);
    }
    foreach (ref_it, ((micro_codegen_386_ext_t*)codegen->ext)->goto_refs) {
        size_t micro_pos_save = codegen->toks_pos;
        codegen->toks_pos = ((micro_goto_ref_t*)ref_it->val)->code_ref;

        micro_token_t tok_lbl = __micro_peek(1);
        if (tok_lbl.type != MICRO_TT_IDENT) {
            micro_push_err((micro_error_t){
                .msg = "Expected label name",
                .line_ref = tok_lbl.line_ref,
                .chpos_ref = tok_lbl.chpos_ref
            });
            goto err_exit;
        }

        micro_codegen_386_ident_info_t *ident_info = sct_hashmap_get(((micro_codegen_386_ext_t*)codegen->ext)->idents, tok_lbl.val);
        if (!ident_info) {
            micro_push_err((micro_error_t){
                .msg = "Undefined label name",
                .line_ref = tok_lbl.line_ref,
                .chpos_ref = tok_lbl.chpos_ref
            });
            goto err_exit;
        }
        if (ident_info->type != IT_LBL) {
            micro_push_err((micro_error_t){
                .msg = "Expected label name",
                .line_ref = tok_lbl.line_ref,
                .chpos_ref = tok_lbl.chpos_ref
            });
            goto err_exit;
        }
        
        micro_imm_le_t addr = micro_imm_le_gen(ident_info->lbl_info.offset - (((micro_goto_ref_t*)ref_it->val)->outbuf_ref + 5));
        asm386_jmpL32(addr);
        asm_put_instructions_to_addr(codegen, (((micro_goto_ref_t*)ref_it->val)->outbuf_ref));
        goto exit;

err_exit:
        while (codegen->toks_pos < codegen->toks->size && codegen->toks->toks[codegen->toks_pos].type != MICRO_TT_SEMICOLON) {
            codegen->toks_pos++;
        }
exit:
        codegen->toks_pos = micro_pos_save;
    }
}

void micro_codegen_386_init(micro_codegen_t *codegen, micro_tok_vec_t *toks)
{
    codegen->toks = toks;
    codegen->toks_pos = 0;
    codegen->outbuf = sct_string_create();
    codegen->ext = malloc(sizeof(micro_codegen_386_ext_t));
    get_codegen_386_ext(codegen)->code_in_function = 0;
    get_codegen_386_ext(codegen)->top_stack_offset = 0;
    get_codegen_386_ext(codegen)->idents = sct_hashmap_create();
    get_codegen_386_ext(codegen)->local_var_list = sct_list_pair_create(0);
    get_codegen_386_ext(codegen)->goto_refs = sct_list_pair_create(0);
}

void micro_codegen_386_deinit(micro_codegen_t *codegen)
{
    sct_string_free(codegen->outbuf);
    sct_hashmap_full_free(get_codegen_386_ext(codegen)->idents);
    sct_list_full_free(get_codegen_386_ext(codegen)->local_var_list);
    sct_list_full_free(get_codegen_386_ext(codegen)->goto_refs);
}
