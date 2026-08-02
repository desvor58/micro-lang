#ifndef MICRO_CODEGEN386_EXPR_OPS_COMMON_H
#define MICRO_CODEGEN386_EXPR_OPS_COMMON_H

#include "errno.h"

#include "../../internal.h"

typedef void (*opMI_t)(micro_addr_le_t, micro_imm_le_t);
typedef void (*opSI_t)(micro_imm_le_t, micro_imm_le_t);
typedef void (*opRI_t)(micro_asm386_reg_t, micro_imm_le_t);

static opMI_t opMI_tbl[3];
static opSI_t opSI_fn;
static opRI_t opRI_tbl[3];

static void op_lit_to_dst(micro_codegen_t *codegen, micro_codegen386_storage_t dst, i32 lit)
{
    switch (dst.type) {
        case MICRO_STORAGE_DATASEC:
            opMI_tbl[dst.datasec.size](micro_imm_le_gen(dst.datasec.address), micro_imm_le_gen(lit));
            break;
            
        case MICRO_STORAGE_STACK:
            opSI_fn(micro_imm_le_gen(dst.stack.ebp_offset), micro_imm_le_gen(lit));
            break;

        case MICRO_STORAGE_REG:
            opRI_tbl[dst.reg.size](dst.reg.reg, micro_imm_le_gen(lit));
            break;
    }
}

typedef void (*opMR_t)(micro_addr_le_t, micro_asm386_reg_t);
typedef void (*opRR_t)(micro_asm386_reg_t, micro_asm386_reg_t);
typedef void (*opRS_t)(micro_asm386_reg_t, micro_imm_le_t);
typedef void (*opSR_t)(micro_imm_le_t, micro_asm386_reg_t);

static opMR_t opMR_tbl[3];
static opRR_t opRR_tbl[3];
static opRS_t opRS_tbl[3];
static opSR_t opSR_fn;

static int op_expr_to_dst(micro_codegen_t *codegen, micro_codegen386_storage_t dst, micro_token_t *expr)
{
    micro_codegen386_ext_t *ext = _micro_codegen386_ext(codegen);

    int free_space = get_last_free_space(codegen);

    micro_codegen386_storage_t expr_dst;
    if (free_space < 0) {
        expr_dst.type = MICRO_STORAGE_STACK;
        expr_dst.stack.ebp_offset = free_space;
    } else {
        expr_dst.type = MICRO_STORAGE_REG;
        expr_dst.reg.reg = free_space;
        expr_dst.reg.size = MICRO_SIZE_32;
    }

    int expr_offset = expr_parse(codegen, expr_dst, expr);
    if (!expr_offset) return 0;

    if (expr_dst.type == MICRO_STORAGE_REG) {
        switch (dst.type) {
            case MICRO_STORAGE_DATASEC: {
                opMR_tbl[dst.datasec.size](micro_imm_le_gen(dst.datasec.address), expr_dst.reg.reg);
            } break;

            case MICRO_STORAGE_STACK:
                opSR_fn(micro_imm_le_gen(dst.stack.ebp_offset), expr_dst.reg.reg);
                break;

            case MICRO_STORAGE_REG: {
                opRR_tbl[dst.reg.size](dst.reg.reg, expr_dst.reg.reg);
            } break;
        }
    } else
    if (expr_dst.type == MICRO_STORAGE_STACK) {
        switch (dst.type) {
            case MICRO_STORAGE_DATASEC: {
                if (ext->used_regs[0]) {
                    micro_asm386_movS32R32(micro_imm_le_gen(ext->ebp_top_offset), 0);
                }
                micro_asm386_movR32S32(MICRO_ASM386_REG32_EAX, micro_imm_le_gen(free_space));
                opMR_tbl[dst.datasec.size](micro_imm_le_gen(dst.datasec.address), MICRO_ASM386_REG32_EAX);
                if (ext->used_regs[0]) {
                    micro_asm386_movR32S32(0, micro_imm_le_gen(ext->ebp_top_offset));
                }
            } break;

            case MICRO_STORAGE_STACK:
                if (ext->used_regs[0]) {
                    micro_asm386_movS32R32(micro_imm_le_gen(ext->ebp_top_offset), 0);
                }
                opRS_tbl[MICRO_SIZE_32](MICRO_ASM386_REG32_EAX, micro_imm_le_gen(free_space));
                opSR_fn(micro_imm_le_gen(dst.stack.ebp_offset), MICRO_ASM386_REG32_EAX);
                if (ext->used_regs[0]) {
                    micro_asm386_movR32S32(0, micro_imm_le_gen(ext->ebp_top_offset));
                }
                break;

            case MICRO_STORAGE_REG: {
                opRS_tbl[dst.reg.size](dst.reg.reg, micro_imm_le_gen(free_space));
            } break;
        }
    }

    return expr_offset;
}

#endif