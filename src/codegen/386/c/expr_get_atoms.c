#include "../expr_get_atoms.h"

int __micro_codegen_386_expr_parse_get_lit(micro_codegen_t *codegen, size_t pos, micro_codegen_386_storage_info_t dst)
{
    micro_imm_le_t imm_val = micro_imm_le_gen(strtoll(codegen->toks->toks[pos].val, (char**)0, 10));

    if (dst.type == MICRO_ST_DATASEG) {
        micro_imm_le_t dst_addr = micro_imm_le_gen(dst.offset);
        
        void (*instr_tbl[])(micro_addr_le_t, micro_imm_le_t) = {
            [MICRO_SZ_8]  = asm386_movM8I8,
            [MICRO_SZ_16] = asm386_movM16I16,
            [MICRO_SZ_32] = asm386_movM32I32,
        };
        instr_tbl[dst.size](dst_addr, imm_val);
    } else
    if (dst.type == MICRO_ST_REG) {
        asm386_reg dst_reg = dst.offset;
        
        void (*instr_tbl[])(asm386_reg, micro_imm_le_t) = {
            [MICRO_SZ_8]  = asm386_movR8I8,
            [MICRO_SZ_16] = asm386_movR16I16,
            [MICRO_SZ_32] = asm386_movR32I32,
        };
        instr_tbl[dst.size](dst_reg, imm_val);
    } else
    if (dst.type == MICRO_ST_STACK) {
        micro_imm_le_t dst_addr = micro_imm_le_gen(dst.offset);

        void (*instr_tbl[])(micro_addr_le_t, micro_imm_le_t) = {
            [MICRO_SZ_8]  = asm386_movS32I8,
            [MICRO_SZ_16] = asm386_movS32I16,
            [MICRO_SZ_32] = asm386_movS32I32,
        };
        instr_tbl[dst.size](dst_addr, imm_val);
    }
    asm_put_instructions(codegen);
    return 1;
}

int __micro_codegen_386_expr_parse_get_ident(micro_codegen_t *codegen, size_t pos, micro_codegen_386_storage_info_t dst, int expected_addr)
{
    micro_codegen_386_ident_info_t *ident_info = sct_hashmap_get(get_codegen_386_ext(codegen)->idents, codegen->toks->toks[pos].val);
    if (!ident_info) {
        micro_push_err((micro_error_t){
            .msg = "Unknown identfire",
            .line_ref = codegen->toks->toks[pos].line_ref,
            .chpos_ref = codegen->toks->toks[pos].chpos_ref
        });
        return 0;
    }
    
    if (micro_mt_size[ident_info->var_info.type] != dst.size) {
        micro_push_err((micro_error_t){
            .msg = "Variable has wrong type for this expression",
            .line_ref = codegen->toks->toks[pos].line_ref,
            .chpos_ref = codegen->toks->toks[pos].chpos_ref
        });
        return 0;
    }

    micro_addr_le_t ident_addr = micro_imm_le_gen(ident_info->var_info.storage_info.offset);

    if (expected_addr) {
        if (ident_info->var_info.storage_info.type == MICRO_ST_DATASEG) {
            int get_err = __micro_codegen_386_expr_parse_get_ident_addr_from_dataseg(codegen, ident_addr, &ident_info->var_info, dst);
            if (get_err) {
                goto err_wrong_dst_type_size;
            }
        } else
        if (ident_info->var_info.storage_info.type == MICRO_ST_STACK) {
            int get_err = __micro_codegen_386_expr_parse_get_ident_addr_from_stack(codegen, ident_addr, &ident_info->var_info, dst);
            if (get_err) {
                goto err_wrong_dst_type_size;
            }
        }
    } else {
        if (ident_info->var_info.storage_info.type == MICRO_ST_DATASEG) {
            int get_err = __micro_codegen_386_expr_parse_get_ident_from_dataseg(codegen, ident_addr, &ident_info->var_info, dst);
            if (get_err) {
                goto err_wrong_dst_type_size;
            }
        } else
        if (ident_info->var_info.storage_info.type == MICRO_ST_STACK) {
            int get_err = __micro_codegen_386_expr_parse_get_ident_from_stack(codegen, ident_addr, &ident_info->var_info, dst);
            if (get_err) {
                goto err_wrong_dst_type_size;
            }
        }
    }
    return 1;

err_wrong_dst_type_size:
    micro_push_err((micro_error_t){
        .msg = "Wrong destination type size",
        .line_ref = codegen->toks->toks[pos].line_ref,
        .chpos_ref = codegen->toks->toks[pos].chpos_ref
    });
    return 0;
}