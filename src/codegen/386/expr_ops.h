#ifndef MICRO_CODEGEN386_EXPR_OPS_H
#define MICRO_CODEGEN386_EXPR_OPS_H

#include "internal.h"

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

int op_plus_handler(micro_codegen_t *codegen, micro_codegen386_storage_t dst, micro_token_t *start)
{
    micro_codegen386_ext_t *ext = _micro_codegen386_ext(codegen);

    if (unlikely(start->type != MICRO_TOK_PLUS)) {
        return 0;
    }

    opMI_tbl[MICRO_SIZE_8]  = micro_asm386_addM8I8;
    opMI_tbl[MICRO_SIZE_16] = micro_asm386_addM16I16;
    opMI_tbl[MICRO_SIZE_32] = micro_asm386_addM32I32;
    
    opSI_fn = micro_asm386_addS32I32;

    opRI_tbl[MICRO_SIZE_8]  = micro_asm386_addR8I8;
    opRI_tbl[MICRO_SIZE_16] = micro_asm386_addR16I16;
    opRI_tbl[MICRO_SIZE_32] = micro_asm386_addR32I32;

    micro_token_t *first_operand = start + 1;
    if (_micro_tok_is_lit(first_operand->type)) {
        char *end;
        errno = 0;
        i32 first_lit = strtol(first_operand->val, &end, 10);

        micro_token_t *second_operand = start + 2;
        if (_micro_tok_is_lit(second_operand->type)) {
            errno = 0;
            i32 second_lit = strtol(second_operand->val, &end, 10);

            expr_lit_parse(codegen, dst, first_lit);

            op_lit_to_dst(codegen, dst, second_lit);
            return 3;
        }
        if (_micro_tok_is_op(second_operand->type)) {
            int expr_offset = expr_parse(codegen, dst, second_operand);
            if (!expr_offset) return 0;
            op_lit_to_dst(codegen, dst, first_lit);
            return 2 + expr_offset;
        }
        return 0;
    }
    if (_micro_tok_is_op(first_operand->type)) {
        int expr_offset = expr_parse(codegen, dst, first_operand);
        if (!expr_offset) return 0;

        if (dst.type == MICRO_STORAGE_REG) {
            ext->used_regs[dst.reg.reg] = 1;
        }

        micro_token_t *second_operand = start + expr_offset + 1;
        if (_micro_tok_is_lit(second_operand->type)) {
            char *end;
            errno = 0;
            i32 second_lit = strtol(second_operand->val, &end, 10);

            op_lit_to_dst(codegen, dst, second_lit);
            return 2 + expr_offset;
        }
        if (_micro_tok_is_op(second_operand->type)) {
            opMR_tbl[MICRO_SIZE_8]  = micro_asm386_addM8R8;
            opMR_tbl[MICRO_SIZE_16] = micro_asm386_addM16R16;
            opMR_tbl[MICRO_SIZE_32] = micro_asm386_addM32R32;

            opRR_tbl[MICRO_SIZE_8]  = micro_asm386_addR8R8;
            opRR_tbl[MICRO_SIZE_16] = micro_asm386_addR16R16;
            opRR_tbl[MICRO_SIZE_32] = micro_asm386_addR32R32;

            opRS_tbl[MICRO_SIZE_8]  = micro_asm386_addR8S32;
            opRS_tbl[MICRO_SIZE_16] = micro_asm386_addR16S32;
            opRS_tbl[MICRO_SIZE_32] = micro_asm386_addR32S32;

            opSR_fn = micro_asm386_addS32R32;

            int expr2_offset = op_expr_to_dst(codegen, dst, second_operand);
            if (!expr2_offset) return 0;
            return 1 + expr_offset + expr2_offset;
        }
    }
    return 0;
}

// int op_minus_handler(micro_codegen_t *codegen, micro_codegen386_storage_t dst, micro_token_t *start)
// {
//     micro_codegen386_ext_t *ext = _micro_codegen386_ext(codegen);

//     if (unlikely(start->type != MICRO_TOK_MINUS)) {
//         return 0;
//     }

//     opMI_tbl[MICRO_SIZE_8]  = micro_asm386_subM8I8;
//     opMI_tbl[MICRO_SIZE_16] = micro_asm386_addM16I16;
//     opMI_tbl[MICRO_SIZE_32] = micro_asm386_addM32I32;
    
//     opSI_fn = micro_asm386_addS32I32;

//     opRI_tbl[MICRO_SIZE_8]  = micro_asm386_addR8I8;
//     opRI_tbl[MICRO_SIZE_16] = micro_asm386_addR16I16;
//     opRI_tbl[MICRO_SIZE_32] = micro_asm386_addR32I32;

//     micro_token_t *first_operand = start + 1;
//     if (_micro_tok_is_lit(first_operand->type)) {
//         char *end;
//         errno = 0;
//         i32 first_lit = strtol(first_operand->val, &end, 10);

//         micro_token_t *second_operand = start + 2;
//         if (_micro_tok_is_lit(second_operand->type)) {
//             errno = 0;
//             i32 second_lit = strtol(second_operand->val, &end, 10);

//             expr_lit_parse(codegen, dst, first_lit);

//             op_lit_to_dst(codegen, dst, second_lit);
//             return 3;
//         }
//         if (_micro_tok_is_op(second_operand->type)) {
//             int expr_offset = expr_parse(codegen, dst, second_operand);
//             if (!expr_offset) return 0;
//             op_lit_to_dst(codegen, dst, first_lit);
//             return 2 + expr_offset;
//         }
//         return 0;
//     }
//     if (_micro_tok_is_op(first_operand->type)) {
//         int expr_offset = expr_parse(codegen, dst, first_operand);
//         if (!expr_offset) return 0;

//         if (dst.type == MICRO_STORAGE_REG) {
//             ext->used_regs[dst.reg.reg] = 1;
//         }

//         micro_token_t *second_operand = start + expr_offset + 1;
//         if (_micro_tok_is_lit(second_operand->type)) {
//             char *end;
//             errno = 0;
//             i32 second_lit = strtol(second_operand->val, &end, 10);

//             op_lit_to_dst(codegen, dst, second_lit);
//             return 2 + expr_offset;
//         }
//         if (_micro_tok_is_op(second_operand->type)) {
//             opMR_tbl[MICRO_SIZE_8]  = micro_asm386_addM8R8;
//             opMR_tbl[MICRO_SIZE_16] = micro_asm386_addM16R16;
//             opMR_tbl[MICRO_SIZE_32] = micro_asm386_addM32R32;

//             opRR_tbl[MICRO_SIZE_8]  = micro_asm386_addR8R8;
//             opRR_tbl[MICRO_SIZE_16] = micro_asm386_addR16R16;
//             opRR_tbl[MICRO_SIZE_32] = micro_asm386_addR32R32;

//             opRS_tbl[MICRO_SIZE_8]  = micro_asm386_addR8S32;
//             opRS_tbl[MICRO_SIZE_16] = micro_asm386_addR16S32;
//             opRS_tbl[MICRO_SIZE_32] = micro_asm386_addR32S32;

//             opSR_fn = micro_asm386_addS32R32;

//             int expr2_offset = op_expr_to_dst(codegen, dst, second_operand);
//             if (!expr2_offset) return 0;
//             return 1 + expr_offset + expr2_offset;
//         }
//     }
//     return 0;
// }

typedef struct {
    int (*handler)(micro_codegen_t *codegen, micro_codegen386_storage_t dst, micro_token_t *start);
} op_info_t;

op_info_t op_tbl[] = {
    [MICRO_TOK_PLUS] = { .handler = op_plus_handler },
};

#endif