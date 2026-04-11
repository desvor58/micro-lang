#include <micro/codegen/386/statements.h>

void micro_codegen_386__if(micro_codegen_t *codegen)
{
    micro_token_t tok_expr_start = __micro_peek(codegen, 1);
    if (tok_expr_start.type == MICRO_TT_NULL || tok_expr_start.type != MICRO_TT_IDENT) {
        micro_push_err((micro_error_t){
            .msg = "Expected expression after 'if' keyword",
            .line_ref = codegen->toks->toks[codegen->toks_pos].line_ref,
            .chpos_ref = codegen->toks->toks[codegen->toks_pos].chpos_ref
        });
        goto err_exit;
    }

    micro_codegen_386_ident_info_t *ident = sct_hashmap_get(get_codegen_386_ext(codegen)->idents, tok_expr_start.val);
    if (ident->type != MICRO_IT_VAR) {
        micro_push_err((micro_error_t){
            .msg = "Expected variable",
            .line_ref = tok_expr_start.line_ref,
            .chpos_ref = tok_expr_start.chpos_ref,
        });
        goto err_exit;
    }

    micro_codegen_386_storage_info_t storage_info = ident->var_info.storage_info;
    switch (storage_info.type) {
        case MICRO_ST_DATASEG: {
            void (*static_var_test_tbl[])(micro_addr_le_t, micro_imm_le_t) = {
                [MICRO_SZ_8]  = asm386_testM8I8,
                [MICRO_SZ_16] = asm386_testM16I16,
                [MICRO_SZ_32] = asm386_testM32I32,
            };
            static_var_test_tbl[storage_info.size](micro_imm_le_gen(storage_info.offset), micro_imm_le_gen(0));
        } break;

        case MICRO_ST_STACK: {
            void (*local_var_test_tbl[])(micro_addr_le_t, micro_imm_le_t) = {
                [MICRO_SZ_8]  = asm386_testS32I8,
                [MICRO_SZ_16] = asm386_testS32I16,
                [MICRO_SZ_32] = asm386_testS32I32,
            };
            local_var_test_tbl[storage_info.size](micro_imm_le_gen(storage_info.offset), micro_imm_le_gen(0));
        } break;

        case MICRO_ST_REG: {
            void (*reg_var_test_tbl[])(asm386_reg, asm386_reg) = {
                [MICRO_SZ_8]  = asm386_testR8R8,
                [MICRO_SZ_16] = asm386_testR16R16,
                [MICRO_SZ_32] = asm386_testR32R32,
            };
            reg_var_test_tbl[storage_info.size]((asm386_reg)storage_info.offset, (asm386_reg)storage_info.offset);
        } break;
    }
    asm_put_instructions(codegen);

    i32 o = asm386_jzL32(micro_imm_le_gen(0));
    micro_defer_addr_ref_t *ref = malloc(sizeof(micro_defer_addr_ref_t));
    ref->outbuf_ref = codegen->outbuf->size + o;

    while (codegen->toks->toks[codegen->toks_pos].type != MICRO_TT_COLON) {
        codegen->toks_pos++;
    }
    if (codegen->toks->toks[++codegen->toks_pos].type != MICRO_TT_IDENT) {
        micro_push_err((micro_error_t){
            .msg = "Expected label",
            .line_ref = codegen->toks->toks[codegen->toks_pos].line_ref,
            .chpos_ref = codegen->toks->toks[codegen->toks_pos].chpos_ref,
        });
        return;
    }

    ref->lbl_name = codegen->toks->toks[codegen->toks_pos].val;
    ref->lbl_code_ref = codegen->toks_pos;

    sct_list_push_back(get_codegen_386_ext(codegen)->defer_addr_refs, ref);
    asm_put_instructions(codegen);
    
err_exit:
    while (codegen->toks_pos < codegen->toks->size && codegen->toks->toks[codegen->toks_pos].type != MICRO_TT_SEMICOLON) {
        codegen->toks_pos++;
    }
    return;
}
