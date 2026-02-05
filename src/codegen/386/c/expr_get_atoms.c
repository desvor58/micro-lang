#include "../expr_get_atoms.h"

int __micro_codegen_386_expr_parse_get_lit(size_t pos, micro_codegen_386_storage_info_t dst)
{
    u8 imm_val[4];

    if (dst.type == MICRO_ST_DATASEG) {
        u8 dst_addr[4];
        micro_gen32imm_le(dst_addr, dst.offset);

        if (dst.size == 4) {
            micro_gen32imm_le(imm_val, strtoll(micro_toks[pos].val, (char**)0, 10));
            u8 instruction[] = asm_movM32I32(dst_addr, imm_val);
            push_instruction(instruction);
        } else
        if (dst.size == 2) {
            micro_gen16imm_le(imm_val, strtoll(micro_toks[pos].val, (char**)0, 10));
            u8 instruction[] = asm_movM16I16(dst_addr, imm_val);
            push_instruction(instruction);
        } else
        if (dst.size == 1) {
            *imm_val = strtoll(micro_toks[pos].val, (char**)0, 10);
            u8 instruction[] = asm_movM8I8(dst_addr, imm_val);
            push_instruction(instruction);
        } else {
            goto err_wrong_dst_type_size;
        }
    } else
    if (dst.type == MICRO_ST_REG) {
        int dst_reg = dst.offset;
        if (dst.size == 4) {
            micro_gen32imm_le(imm_val, strtoll(micro_toks[pos].val, (char**)0, 10));
            u8 instruction[] = asm_movR32I32(dst_reg, imm_val);
            push_instruction(instruction);
        } else
        if (dst.size == 2) {
            micro_gen16imm_le(imm_val, strtoll(micro_toks[pos].val, (char**)0, 10));
            u8 instruction[] = asm_movR16I16(dst_reg, imm_val);
            push_instruction(instruction);
        } else
        if (dst.size == 1) {
            *imm_val = strtoll(micro_toks[pos].val, (char**)0, 10);
            u8 instruction[] = asm_movR8I8(dst_reg, imm_val);
            push_instruction(instruction);
        } else {
            goto err_wrong_dst_type_size;
        }
    } else
    if (dst.type == MICRO_ST_STACK) {
        u8 dst_addr[4];
        micro_gen32imm_le(dst_addr, dst.offset);
        if (dst.size == 4) {
            micro_gen32imm_le(imm_val, strtoll(micro_toks[pos].val, (char**)0, 10));
            u8 instruction[] = asm_movS32I32(dst_addr, imm_val);
            push_instruction(instruction);
        } else
        if (dst.size == 2) {
            micro_gen16imm_le(imm_val, strtoll(micro_toks[pos].val, (char**)0, 10));
            u8 instruction[] = asm_movS32I16(dst_addr, imm_val);
            push_instruction(instruction);
        } else
        if (dst.size == 1) {
            *imm_val = strtoll(micro_toks[pos].val, (char**)0, 10);
            u8 instruction[] = asm_movS32I8(dst_addr, imm_val);
            push_instruction(instruction);
        } else {
            goto err_wrong_dst_type_size;
        }
    }
    return 1;

err_wrong_dst_type_size:
    micro_error_t err = {.msg = "Wrong destination type size", .line_ref = micro_toks[pos].line_ref, .chpos_ref = micro_toks[pos].chpos_ref};
    __micro_push_err(err);
    return 0;
}

int __micro_codegen_386_expr_parse_get_ident(size_t pos, micro_codegen_386_storage_info_t dst, int expected_addr)
{
    micro_codegen_386_ident_info_t *ident_info = sct_hashmap_get(micro_codegen_386_idents, micro_toks[pos].val);
    if (!ident_info) {
        micro_error_t err = {.msg = "Unknown identifire",
                             .line_ref = micro_toks[pos].line_ref, .chpos_ref = micro_toks[pos].chpos_ref};
        __micro_push_err(err);
        return 0;
    }
    
    if (micro_mt_size[ident_info->var_info.type] != dst.size) {
        micro_error_t err = {.msg = "Variable has wrong type for this expression",
                             .line_ref = micro_toks[pos].line_ref,
                             .chpos_ref = micro_toks[pos].chpos_ref};
        __micro_push_err(err);
        return 0;
    }

    u8 ident_addr[4];
    micro_gen32imm_le(ident_addr, ident_info->var_info.storage_info.offset);

    if (expected_addr) {
        if (ident_info->var_info.storage_info.type == MICRO_ST_DATASEG) {
            int get_err = __micro_codegen_386_expr_parse_get_ident_addr_from_dataseg(ident_addr, &ident_info->var_info, dst);
            if (get_err) {
                goto err_wrong_dst_type_size;
            }
        } else
        if (ident_info->var_info.storage_info.type == MICRO_ST_STACK) {
            int get_err = __micro_codegen_386_expr_parse_get_ident_addr_from_stack(ident_addr, &ident_info->var_info, dst);
            if (get_err) {
                goto err_wrong_dst_type_size;
            }
        }
    } else {
        if (ident_info->var_info.storage_info.type == MICRO_ST_DATASEG) {
            int get_err = __micro_codegen_386_expr_parse_get_ident_from_dataseg(ident_addr, &ident_info->var_info, dst);
            if (get_err) {
                goto err_wrong_dst_type_size;
            }
        } else
        if (ident_info->var_info.storage_info.type == MICRO_ST_STACK) {
            int get_err = __micro_codegen_386_expr_parse_get_ident_from_stack(ident_addr, &ident_info->var_info, dst);
            if (get_err) {
                goto err_wrong_dst_type_size;
            }
        }
    }
    return 1;

err_wrong_dst_type_size:
    micro_error_t err = {.msg = "Wrong destination type size", .line_ref = micro_toks[pos].line_ref, .chpos_ref = micro_toks[pos].chpos_ref};
    __micro_push_err(err);
    return 0;
}