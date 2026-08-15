#ifndef MICRO_CODEGEN386_EXPR_OPS_COMMON_H
#define MICRO_CODEGEN386_EXPR_OPS_COMMON_H

#include "errno.h"

#include "../../internal.h"

typedef struct {
    micro_asm386_instruction_type_t opMI_tbl[3];
    micro_asm386_instruction_type_t opSI_fn;
    micro_asm386_instruction_type_t opRI_tbl[3];
    micro_asm386_instruction_type_t opMR_tbl[3];
    micro_asm386_instruction_type_t opRR_tbl[3];
    micro_asm386_instruction_type_t opRS_tbl[3];
    micro_asm386_instruction_type_t opSR_fn;
} op_tbls_t;

static void op_lit_to_dst(micro_codegen_t *codegen, const op_tbls_t *op_tbls, micro_codegen386_storage_t dst, i32 lit)
{
    switch (dst.type) {
        case MICRO_STORAGE_DATASEC:
            push_asm_instr(op_tbls->opMI_tbl[dst.datasec.size], { .addr = micro_imm_le_gen(dst.datasec.address) }, { .imm = micro_imm_le_gen(lit) });
            return;
            
        case MICRO_STORAGE_STACK:
            push_asm_instr(op_tbls->opSI_fn, { .imm = micro_imm_le_gen(dst.stack.ebp_offset) }, { .imm = micro_imm_le_gen(lit) });
            return;

        case MICRO_STORAGE_REG:
            push_asm_instr(op_tbls->opRI_tbl[dst.reg.size], { .reg = dst.reg.reg }, { .imm = micro_imm_le_gen(lit) });
            return;
    }
}

static int op_expr_to_dst(micro_codegen_t *codegen, const op_tbls_t *op_tbls, micro_codegen386_storage_t dst, micro_token_t *expr)
{
    micro_codegen386_ext_t *ext = _micro_codegen386_ext(codegen);

    int free_space = get_last_free_space(codegen);

    micro_codegen386_storage_t expr_dst;
    if (free_space < 0) {
        expr_dst.type = MICRO_STORAGE_STACK;
        expr_dst.stack.ebp_offset = free_space;
        ext->max_stack_offset -= 4;
    } else {
        expr_dst.type = MICRO_STORAGE_REG;
        expr_dst.reg.reg = free_space;
        expr_dst.reg.size = MICRO_SIZE_32;
    }

    int expr_offset = expr_parse(codegen, expr_dst, expr);
    if (!expr_offset) return 0;

    if (expr_dst.type == MICRO_STORAGE_REG) {
        switch (dst.type) {
            case MICRO_STORAGE_DATASEC:
                push_asm_instr(op_tbls->opMR_tbl[dst.datasec.size], { .addr = micro_imm_le_gen(dst.datasec.address) }, { .reg = expr_dst.reg.reg });
                break;

            case MICRO_STORAGE_STACK:
                push_asm_instr(op_tbls->opSR_fn, { .imm = micro_imm_le_gen(dst.stack.ebp_offset) }, { .reg = expr_dst.reg.reg });
                break;

            case MICRO_STORAGE_REG:
                push_asm_instr(op_tbls->opRR_tbl[dst.reg.size], { .reg = dst.reg.reg }, { .reg = expr_dst.reg.reg });
                break;
        }
    } else
    if (expr_dst.type == MICRO_STORAGE_STACK) {
        switch (dst.type) {
            case MICRO_STORAGE_DATASEC:
                if (ext->used_regs[0]) {
                    push_asm_instr(MICRO_ASM386_INSTR_MOV_S32R32, { .imm = micro_imm_le_gen(ext->ebp_top_offset) }, { .reg = MICRO_ASM386_REG32_EAX });
                    ext->max_stack_offset -= 4;
                }
                push_asm_instr(MICRO_ASM386_INSTR_MOV_R32S32, { .reg = MICRO_ASM386_REG32_EAX }, { .imm = micro_imm_le_gen(free_space) });
                push_asm_instr(op_tbls->opMR_tbl[dst.datasec.size], { .addr = micro_imm_le_gen(dst.datasec.address) }, { .reg = MICRO_ASM386_REG32_EAX });
                if (ext->used_regs[0]) {
                    push_asm_instr(MICRO_ASM386_INSTR_MOV_R32S32, { .reg = MICRO_ASM386_REG32_EAX }, { .imm = micro_imm_le_gen(ext->ebp_top_offset) });
                }
                break;

            case MICRO_STORAGE_STACK:
                if (ext->used_regs[0]) {
                    push_asm_instr(MICRO_ASM386_INSTR_MOV_S32R32, { .imm = micro_imm_le_gen(ext->ebp_top_offset) }, { .reg = MICRO_ASM386_REG32_EAX });
                    ext->max_stack_offset -= 4;
                }
                push_asm_instr(op_tbls->opRS_tbl[MICRO_SIZE_32], { .reg = MICRO_ASM386_REG32_EAX }, { .imm = micro_imm_le_gen(free_space) });
                push_asm_instr(op_tbls->opSR_fn, { .imm = micro_imm_le_gen(dst.stack.ebp_offset) }, { .reg = MICRO_ASM386_REG32_EAX });
                if (ext->used_regs[0]) {
                    push_asm_instr(MICRO_ASM386_INSTR_MOV_R32S32, { .reg = MICRO_ASM386_REG32_EAX }, { .imm = micro_imm_le_gen(ext->ebp_top_offset) });
                }
                break;

            case MICRO_STORAGE_REG:
                push_asm_instr(op_tbls->opRS_tbl[dst.reg.size], { .reg = dst.reg.reg }, { .imm = micro_imm_le_gen(free_space) });
                break;
        }
    }
    return expr_offset;
}

static void op_vreg_to_dst(micro_codegen_t *codegen, const op_tbls_t *op_tbls, micro_codegen386_storage_t dst, micro_codegen386_ident_vreg_t vreg)
{
    micro_codegen386_ext_t *ext = _micro_codegen386_ext(codegen);

    if (unlikely(vreg.storage.type == MICRO_STORAGE_DATASEC)) {
        micro_push_err((micro_error_t){
            .msg = "err",
            .line_ref = 0,
            .chpos_ref = 0
        });
        return;
    }

    switch (vreg.storage.type) {
        case MICRO_STORAGE_STACK:
            switch (dst.type) {
                case MICRO_STORAGE_DATASEC:
                    if (ext->used_regs[0]) {
                        push_asm_instr(MICRO_ASM386_INSTR_MOV_S32R32, { .imm = micro_imm_le_gen(ext->ebp_top_offset) }, { .reg = MICRO_ASM386_REG32_EAX });
                        ext->max_stack_offset -= 4;
                    }
                    push_asm_instr(movRS_tbl[micro_type_to_size[vreg.type]], { .reg = 0 }, { .imm = micro_imm_le_gen(vreg.storage.stack.ebp_offset) });
                    push_asm_instr(op_tbls->opMR_tbl[dst.datasec.size], { .addr = micro_imm_le_gen(dst.datasec.address) }, { .reg = 0 });
                    if (ext->used_regs[0]) {
                        push_asm_instr(MICRO_ASM386_INSTR_MOV_R32S32, { .reg = MICRO_ASM386_REG32_EAX }, { .imm = micro_imm_le_gen(ext->ebp_top_offset) });
                    }
                    break;

                case MICRO_STORAGE_STACK:
                    if (ext->used_regs[0]) {
                        push_asm_instr(MICRO_ASM386_INSTR_MOV_S32R32, { .imm = micro_imm_le_gen(ext->ebp_top_offset) }, { .reg = MICRO_ASM386_REG32_EAX });
                        ext->max_stack_offset -= 4;
                    }
                    push_asm_instr(movRS_tbl[micro_type_to_size[vreg.type]], { .reg = 0 }, { .imm = micro_imm_le_gen(vreg.storage.stack.ebp_offset) });
                    push_asm_instr(op_tbls->opSR_fn, { .addr = micro_imm_le_gen(dst.stack.ebp_offset) }, { .reg = 0 });
                    if (ext->used_regs[0]) {
                        push_asm_instr(MICRO_ASM386_INSTR_MOV_R32S32, { .reg = MICRO_ASM386_REG32_EAX }, { .imm = micro_imm_le_gen(ext->ebp_top_offset) });
                    }
                    break;

                case MICRO_STORAGE_REG:
                    push_asm_instr(op_tbls->opRS_tbl[dst.reg.size], { .reg = dst.reg.reg }, { .imm = micro_imm_le_gen(vreg.storage.stack.ebp_offset) });
                    break;
            };
            break;

        case MICRO_STORAGE_REG:
            switch (dst.type) {
                case MICRO_STORAGE_DATASEC:
                    push_asm_instr(op_tbls->opMR_tbl[dst.datasec.size], { .addr = micro_imm_le_gen(dst.datasec.address) }, { .reg = vreg.storage.reg.reg });
                    break;

                case MICRO_STORAGE_STACK:
                    push_asm_instr(op_tbls->opSR_fn, { .addr = micro_imm_le_gen(dst.stack.ebp_offset) }, { .reg = vreg.storage.reg.reg });
                    break;

                case MICRO_STORAGE_REG:
                    push_asm_instr(op_tbls->opRR_tbl[dst.reg.size], { .reg = dst.reg.reg }, { .reg = vreg.storage.reg.reg });
                    break;
            };
            break;
    };
}

#endif