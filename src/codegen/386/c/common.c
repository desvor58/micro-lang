#include "../common.h"

micro_codegen_386_size micro_mt_size[] = {
    [MICRO_MT_NULL] = MICRO_SZ_8,
    [MICRO_MT_I8]   = MICRO_SZ_8,
    [MICRO_MT_U8]   = MICRO_SZ_8,
    [MICRO_MT_I16]  = MICRO_SZ_16,
    [MICRO_MT_U16]  = MICRO_SZ_16,
    [MICRO_MT_I32]  = MICRO_SZ_32,
    [MICRO_MT_U32]  = MICRO_SZ_32,
    [MICRO_MT_F32]  = MICRO_SZ_32,
    [MICRO_MT_PTR]  = MICRO_SZ_32
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

micro_token_t __micro_peek(micro_codegen_t *codegen, size_t offset)
{
    if (codegen->toks_pos + offset >= codegen->toks->size) {
        return (micro_token_t){
            .type = MICRO_TT_NULL,
            .line_ref = codegen->toks->toks[codegen->toks_pos].line_ref,
            .chpos_ref = codegen->toks->toks[codegen->toks_pos].chpos_ref
        };
    }
    return codegen->toks->toks[codegen->toks_pos + offset];
}

micro_token_t __micro_get(micro_codegen_t *codegen, size_t offset)
{
    if (codegen->toks_pos + offset >= codegen->toks->size) {
        return (micro_token_t){
            .type = MICRO_TT_NULL,
            .line_ref = codegen->toks->toks[codegen->toks_pos].line_ref,
            .chpos_ref = codegen->toks->toks[codegen->toks_pos].chpos_ref
        };
    }
    codegen->toks_pos += offset;
    return codegen->toks->toks[codegen->toks_pos];
}

micro_codegen_386_micro_type micro_gettype(micro_codegen_t *codegen, micro_token_t tok, micro_codegen_386_micro_type expected)
{
    if (micro_tokislit(tok)) {
        return micro_lit2mt(tok, expected);
    } else
    if (tok.type == MICRO_TT_IDENT) {
        //__micro_dbg_print_vars();
        micro_codegen_386_ident_info_t *ident_info = sct_hashmap_get(get_codegen_386_ext(codegen)->idents, tok.val);
        if (!ident_info) {
            micro_push_err((micro_error_t){
                .msg = "Undeclareted variable",
                .line_ref = tok.line_ref,
                .chpos_ref = tok.chpos_ref
            });
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

void __micro_dbg_print_idents(micro_codegen_t *codegen)
{
    foreach (key_it, get_codegen_386_ext(codegen)->idents->keys) {
        // printf("putting:%s\n", (char*)key_it->val);
        micro_codegen_386_ident_info_t *ident_info = sct_hashmap_get(get_codegen_386_ext(codegen)->idents, (char*)key_it->val);
        if (!ident_info) {
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