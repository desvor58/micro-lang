#include <microc/instrgen.h>

void mc_instrgen_parse_fun(mc_instrgen_t *instrgen)
{
    mc_token_t *fun_tok = sct_vector_get(instrgen->toks, instrgen->pos++);
    if (unlikely(!fun_tok || fun_tok->type != MC_TOK_KW_FUN)) {
        micro_push_err((micro_error_t){
            .err = MICRO_ERROR_EXPECTED_FUN_KW,
            .instr = MICRO_INSTR_FUN
        });
        goto exit;
    }

    if (instrgen->code_in_function) {
        micro_push_err((micro_error_t){
            .err = MICRO_ERROR_FUN_INSIDE_FUNCTION,
            .instr = MICRO_INSTR_FUN
        });
        goto exit;
    }

    mc_token_t *name_tok = sct_vector_get(instrgen->toks, instrgen->pos++);
    if (!name_tok || name_tok->type != MC_TOK_IDENT) {
        micro_push_err((micro_error_t){
            .err = MICRO_ERROR_EXPECTED_FUN_NAME,
            .instr = MICRO_INSTR_FUN
        });
        goto exit;
    }

    micro_instruction_fun_t instr;
    strcpy(instr.name, name_tok->val);
    sct_vector_init(&instr.args, sizeof(micro_instruction_fun_arg_t));
    instr.ret_type = MICRO_TYPE_NULL;

    micro_instruction_hints_t hints = {0};
    mc_token_t *tok = sct_vector_get(instrgen->toks, instrgen->pos);
    if (tok && tok->type == MC_TOK_LBRACE) {
        if (!mc_instrgen_parse_hints(instrgen, &hints, MICRO_INSTR_FUN)) {
            goto exit;
        }
        tok = sct_vector_get(instrgen->toks, instrgen->pos);
        if (tok && tok->type == MC_TOK_SEMICOLON) {
            instrgen->pos++;
            tok = sct_vector_get(instrgen->toks, instrgen->pos);
        }
    }

    while (tok && tok->type != MC_TOK_KW_RET && tok->type != MC_TOK_KW_START && tok->type != MC_TOK_LBRACE) {
        mc_token_t *arg_type_tok = sct_vector_get(instrgen->toks, instrgen->pos++);
        if (!arg_type_tok || arg_type_tok->type != MC_TOK_TYPE_NAME) {
            micro_push_err((micro_error_t){
                .err = MICRO_ERROR_EXPECTED_ARG_TYPE,
                .instr = MICRO_INSTR_FUN
            });
            goto exit;
        }

        mc_token_t *arg_name_tok = sct_vector_get(instrgen->toks, instrgen->pos++);
        if (!arg_name_tok || arg_name_tok->type != MC_TOK_IDENT) {
            micro_push_err((micro_error_t){
                .err = MICRO_ERROR_EXPECTED_ARG_NAME,
                .instr = MICRO_INSTR_FUN
            });
            goto exit;
        }

        micro_instruction_fun_arg_t arg;
        arg.type = mc_type_str_parse(arg_type_tok->val);
        strcpy(arg.name, arg_name_tok->val);

        sct_vector_push(&instr.args, &arg);

        tok = sct_vector_get(instrgen->toks, instrgen->pos);
    }

    if (tok && tok->type == MC_TOK_LBRACE) {
        if (!mc_instrgen_parse_hints(instrgen, &hints, MICRO_INSTR_FUN)) {
            goto exit;
        }
        tok = sct_vector_get(instrgen->toks, instrgen->pos);
        if (tok && tok->type == MC_TOK_SEMICOLON) {
            instrgen->pos++;
            tok = sct_vector_get(instrgen->toks, instrgen->pos);
        }
    }

    if (tok && tok->type == MC_TOK_KW_RET) {
        mc_token_t *ret_type_tok = sct_vector_get(instrgen->toks, ++instrgen->pos);
        if (!ret_type_tok || ret_type_tok->type != MC_TOK_TYPE_NAME) {
            micro_push_err((micro_error_t){
                .err = MICRO_ERROR_EXPECTED_RET_TYPE,
                .instr = MICRO_INSTR_FUN
            });
            goto exit;
        }

        instr.ret_type = mc_type_str_parse(ret_type_tok->val);
        tok = sct_vector_get(instrgen->toks, instrgen->pos + 1);
        if (tok && tok->type == MC_TOK_LBRACE) {
            instrgen->pos++;
            if (!mc_instrgen_parse_hints(instrgen, &hints, MICRO_INSTR_FUN)) {
                goto exit;
            }
            tok = sct_vector_get(instrgen->toks, instrgen->pos);
            if (tok && tok->type == MC_TOK_SEMICOLON) {
                instrgen->pos++;
            }
        } else {
            instrgen->pos++;
        }
    }

    sct_vector_t body;
    sct_vector_init(&body, sizeof(mc_token_t));

    tok = sct_vector_get(instrgen->toks, instrgen->pos);
    if (!tok || tok->type != MC_TOK_KW_START) {
        micro_push_err((micro_error_t){
            .err = MICRO_ERROR_EXPECTED_START_KW,
            .instr = MICRO_INSTR_FUN
        });
    }
    
    tok = sct_vector_get(instrgen->toks, ++instrgen->pos);
    for (;;) {
        if (!tok) {
            micro_push_err((micro_error_t){
                .err = MICRO_ERROR_EXPECTED_END_KW,
                .instr = MICRO_INSTR_FUN
            });
            return;
        }
        if (tok->type == MC_TOK_KW_END) {
            break;
        }
        sct_vector_push(&body, tok);

        tok = sct_vector_get(instrgen->toks, ++instrgen->pos);
    }

    mc_token_t *end_hint_tok = sct_vector_get(instrgen->toks, instrgen->pos + 1);
    if (end_hint_tok && end_hint_tok->type == MC_TOK_LBRACE) {
        instrgen->pos++;
        if (!mc_instrgen_parse_hints(instrgen, &hints, MICRO_INSTR_FUN)) {
            goto exit;
        }
        mc_token_t *next_tok = sct_vector_get(instrgen->toks, instrgen->pos);
        if (!next_tok || next_tok->type != MC_TOK_SEMICOLON) {
            instrgen->pos--;
        }
    }

    mc_instrgen_t body_instrgen;
    mc_instrgen_init(&body_instrgen, &body);
    body_instrgen.code_in_function = 1;
        mc_instrgen_gen(&body_instrgen);

        sct_vector_init(&instr.body, sizeof(micro_instruction_t));

        for (size_t i = 0; i < body_instrgen.instructions.size; i++) {
            sct_vector_push(&instr.body, sct_vector_get(&body_instrgen.instructions, i));
        }
    mc_instrgen_deinit(&body_instrgen);

    sct_vector_push(&instrgen->instructions, &(micro_instruction_t){
        .type = MICRO_INSTR_FUN,
        .hints = hints,
        .fun = instr,
    });
    return;

exit:
    while (instrgen->pos < instrgen->toks->size
        && ((mc_token_t*)sct_vector_get(instrgen->toks, instrgen->pos))->type != MC_TOK_KW_END) {
        instrgen->pos++;
    }
}