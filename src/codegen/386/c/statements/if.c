#include "../../statements.h"

void micro_codegen_386__if()
{
    micro_token_t tok_expr_start = __micro_peek(1);
    if (tok_expr_start.type == MICRO_TT_NULL || tok_expr_start.type != MICRO_TT_IDENT) {
        micro_error_t err = {.msg = "Expected expression after 'if' keyword",
                             .line_ref = micro_toks[micro_pos].line_ref,
                             .chpos_ref = micro_toks[micro_pos].chpos_ref};
        __micro_push_err(err);
        goto err_exit;
    }
    
    

err_exit:
    while (micro_pos < micro_toks_size && micro_toks[micro_pos].type != MICRO_TT_SEMICOLON) {
        micro_pos++;
    }
    return;
}
