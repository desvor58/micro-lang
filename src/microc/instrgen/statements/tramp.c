#include <microc/instrgen.h>

void mc_instrgen_parse_tramp(mc_instrgen_t *instrgen)
{
    mc_token_t *tramp_tok = sct_vector_get(instrgen->toks, instrgen->pos++);
    if (unlikely(!tramp_tok || tramp_tok->type != MC_TOK_KW_TRAMP)) {
        micro_push_err((micro_error_t){
            .err = MICRO_ERROR_EXPECTED_TRAMP_KW,
            .instr = MICRO_INSTR_TRAMP
        });
        goto exit;
    }

    if (instrgen->code_in_function) {
        micro_push_err((micro_error_t){
            .err = MICRO_ERROR_TRAMP_INSIDE_FUNCTION,
            .instr = MICRO_INSTR_TRAMP
        });
        goto exit;
    }

    mc_token_t *name_tok = sct_vector_get(instrgen->toks, instrgen->pos++);
    if (!name_tok || name_tok->type != MC_TOK_IDENT) {
        micro_push_err((micro_error_t){
            .err = MICRO_ERROR_EXPECTED_TRAMP_NAME,
            .instr = MICRO_INSTR_TRAMP
        });
        goto exit;
    }

    micro_instruction_tramp_t instr;
    strcpy(instr.name, name_tok->val);
    sct_vector_init(&instr.args, sizeof(micro_instruction_fun_arg_t));
    instr.ret_type = MICRO_TYPE_NULL;

    micro_instruction_hints_t hints = {
        .lifetime = -1,
        .forced_stack = 0,
        .lazy_init = 0,
    };

    mc_token_t *tok = sct_vector_get(instrgen->toks, instrgen->pos);
    if (tok && tok->type == MC_TOK_LBRACE) {
        if (!mc_instrgen_parse_hints(instrgen, &hints, MICRO_INSTR_TRAMP)) {
            goto exit;
        }
        tok = sct_vector_get(instrgen->toks, instrgen->pos);
    }

    while (tok && tok->type == MC_TOK_TYPE_NAME) {
        mc_token_t *arg_type_tok = sct_vector_get(instrgen->toks, instrgen->pos++);
        mc_token_t *arg_name_tok = sct_vector_get(instrgen->toks, instrgen->pos++);
        if (!arg_name_tok || arg_name_tok->type != MC_TOK_IDENT) {
            micro_push_err((micro_error_t){
                .err = MICRO_ERROR_EXPECTED_ARG_NAME,
                .instr = MICRO_INSTR_TRAMP
            });
            goto exit;
        }

        micro_instruction_fun_arg_t arg;
        arg.type = mc_type_str_parse(arg_type_tok->val);
        strcpy(arg.name, arg_name_tok->val);

        sct_vector_push(&instr.args, &arg);

        tok = sct_vector_get(instrgen->toks, instrgen->pos);
    }

    if (tok && tok->type == MC_TOK_KW_RET) {
        mc_token_t *ret_type_tok = sct_vector_get(instrgen->toks, ++instrgen->pos);
        if (!ret_type_tok || ret_type_tok->type != MC_TOK_TYPE_NAME) {
            micro_push_err((micro_error_t){
                .err = MICRO_ERROR_EXPECTED_RET_TYPE,
                .instr = MICRO_INSTR_TRAMP
            });
            goto exit;
        }

        instr.ret_type = mc_type_str_parse(ret_type_tok->val);
        instrgen->pos++;
        tok = sct_vector_get(instrgen->toks, instrgen->pos);
    }

    if (!tok || tok->type != MC_TOK_KW_END) {
        micro_push_err((micro_error_t){
            .err = MICRO_ERROR_EXPECTED_END_KW,
            .instr = MICRO_INSTR_TRAMP
        });
        goto exit;
    }

    sct_vector_push(&instrgen->instructions, &(micro_instruction_t){
        .type = MICRO_INSTR_TRAMP,
        .hints = hints,
        .tramp = instr,
    });

exit:
    while (instrgen->pos < instrgen->toks->size
        && ((mc_token_t *)sct_vector_get(instrgen->toks, instrgen->pos))->type != MC_TOK_KW_END) {
        instrgen->pos++;
    }
}