#include <micro/codegen/386/statements.h>

void micro_codegen_386__if(micro_codegen_t *codegen)
{
    // micro_token_t tok_expr_start = __micro_peek(codegen, 1);
    // if (tok_expr_start.type == MICRO_TT_NULL || tok_expr_start.type != MICRO_TT_IDENT) {
    //     micro_push_err((micro_error_t){
    //         .msg = "Expected expression after 'if' keyword",
    //         .line_ref = codegen->toks->toks[codegen->toks_pos].line_ref,
    //         .chpos_ref = codegen->toks->toks[codegen->toks_pos].chpos_ref
    //     });
    //     goto err_exit;
    // }

    int offset = micro_codegen_386_expr_parse(codegen,
        codegen->toks_pos + 1,
        (micro_codegen_386_storage_info_t){
            .is_unsigned = 0,
            .offset = REG32_EAX,
            .size = MICRO_SZ_32,
            .type = MICRO_ST_REG,
        }
    );
    
    asm386_testR32R32(REG32_EAX, REG32_EAX);
    asm_put_instructions(codegen);

    codegen->toks_pos += offset;
    if (codegen->toks->toks[codegen->toks_pos += 2].type != MICRO_TT_IDENT) {
        micro_push_err((micro_error_t){
            .msg = "Expected label",
            .line_ref = codegen->toks->toks[codegen->toks_pos].line_ref,
            .chpos_ref = codegen->toks->toks[codegen->toks_pos].chpos_ref,
        });
        return;
    }

    i32 o = asm386_jzL32(micro_imm_le_gen(0));
    micro_defer_addr_ref_t *ref = malloc(sizeof(micro_defer_addr_ref_t));
    ref->outbuf_ref = codegen->outbuf->size + o;
    ref->lbl_name = codegen->toks->toks[codegen->toks_pos].val;
    ref->lbl_code_ref = codegen->toks_pos;

    sct_list_push_back(get_codegen_386_ext(codegen)->defer_addr_refs, ref);
    asm_put_instructions(codegen);
    
    while (codegen->toks_pos < codegen->toks->size && codegen->toks->toks[codegen->toks_pos].type != MICRO_TT_SEMICOLON) {
        codegen->toks_pos++;
    }
    return;
}
