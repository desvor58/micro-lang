#include "../../statements.h"

void micro_codegen_386__call()
{
    if (!micro_code_in_function) {
        micro_error_t err = {.msg = "Instruction 'call' can be usage only in function body",
                             .line_ref = micro_toks[micro_pos].line_ref,
                             .chpos_ref = micro_toks[micro_pos].chpos_ref};
        __micro_push_err(err);
        goto err_exit;
    }

    micro_token_t tok_dst_var_name = __micro_get(1);
    micro_token_t tok_fun_name     = __micro_get(1);
    micro_pos++;

    if (tok_dst_var_name.type != MICRO_TT_IDENT || tok_dst_var_name.type == MICRO_TT_NULL) {
        micro_error_t err = {.msg = "Expected destination variable name after 'call' keyword",
                             .line_ref = micro_toks[micro_pos].line_ref,
                             .chpos_ref = micro_toks[micro_pos].chpos_ref};
        __micro_push_err(err);
        goto err_exit;
    }
    if (tok_fun_name.type != MICRO_TT_IDENT || tok_fun_name.type == MICRO_TT_NULL) {
        micro_error_t err = {.msg = "Expected called function name after destination variable name",
                             .line_ref = micro_toks[micro_pos].line_ref,
                             .chpos_ref = micro_toks[micro_pos].chpos_ref};
        __micro_push_err(err);
        goto err_exit;
    }

    micro_codegen_386_ident_info_t *fun_ident_info = sct_hashmap_get(micro_codegen_386_idents, tok_fun_name.val);
    if (!fun_ident_info) {
        micro_error_t err = {.msg = "Unknown function name",
                             .line_ref = tok_fun_name.line_ref,
                             .chpos_ref = tok_fun_name.chpos_ref};
        __micro_push_err(err);
        goto err_exit;
    }
    if (fun_ident_info->type != IT_FUN) {
        micro_error_t err = {.msg = "Expected function name",
                             .line_ref = tok_fun_name.line_ref,
                             .chpos_ref = tok_fun_name.chpos_ref};
        __micro_push_err(err);
        goto err_exit;
    }

    size_t arg_expr_starts[16] = {0};
    size_t cur_arg_num = 0;
    while (micro_toks[micro_pos].type != MICRO_TT_SEMICOLON) {
        arg_expr_starts[cur_arg_num++] = micro_pos;
        micro_pos += micro_expr_peek(micro_pos) - 1;

        if (++micro_pos >= micro_toks_size) {
            micro_error_t err = {.msg = "Expected ';'",
                                 .line_ref = micro_toks[micro_pos - 1].line_ref,
                                 .chpos_ref = micro_toks[micro_pos - 1].chpos_ref};
            __micro_push_err(err);
            goto err_exit;
        }
    }
    for (ptrdiff_t i = cur_arg_num - 1, arg_offset = 0; i >= 0; i--,arg_offset++) {
        micro_codegen_386_expr_parse(arg_expr_starts[i], (micro_codegen_386_storage_info_t){
            .type = MICRO_ST_STACK,
            .size = 4,
            .offset = micro_top_stack_offset - (arg_offset * 4),
            .is_unsigned = 0
        });
    }


    micro_addr_le_t call_addr = micro_imm_le_gen(fun_ident_info->fun_info.offset - (micro_outbuf->size + 5));
    asm386_call(call_addr);
    asm_put_instructions();

    if (strcmp(tok_dst_var_name.val, "_")) {
        micro_codegen_386_ident_info_t *dst_var_ident_info = sct_hashmap_get(micro_codegen_386_idents, tok_dst_var_name.val);
        if (!dst_var_ident_info) {
            micro_error_t err = {.msg = "Undeclareted variable name",
                                .line_ref = tok_dst_var_name.line_ref,
                                .chpos_ref = tok_dst_var_name.chpos_ref};
            __micro_push_err(err);
            goto err_exit;
        }
        if (dst_var_ident_info->type != IT_VAR) {
            micro_error_t err = {.msg = "Expected variable name",
                                .line_ref = tok_dst_var_name.line_ref,
                                .chpos_ref = tok_dst_var_name.chpos_ref};
            __micro_push_err(err);
            goto err_exit;
        }

        if (dst_var_ident_info->var_info.type != fun_ident_info->fun_info.ret_type) {
            micro_error_t err = {.msg = "Wrong return type of function for this variable",
                                .line_ref = tok_dst_var_name.line_ref,
                                .chpos_ref = tok_dst_var_name.chpos_ref};
            __micro_push_err(err);
            goto err_exit;
        }

        if (dst_var_ident_info->var_info.storage_info.type == MICRO_ST_DATASEG) {
            micro_addr_le_t addr = micro_imm_le_gen(dst_var_ident_info->var_info.storage_info.offset);
            
            void (*instr_tbl[])(micro_addr_le_t, asm386_reg) = {
                [MICRO_SZ_8]  = asm386_movM8R8,
                [MICRO_SZ_16] = asm386_movM16R16,
                [MICRO_SZ_32] = asm386_movM32R32,
            };
            instr_tbl[dst_var_ident_info->var_info.storage_info.size](addr, REG32_EAX);
            asm_put_instructions();
        }
    }
    return;

err_exit:
    while (micro_pos < micro_toks_size && micro_toks[micro_pos].type != MICRO_TT_SEMICOLON) {
        micro_pos++;
    }
    return;
}