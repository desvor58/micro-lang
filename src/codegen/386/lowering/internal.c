#include "../internal.h"
#include "expr_ops.h"

int expr_lit_parse(micro_codegen_t *codegen, micro_codegen386_storage_t dst, i32 imm)
{
    typedef void (*datasec_mov_fn_t)(micro_addr_le_t, micro_imm_le_t);
    typedef void (*reg_mov_fn_t)(micro_asm386_reg_t, micro_imm_le_t);

    switch (dst.type) {
        case MICRO_STORAGE_DATASEC: {
            static const datasec_mov_fn_t tbl[] = {
                [MICRO_SIZE_8]  = micro_asm386_movM8I8,
                [MICRO_SIZE_16] = micro_asm386_movM16I16,
                [MICRO_SIZE_32] = micro_asm386_movM32I32,
            };
            tbl[dst.datasec.size](micro_imm_le_gen(dst.datasec.address), micro_imm_le_gen(imm));
            return 1;
        }
        
        case MICRO_STORAGE_STACK:
            micro_asm386_movS32I32(micro_imm_le_gen(dst.stack.ebp_offset), micro_imm_le_gen(imm));
            return 1;

        case MICRO_STORAGE_REG: {
            static const reg_mov_fn_t tbl[] = {
                [MICRO_SIZE_8]  = micro_asm386_movR8I8,
                [MICRO_SIZE_16] = micro_asm386_movR16I16,
                [MICRO_SIZE_32] = micro_asm386_movR32I32,
            };
            tbl[dst.reg.size](dst.reg.reg, micro_imm_le_gen(imm));
            return 1;
        }
    }
    return 0;
}

int expr_vreg_parse(micro_codegen_t *codegen, micro_codegen386_storage_t dst, micro_codegen386_ident_vreg_t vreg)
{
    micro_codegen386_ext_t *ext = _micro_codegen386_ext(codegen);

    if (unlikely(vreg.storage.type == MICRO_STORAGE_DATASEC)) {
        return 0;
    }

    int dst_size = 0;
    if (dst.type == MICRO_STORAGE_DATASEC) {
        dst_size = dst.datasec.size;
    } else if (dst.type == MICRO_STORAGE_REG) {
        dst_size = dst.reg.size;
    } else {
        dst_size = MICRO_SIZE_32;
    }

    typedef void (*r_s_fn_t)(micro_asm386_reg_t, micro_imm_le_t);
    typedef void (*m_r_fn_t)(micro_addr_le_t, micro_asm386_reg_t);
    typedef void (*s_r_fn_t)(micro_imm_le_t, micro_asm386_reg_t);
    typedef void (*r_r_fn_t)(micro_asm386_reg_t, micro_asm386_reg_t);

    static const r_s_fn_t mov_r_s_tbl[] = {
        [MICRO_SIZE_8]  = micro_asm386_movR8S32,
        [MICRO_SIZE_16] = micro_asm386_movR16S32,
        [MICRO_SIZE_32] = micro_asm386_movR32S32,
    };

    static const m_r_fn_t mov_m_r_tbl[] = {
        [MICRO_SIZE_8]  = micro_asm386_movM8R8,
        [MICRO_SIZE_16] = micro_asm386_movM16R16,
        [MICRO_SIZE_32] = micro_asm386_movM32R32,
    };

    static const s_r_fn_t mov_s_r_tbl[] = {
        [MICRO_SIZE_8]  = micro_asm386_movS32R8,
        [MICRO_SIZE_16] = micro_asm386_movS32R16,
        [MICRO_SIZE_32] = micro_asm386_movS32R32,
    };

    static const r_r_fn_t mov_r_r_tbl[] = {
        [MICRO_SIZE_8]  = micro_asm386_movR8R8,
        [MICRO_SIZE_16] = micro_asm386_movR16R16,
        [MICRO_SIZE_32] = micro_asm386_movR32R32,
    };

    switch (vreg.storage.type) {
        case MICRO_STORAGE_DATASEC:;
        case MICRO_STORAGE_STACK:
            switch (dst.type) {
                case MICRO_STORAGE_DATASEC:
                    if (ext->used_regs[0]) {
                        micro_asm386_movS32R32(micro_imm_le_gen(ext->ebp_top_offset), 0);
                        ext->max_stack_offset -= 4;
                    }
                    mov_r_s_tbl[dst_size](0, micro_imm_le_gen(vreg.storage.stack.ebp_offset));
                    mov_m_r_tbl[dst_size](micro_imm_le_gen(dst.datasec.address), 0);
                    if (ext->used_regs[0]) {
                        micro_asm386_movR32S32(0, micro_imm_le_gen(ext->ebp_top_offset));
                    }
                    return 1;

                case MICRO_STORAGE_STACK:
                    if (ext->used_regs[0]) {
                        micro_asm386_movS32R32(micro_imm_le_gen(ext->ebp_top_offset), 0);
                        ext->max_stack_offset -= 4;
                    }
                    mov_r_s_tbl[dst_size](0, micro_imm_le_gen(vreg.storage.stack.ebp_offset));
                    mov_s_r_tbl[dst_size](micro_imm_le_gen(dst.stack.ebp_offset), 0);
                    if (ext->used_regs[0]) {
                        micro_asm386_movR32S32(0, micro_imm_le_gen(ext->ebp_top_offset));
                    }
                    return 1;

                case MICRO_STORAGE_REG:
                    mov_r_s_tbl[dst_size](dst.reg.reg, micro_imm_le_gen(vreg.storage.stack.ebp_offset));
                    return 1;
            }
            return 0;

        case MICRO_STORAGE_REG:
            switch (dst.type) {
                case MICRO_STORAGE_DATASEC:
                    mov_m_r_tbl[dst_size](micro_imm_le_gen(dst.datasec.address), vreg.storage.reg.reg);
                    return 1;

                case MICRO_STORAGE_STACK:
                    mov_s_r_tbl[dst_size](micro_imm_le_gen(dst.stack.ebp_offset), vreg.storage.reg.reg);
                    return 1;

                case MICRO_STORAGE_REG:
                    mov_r_r_tbl[dst_size](dst.reg.reg, vreg.storage.reg.reg);
                    return 1;
            }
            return 0;
    }
    return 0;
}

int expr_parse(micro_codegen_t *codegen, micro_codegen386_storage_t dst, micro_token_t *start)
{
    micro_codegen386_ext_t *ext = _micro_codegen386_ext(codegen);

    if (dst.type == MICRO_STORAGE_REG) {
        ext->used_regs[dst.reg.reg] = 1;
    }

    if (_micro_tok_is_lit(start->type)) {
        char *end;
        errno = 0;
        i32 lit = strtol(start->val, &end, 10);
        return expr_lit_parse(codegen, dst, lit);
    }
    if (start->type == MICRO_TOK_IDENT) {
        micro_codegen386_ident_t *ident = sct_hashmap_get(&ext->idents, start->val);
        if (!ident) {
            micro_push_err((micro_error_t){
                .msg = "Undefined identifier",
                .line_ref = start->line_ref,
                .chpos_ref = start->chpos_ref
            });
            return 0;
        }

        switch (ident->type) {
            case MICRO_IDENT_FUN:
                return expr_lit_parse(codegen, dst, ident->fun.address);

            case MICRO_IDENT_VREG:
                return expr_vreg_parse(codegen, dst, ident->vreg);

            case MICRO_IDENT_LBL:
                return expr_lit_parse(codegen, dst, ident->lbl.address);
        };
    }
    if (_micro_tok_is_op(start->type)) {
        op_info_t op_info = op_tbl[start->type];
        return op_info.handler(codegen, dst, start);
    }
    return 1;
}

int get_last_free_space(micro_codegen_t *codegen)
{
    micro_codegen386_ext_t *ext = _micro_codegen386_ext(codegen);

    for (int i = 0; i < 8; i++) {
        if (!ext->used_regs[i]) {
            if (i > 2) {
                ext->use_callee_save_regs = 1;
            }
            return i;
        }
    }
    return ext->ebp_top_offset;
}
