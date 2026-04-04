#include "../../statements.h"

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
    
err_exit:
    while (codegen->toks_pos < codegen->toks->size && codegen->toks->toks[codegen->toks_pos].type != MICRO_TT_SEMICOLON) {
        codegen->toks_pos++;
    }
    return;
}
