#ifndef MICRO_CODEGEN_386_H
#define MICRO_CODEGEN_386_H

#include "386/common.h"
#include "386/statements.h"

void micro_codegen_386_micro_instruction_parse()
{
    if (micro_toks[micro_pos].type == MICRO_TT_KW_VAR) {
        if (micro_code_in_function) {
            micro_codegen_386__var();
        } else {
            micro_codegen_386__static_var();
        }
    } else

    if (micro_toks[micro_pos].type == MICRO_TT_KW_SET) {
        micro_codegen_386__set();
    } else
    if (micro_toks[micro_pos].type == MICRO_TT_KW_FUN) {
        micro_codegen_386__fun();
    } else
    if (micro_toks[micro_pos].type == MICRO_TT_KW_CALL) {
        micro_codegen_386__call();
    } else
    if (micro_toks[micro_pos].type == MICRO_TT_KW_GOTO) {
        micro_codegen_386__goto();
    } else
    if (micro_toks[micro_pos].type == MICRO_TT_IDENT) {
        if (micro_pos == micro_toks_size - 1) {
            micro_error_t err = {.msg = "Expected ':' for the label",
                                .line_ref = micro_toks[micro_pos].line_ref,
                                .chpos_ref = micro_toks[micro_pos].chpos_ref};
            __micro_push_err(err);
            return;
        }
        if (micro_toks[micro_pos + 1].type == MICRO_TT_COLON) {
            micro_codegen_386_lbl_info_t lbl_info;
            strcpy(lbl_info.name, micro_toks[micro_pos].val);
            lbl_info.offset = micro_outbuf->size;
            micro_codegen_386_ident_info_t *ident_info = malloc(sizeof(micro_codegen_386_ident_info_t));
            ident_info->type = IT_LBL;
            ident_info->lbl_info = lbl_info;
            sct_hashmap_set(micro_codegen_386_idents, micro_toks[micro_pos].val, ident_info);
        }
    }
}

void micro_codegen_386()
{
    for (micro_pos = 0; micro_pos < micro_toks_size; micro_pos++) {
        micro_codegen_386_micro_instruction_parse();
    }
    foreach (ref_it, micro_goto_refs) {
        size_t micro_pos_save = micro_pos;
        micro_pos = ((micro_goto_ref_t*)ref_it->val)->code_ref;

        micro_token_t tok_lbl = __micro_peek(1);
        if (tok_lbl.type != MICRO_TT_IDENT) {
            micro_error_t err = {.msg = "Expected label name",
                                .line_ref = tok_lbl.line_ref,
                                .chpos_ref = tok_lbl.chpos_ref};
            __micro_push_err(err);
            goto err_exit;
        }

        micro_codegen_386_ident_info_t *ident_info = sct_hashmap_get(micro_codegen_386_idents, tok_lbl.val);
        if (!ident_info) {
            micro_error_t err = {.msg = "Undefined label name",
                                .line_ref = tok_lbl.line_ref,
                                .chpos_ref = tok_lbl.chpos_ref};
            __micro_push_err(err);
            goto err_exit;
        }
        if (ident_info->type != IT_LBL) {
            micro_error_t err = {.msg = "Expected label name",
                                .line_ref = tok_lbl.line_ref,
                                .chpos_ref = tok_lbl.chpos_ref};
            __micro_push_err(err);
            goto err_exit;
        }
        

        u8 addr[4];
        micro_gen32imm_le(addr, ident_info->lbl_info.offset - (((micro_goto_ref_t*)ref_it->val)->outbuf_ref + 5));
        u8 instruction[] = asm_jmpL32(addr);
        push_instruction2addr(instruction, ((micro_goto_ref_t*)ref_it->val)->outbuf_ref);
        goto exit;

err_exit:
        while (micro_pos < micro_toks_size && micro_toks[micro_pos].type != MICRO_TT_SEMICOLON) {
            micro_pos++;
        }
exit:
        micro_pos = micro_pos_save;
    }
}

#endif