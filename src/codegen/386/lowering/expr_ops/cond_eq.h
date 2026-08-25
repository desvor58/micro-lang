#ifndef MICRO_CODEGEN386_EXPR_OPS_COND_EQ_H
#define MICRO_CODEGEN386_EXPR_OPS_COND_EQ_H

#include "common.h"

static const op_tbls_t cond_eq_op_tbls = {
    .opMI_tbl[MICRO_SIZE_8]  = MICRO_ASM386_INSTR_CMP_M8I8,
    .opMI_tbl[MICRO_SIZE_16] = MICRO_ASM386_INSTR_CMP_M16I16,
    .opMI_tbl[MICRO_SIZE_32] = MICRO_ASM386_INSTR_CMP_M32I32,
    
    .opSI_fn = MICRO_ASM386_INSTR_CMP_S32I32,

    .opRI_tbl[MICRO_SIZE_8]  = MICRO_ASM386_INSTR_CMP_R8I8,
    .opRI_tbl[MICRO_SIZE_16] = MICRO_ASM386_INSTR_CMP_R16I16,
    .opRI_tbl[MICRO_SIZE_32] = MICRO_ASM386_INSTR_CMP_R32I32,
    
    .opMR_tbl[MICRO_SIZE_8]  = MICRO_ASM386_INSTR_SUB_M8R8,
    .opMR_tbl[MICRO_SIZE_16] = MICRO_ASM386_INSTR_SUB_M16R16,
    .opMR_tbl[MICRO_SIZE_32] = MICRO_ASM386_INSTR_SUB_M32R32,

    .opRR_tbl[MICRO_SIZE_8]  = MICRO_ASM386_INSTR_CMP_R8R8,
    .opRR_tbl[MICRO_SIZE_16] = MICRO_ASM386_INSTR_CMP_R16R16,
    .opRR_tbl[MICRO_SIZE_32] = MICRO_ASM386_INSTR_CMP_R32R32,

    .opRS_tbl[MICRO_SIZE_8]  = MICRO_ASM386_INSTR_SUB_R8S32,
    .opRS_tbl[MICRO_SIZE_16] = MICRO_ASM386_INSTR_SUB_R16S32,
    .opRS_tbl[MICRO_SIZE_32] = MICRO_ASM386_INSTR_SUB_R32S32,

    .opSR_fn = MICRO_ASM386_INSTR_SUB_S32R32,
};

int cond_op_eq_handler(micro_codegen_t *codegen, micro_codegen386_storage_t dst, micro_token_t *start)
{
    micro_codegen386_ext_t *ext = _micro_codegen386_ext(codegen);

    if (unlikely(start->type != MICRO_TOK_EQ)) {
        return 0;
    }

    micro_token_t *first_operand = start + 1;
    if (first_operand->type == MICRO_TOK_IDENT) {
        micro_codegen386_ident_t *ident = sct_hashmap_get(&ext->idents, first_operand->val);

        micro_token_t *second_operand = start + 2;
        if (ident->type == MICRO_IDENT_VREG) {
            if (_micro_tok_is_lit(second_operand->type)) {
                char *end;
                errno = 0;
                i32 lit = strtol(second_operand->val, &end, 10);
                if (ident->vreg.storage.type == MICRO_STORAGE_STACK) {
                    push_asm_instr(MICRO_ASM386_INSTR_CMP_S32I32, { .imm = micro_imm_le_gen(ident->vreg.storage.stack.ebp_offset) }, { .imm = micro_imm_le_gen(lit) });
                } else
                if (ident->vreg.storage.type == MICRO_STORAGE_REG) {
                    push_asm_instr(((micro_asm386_instruction_type_t[]){
                        [MICRO_SIZE_8]  = MICRO_ASM386_INSTR_CMP_R8I8,
                        [MICRO_SIZE_16] = MICRO_ASM386_INSTR_CMP_R16I16,
                        [MICRO_SIZE_32] = MICRO_ASM386_INSTR_CMP_R32I32,
                    })[ident->vreg.storage.reg.size], { .reg = ident->vreg.storage.reg.reg }, { .imm = micro_imm_le_gen(lit) });
                }
                return 3;
            }
            if (_micro_tok_is_op(second_operand->type)) {
                int free_space = get_last_free_space(codegen);
                micro_codegen386_storage_t op_dst;
                if (free_space < 0) {
                    op_dst.type = MICRO_STORAGE_STACK;
                    op_dst.stack.ebp_offset = free_space;
                } else {
                    op_dst.type = MICRO_STORAGE_REG;
                    op_dst.reg.reg = free_space;
                    op_dst.reg.size = MICRO_SIZE_32;
                }

                size_t expr_size = expr_parse(codegen, op_dst, second_operand);
                if (!expr_size) {
                    return 0;
                }
                
                if (op_dst.type == MICRO_STORAGE_STACK) {
                    if (ident->vreg.storage.type == MICRO_STORAGE_STACK) {
                        push_asm_instr(MICRO_ASM386_INSTR_PUSH_R32, { .reg = MICRO_ASM386_REG32_EAX }, {});
                        push_asm_instr(MICRO_ASM386_INSTR_MOV_R32S32, { .reg = MICRO_ASM386_REG32_EAX }, { .imm = micro_imm_le_gen(op_dst.stack.ebp_offset) });
                        push_asm_instr(((micro_asm386_instruction_type_t[]){
                            [MICRO_SIZE_8]  = MICRO_ASM386_INSTR_CMP_S32R8,
                            [MICRO_SIZE_16] = MICRO_ASM386_INSTR_CMP_S32R16,
                            [MICRO_SIZE_32] = MICRO_ASM386_INSTR_CMP_S32R32,
                        })[micro_type_to_size[ident->vreg.type]], { .imm = micro_imm_le_gen(ident->vreg.storage.stack.ebp_offset) }, { .reg = MICRO_ASM386_REG32_EAX });
                        push_asm_instr(MICRO_ASM386_INSTR_POP_R32, { .reg = MICRO_ASM386_REG32_EAX }, {});
                    } else
                    if (ident->vreg.storage.type == MICRO_STORAGE_REG) {
                        push_asm_instr(((micro_asm386_instruction_type_t[]){
                            [MICRO_SIZE_8]  = MICRO_ASM386_INSTR_CMP_R8S32,
                            [MICRO_SIZE_16] = MICRO_ASM386_INSTR_CMP_R16S32,
                            [MICRO_SIZE_32] = MICRO_ASM386_INSTR_CMP_R32S32,
                        })[ident->vreg.storage.reg.size], { .reg = ident->vreg.storage.reg.reg }, { .imm = micro_imm_le_gen(op_dst.stack.ebp_offset) });
                    }
                } else
                if (op_dst.type == MICRO_STORAGE_REG) {
                    if (ident->vreg.storage.type == MICRO_STORAGE_STACK) {
                        push_asm_instr(((micro_asm386_instruction_type_t[]){
                            [MICRO_SIZE_8]  = MICRO_ASM386_INSTR_CMP_S32R8,
                            [MICRO_SIZE_16] = MICRO_ASM386_INSTR_CMP_S32R16,
                            [MICRO_SIZE_32] = MICRO_ASM386_INSTR_CMP_S32R32,
                        })[micro_type_to_size[ident->vreg.type]], { .imm = micro_imm_le_gen(ident->vreg.storage.stack.ebp_offset) }, { .reg = op_dst.reg.reg });
                    } else
                    if (ident->vreg.storage.type == MICRO_STORAGE_REG) {
                        push_asm_instr(((micro_asm386_instruction_type_t[]){
                            [MICRO_SIZE_8]  = MICRO_ASM386_INSTR_CMP_R8R8,
                            [MICRO_SIZE_16] = MICRO_ASM386_INSTR_CMP_R16R16,
                            [MICRO_SIZE_32] = MICRO_ASM386_INSTR_CMP_R32R32,
                        })[micro_type_to_size[ident->vreg.type]], { .reg = ident->vreg.storage.stack.ebp_offset }, { .reg = op_dst.reg.reg });
                    }
                }
                return 2 + expr_size;
            }
            if (second_operand->type == MICRO_TOK_IDENT) {
                puts("ds");
                micro_codegen386_ident_t *ident2 = sct_hashmap_get(&ext->idents, second_operand->val);

                if (ident2->type == MICRO_IDENT_VREG) {
                    if (ident2->vreg.storage.type == MICRO_STORAGE_STACK) {
                        if (ident->vreg.storage.type == MICRO_STORAGE_STACK) {
                            push_asm_instr(MICRO_ASM386_INSTR_PUSH_R32, { .reg = MICRO_ASM386_REG32_EAX }, {});
                            push_asm_instr(MICRO_ASM386_INSTR_MOV_R32S32, { .reg = MICRO_ASM386_REG32_EAX }, { .imm = micro_imm_le_gen(ident2->vreg.storage.stack.ebp_offset) });
                            push_asm_instr(((micro_asm386_instruction_type_t[]){
                                [MICRO_SIZE_8]  = MICRO_ASM386_INSTR_CMP_S32R8,
                                [MICRO_SIZE_16] = MICRO_ASM386_INSTR_CMP_S32R16,
                                [MICRO_SIZE_32] = MICRO_ASM386_INSTR_CMP_S32R32,
                            })[micro_type_to_size[ident->vreg.type]], { .imm = micro_imm_le_gen(ident->vreg.storage.stack.ebp_offset) }, { .reg = MICRO_ASM386_REG32_EAX });
                            push_asm_instr(MICRO_ASM386_INSTR_POP_R32, { .reg = MICRO_ASM386_REG32_EAX }, {});
                        } else
                        if (ident->vreg.storage.type == MICRO_STORAGE_REG) {
                                push_asm_instr(((micro_asm386_instruction_type_t[]){
                                [MICRO_SIZE_8]  = MICRO_ASM386_INSTR_CMP_R8S32,
                                [MICRO_SIZE_16] = MICRO_ASM386_INSTR_CMP_R16S32,
                                [MICRO_SIZE_32] = MICRO_ASM386_INSTR_CMP_R32S32,
                            })[ident->vreg.storage.reg.size], { .reg = ident->vreg.storage.reg.reg }, { .imm = micro_imm_le_gen(ident2->vreg.storage.stack.ebp_offset) });
                        }
                    } else
                    if (ident2->vreg.storage.type == MICRO_STORAGE_REG) {
                        if (ident->vreg.storage.type == MICRO_STORAGE_STACK) {
                            push_asm_instr(((micro_asm386_instruction_type_t[]){
                                [MICRO_SIZE_8]  = MICRO_ASM386_INSTR_CMP_S32R8,
                                [MICRO_SIZE_16] = MICRO_ASM386_INSTR_CMP_S32R16,
                                [MICRO_SIZE_32] = MICRO_ASM386_INSTR_CMP_S32R32,
                            })[micro_type_to_size[ident->vreg.type]], { .imm = micro_imm_le_gen(ident->vreg.storage.stack.ebp_offset) }, { .reg = ident2->vreg.storage.reg.reg });
                        } else
                        if (ident->vreg.storage.type == MICRO_STORAGE_REG) {
                            push_asm_instr(((micro_asm386_instruction_type_t[]){
                                [MICRO_SIZE_8]  = MICRO_ASM386_INSTR_CMP_R8R8,
                                [MICRO_SIZE_16] = MICRO_ASM386_INSTR_CMP_R16R16,
                                [MICRO_SIZE_32] = MICRO_ASM386_INSTR_CMP_R32R32,
                            })[micro_type_to_size[ident->vreg.type]], { .reg = ident->vreg.storage.stack.ebp_offset }, { .reg = ident2->vreg.storage.reg.reg });
                        }
                    }
                    return 3;
                }
                return 0;
            }
        }
        return 0;
    }
    return 0;
}

#endif