#include "../common.h"

sct_hashmap_t *micro_codegen_386_idents;

sct_list_pair_t *micro_codegen_386_local_vars_list;

micro_error_t *micro_codegen_386_err_stk;
size_t         micro_codegen_386_err_stk_size;
size_t       __micro_codegen_386_err_stk_real_size;

int      micro_code_in_function;
offset_t micro_top_stack_offset;

sct_list_pair_t *micro_goto_refs;

sct_string_t *micro_outbuf;
size_t micro_pos = 0;

size_t micro_mt_size[] = {
    [MICRO_MT_NULL] = 0,
    [MICRO_MT_I8]   = 1,
    [MICRO_MT_U8]   = 1,
    [MICRO_MT_I16]  = 2,
    [MICRO_MT_U16]  = 2,
    [MICRO_MT_I32]  = 4,
    [MICRO_MT_U32]  = 4,
    [MICRO_MT_F32]  = 4,
    [MICRO_MT_PTR]  = 4
};

micro_codegen_386_micro_type micro_str2mt(char *str)
{
    if (!strcmp(str, "i8"))  return MICRO_MT_I8;
    if (!strcmp(str, "u8"))  return MICRO_MT_U8;
    if (!strcmp(str, "i16")) return MICRO_MT_I16;
    if (!strcmp(str, "u16")) return MICRO_MT_U16;
    if (!strcmp(str, "i32")) return MICRO_MT_I32;
    if (!strcmp(str, "u32")) return MICRO_MT_U32;
    if (!strcmp(str, "f32")) return MICRO_MT_F32;
    if (!strcmp(str, "ptr")) return MICRO_MT_PTR;
    return MICRO_MT_NULL;
}

micro_codegen_386_micro_type micro_lit2mt(micro_token_t lit, micro_codegen_386_micro_type expected)
{
    if (!micro_tokislit(lit)) {
        return MICRO_MT_NULL;
    }
    if (lit.type == MICRO_TT_LIT_FLOAT) {
        if (expected != MICRO_MT_F32) return MICRO_MT_NULL;
        return MICRO_MT_F32;
    }
    if (lit.type == MICRO_TT_LIT_INT) {
        if (expected != MICRO_MT_I8
         && expected != MICRO_MT_U8
         && expected != MICRO_MT_I16
         && expected != MICRO_MT_U16
         && expected != MICRO_MT_I32
         && expected != MICRO_MT_U32
        ) {
            return MICRO_MT_NULL;
        }
        return expected;
    }
    if (lit.type == MICRO_TT_LIT_STR) {
        if (expected != MICRO_MT_PTR) return MICRO_MT_NULL;
        return MICRO_MT_PTR;
    }
    return MICRO_MT_NULL;
}

void micro_imm_from_mt(u8 *buf, micro_codegen_386_micro_type type, size_t val)
{
    if (micro_mt_size[type] == 1) {
        buf[0] = val;
    } else
    if (micro_mt_size[type] == 2) {
        micro_gen16imm_le(buf, val);
    } else
    if (micro_mt_size[type] == 4) {
        micro_gen32imm_le(buf, val);
    }
}

void micro_codegen_386_init()
{
    micro_codegen_386_err_stk = (micro_error_t*)malloc(sizeof(micro_error_t) * MICRO_ERROR_STACK_EXTEND_SIZE);
    micro_codegen_386_err_stk_size = 0;
    __micro_codegen_386_err_stk_real_size = MICRO_ERROR_STACK_EXTEND_SIZE;

    micro_code_in_function = 0;
    micro_top_stack_offset = 0;

    micro_codegen_386_idents = sct_hashmap_create();
    micro_codegen_386_local_vars_list = sct_list_pair_create(0);

    micro_goto_refs = sct_list_pair_create(0);

    micro_outbuf = sct_string_create();
}

void micro_codegen_386_delete()
{
    free(micro_codegen_386_err_stk);
    sct_hashmap_full_free(micro_codegen_386_idents);
    sct_list_full_free(micro_codegen_386_local_vars_list);
    sct_list_full_free(micro_goto_refs);
    sct_string_free(micro_outbuf);
}

micro_token_t __micro_peek(size_t offset)
{
    if (micro_pos + offset >= micro_toks_size) {
        return (micro_token_t){.type = MICRO_TT_NULL, .line_ref = micro_toks[micro_pos].line_ref, .chpos_ref = micro_toks[micro_pos].chpos_ref};
    }
    return micro_toks[micro_pos + offset];
}

micro_token_t __micro_get(size_t offset)
{
    if (micro_pos + offset >= micro_toks_size) {
        return (micro_token_t){.type = MICRO_TT_NULL, .line_ref = micro_toks[micro_pos].line_ref, .chpos_ref = micro_toks[micro_pos].chpos_ref};
    }
    micro_pos += offset;
    return micro_toks[micro_pos];
}

void __micro_codegen_386_err_stk_size_check(size_t offset)
{
    if (micro_codegen_386_err_stk_size + offset >= __micro_codegen_386_err_stk_real_size) {
        micro_error_t *new_stk = (micro_error_t*)malloc(sizeof(micro_error_t) * (__micro_codegen_386_err_stk_real_size += offset / MICRO_ERROR_STACK_EXTEND_SIZE + 1));
        memcpy(new_stk, micro_codegen_386_err_stk, sizeof(micro_error_t) * micro_codegen_386_err_stk_size);
        free(micro_codegen_386_err_stk);
        micro_codegen_386_err_stk = new_stk;
    }
}

micro_codegen_386_micro_type micro_gettype(micro_token_t tok, micro_codegen_386_micro_type expected)
{
    if (micro_tokislit(tok)) {
        return micro_lit2mt(tok, expected);
    } else
    if (tok.type == MICRO_TT_IDENT) {
        //__micro_dbg_print_vars();
        micro_codegen_386_ident_info_t *ident_info = sct_hashmap_get(micro_codegen_386_idents, tok.val);
        if (!ident_info) {
            micro_error_t err = {.msg = "Undeclarated variable",
                                 .line_ref = tok.line_ref,
                                 .chpos_ref = tok.chpos_ref};
            __micro_push_err(err);
            return MICRO_MT_NULL;
        }
        if (ident_info->type == IT_VAR) {
            if (ident_info->var_info.type == expected) {
                return expected;
            }
        } else {
            if (expected == MICRO_MT_PTR) {
                return expected;
            }
        }
    } else
    if (micro_tokisop(tok)) {
        return expected;
    }
    return MICRO_MT_NULL;
}

void __micro_dbg_print_idents()
{
    foreach (key_it, micro_codegen_386_idents->keys) {
        // printf("putting:%s\n", (char*)key_it->val);
        micro_codegen_386_ident_info_t *ident_info = sct_hashmap_get(micro_codegen_386_idents, (char*)key_it->val);
        if (!ident_info) {
            puts("EE");
            exit(1);
        }
        if (ident_info->type == IT_VAR) {
            micro_codegen_386_var_info_t var_info = ident_info->var_info;
            printf("%s:{\n  type:%u,\n  storage_info:{\n    type:%u,\n    size:%u,\n    offset:%d,\n    isunssigned:%d\n  }\n}\n",
                   var_info.name,
                   var_info.type,
                   var_info.storage_info.type,
                   var_info.storage_info.size,
                   var_info.storage_info.offset,
                   var_info.storage_info.is_unsigned);
        }
        if (ident_info->type == IT_FUN) {
            micro_codegen_386_fun_info_t fun_info = ident_info->fun_info;
            printf("%s:{\n  ret_type:%u,\n  offset:%u,\n  agrs:{\n",
                   fun_info.name,
                   fun_info.ret_type,
                   fun_info.offset);
            for (size_t i = 0; i < fun_info.args->size; i++) {
                micro_codegen_386_var_info_t *var_info = fun_info.args->arr[i];
                printf("    %s:{\n      type:%u,\n      storage_info:{\n        type:%u,\n        size:%u,\n        offset:%d,\n        isunssigned:%d\n      }\n}\n",
                       var_info->name,
                       var_info->type,
                       var_info->storage_info.type,
                       var_info->storage_info.size,
                       var_info->storage_info.offset,
                       var_info->storage_info.is_unsigned);
            }
            printf("}\n");
        }
        if (ident_info->type == IT_LBL) {
            micro_codegen_386_lbl_info_t lbl_info = ident_info->lbl_info;
            printf("%s:{\n  offset:%d\n}\n",
                   lbl_info.name,
                   lbl_info.offset);
        }
    }
}