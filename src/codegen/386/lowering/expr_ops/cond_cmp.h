#ifndef MICRO_CODEGEN386_EXPR_OPS_COND_CMP_H
#define MICRO_CODEGEN386_EXPR_OPS_COND_CMP_H

#include "common.h"

expr_info_t cond_op_cmp_handler(micro_codegen_t *codegen, micro_codegen386_storage_t dst, micro_expr_tok_t *start)
{
    micro_codegen386_ext_t *ext = _micro_codegen386_ext(codegen);

    micro_expr_tok_t *first_operand = start + 1;
    if (first_operand->type == MICRO_EXPR_TOK_IDENT) {
        micro_codegen386_ident_t *ident = sct_hashmap_get(&ext->idents, first_operand->val);

        micro_expr_tok_t *second_operand = start + 2;
        if (ident->type == MICRO_IDENT_VREG) {
            if (_micro_expr_is_lit(second_operand->type)) {
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
                return (expr_info_t){ 3, ident->vreg.type };
            }
            if (_micro_expr_is_op(second_operand->type)) {
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

                expr_info_t expr_info = expr_parse(codegen, expr_dst, second_operand);
                if (!expr_info.size) {
                    return (expr_info_t){ 0, MICRO_TYPE_NULL };
                }
                
                if (expr_dst.type == MICRO_STORAGE_STACK) {
                    if (ident->vreg.storage.type == MICRO_STORAGE_STACK) {
                        push_asm_instr(MICRO_ASM386_INSTR_PUSH_R32, { .reg = MICRO_ASM386_REG32_EAX }, {});
                        push_asm_instr(MICRO_ASM386_INSTR_MOV_R32S32, { .reg = MICRO_ASM386_REG32_EAX }, { .imm = micro_imm_le_gen(expr_dst.stack.ebp_offset) });
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
                        })[ident->vreg.storage.reg.size], { .reg = ident->vreg.storage.reg.reg }, { .imm = micro_imm_le_gen(expr_dst.stack.ebp_offset) });
                    }
                } else
                if (expr_dst.type == MICRO_STORAGE_REG) {
                    if (ident->vreg.storage.type == MICRO_STORAGE_STACK) {
                        push_asm_instr(((micro_asm386_instruction_type_t[]){
                            [MICRO_SIZE_8]  = MICRO_ASM386_INSTR_CMP_S32R8,
                            [MICRO_SIZE_16] = MICRO_ASM386_INSTR_CMP_S32R16,
                            [MICRO_SIZE_32] = MICRO_ASM386_INSTR_CMP_S32R32,
                        })[micro_type_to_size[ident->vreg.type]], { .imm = micro_imm_le_gen(ident->vreg.storage.stack.ebp_offset) }, { .reg = expr_dst.reg.reg });
                    } else
                    if (ident->vreg.storage.type == MICRO_STORAGE_REG) {
                        push_asm_instr(((micro_asm386_instruction_type_t[]){
                            [MICRO_SIZE_8]  = MICRO_ASM386_INSTR_CMP_R8R8,
                            [MICRO_SIZE_16] = MICRO_ASM386_INSTR_CMP_R16R16,
                            [MICRO_SIZE_32] = MICRO_ASM386_INSTR_CMP_R32R32,
                        })[micro_type_to_size[ident->vreg.type]], { .reg = ident->vreg.storage.stack.ebp_offset }, { .reg = expr_dst.reg.reg });
                    }
                }
                return (expr_info_t){ 2 + expr_info.size, ident->vreg.type };
            }
            if (second_operand->type == MICRO_EXPR_TOK_IDENT) {
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
                    return (expr_info_t){ 3, ident->vreg.type };
                }
            }
        }
        return (expr_info_t){ 0, MICRO_TYPE_NULL };
    }
    if (_micro_expr_is_op(first_operand->type)) {
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

        expr_info_t expr_info = expr_parse(codegen, expr_dst, first_operand);
        if (!expr_info.size) {
            return (expr_info_t){ 0, MICRO_TYPE_NULL };
        }

        micro_expr_tok_t *second_operand = start + expr_info.size + 1;
        if (second_operand->type == MICRO_EXPR_TOK_IDENT) {
            micro_codegen386_ident_t *ident = sct_hashmap_get(&ext->idents, second_operand->val);

            if (ident->type == MICRO_IDENT_VREG) {
                if (expr_dst.type == MICRO_STORAGE_STACK) {
                    if (ident->vreg.storage.type == MICRO_STORAGE_STACK) {
                        push_asm_instr(MICRO_ASM386_INSTR_PUSH_R32, { .reg = MICRO_ASM386_REG32_EAX }, {});
                        push_asm_instr(MICRO_ASM386_INSTR_MOV_R32S32, { .reg = MICRO_ASM386_REG32_EAX }, { .imm = micro_imm_le_gen(expr_dst.stack.ebp_offset) });
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
                        })[ident->vreg.storage.reg.size], { .reg = ident->vreg.storage.reg.reg }, { .imm = micro_imm_le_gen(expr_dst.stack.ebp_offset) });
                    }
                } else
                if (expr_dst.type == MICRO_STORAGE_REG) {
                    if (ident->vreg.storage.type == MICRO_STORAGE_STACK) {
                        push_asm_instr(((micro_asm386_instruction_type_t[]){
                            [MICRO_SIZE_8]  = MICRO_ASM386_INSTR_CMP_S32R8,
                            [MICRO_SIZE_16] = MICRO_ASM386_INSTR_CMP_S32R16,
                            [MICRO_SIZE_32] = MICRO_ASM386_INSTR_CMP_S32R32,
                        })[micro_type_to_size[ident->vreg.type]], { .imm = micro_imm_le_gen(ident->vreg.storage.stack.ebp_offset) }, { .reg = expr_dst.reg.reg });
                    } else
                    if (ident->vreg.storage.type == MICRO_STORAGE_REG) {
                        push_asm_instr(((micro_asm386_instruction_type_t[]){
                            [MICRO_SIZE_8]  = MICRO_ASM386_INSTR_CMP_R8R8,
                            [MICRO_SIZE_16] = MICRO_ASM386_INSTR_CMP_R16R16,
                            [MICRO_SIZE_32] = MICRO_ASM386_INSTR_CMP_R32R32,
                        })[micro_type_to_size[ident->vreg.type]], { .reg = ident->vreg.storage.stack.ebp_offset }, { .reg = expr_dst.reg.reg });
                    }
                }
            }
            return (expr_info_t){ 2 + expr_info.size, expr_info.type };
        }
        if (_micro_expr_is_lit(second_operand->type)) {
            char *end;
            errno = 0;
            i32 lit = strtol(second_operand->val, &end, 10);
            if (expr_dst.type == MICRO_STORAGE_STACK) {
                push_asm_instr(MICRO_ASM386_INSTR_CMP_S32I32, { .imm = micro_imm_le_gen(expr_dst.stack.ebp_offset) }, { .imm = micro_imm_le_gen(lit) });
            } else
            if (expr_dst.type == MICRO_STORAGE_REG) {
                push_asm_instr(MICRO_ASM386_INSTR_CMP_R32I32, { .reg = expr_dst.reg.reg }, { .imm = micro_imm_le_gen(lit) });
            }
            return (expr_info_t){ 2 + expr_info.size, expr_info.type };
        }
        if (_micro_expr_is_op(second_operand->type)) {
            int free_space2 = get_last_free_space(codegen);
            micro_codegen386_storage_t expr2_dst;

            int need_pop_eax = 0;
            if (free_space2 < 0) {
                need_pop_eax = 1;
                push_asm_instr(MICRO_ASM386_INSTR_PUSH_R32, { .reg = MICRO_ASM386_REG32_EAX }, {});
                ext->ebp_top_offset += 4;
                expr2_dst.type = MICRO_STORAGE_REG;
                expr2_dst.stack.ebp_offset = MICRO_ASM386_REG32_EAX;
                expr2_dst.reg.size = MICRO_SIZE_32;
            } else {
                expr2_dst.type = MICRO_STORAGE_REG;
                expr2_dst.reg.reg = free_space2;
                expr2_dst.reg.size = MICRO_SIZE_32;
            }

            expr_info_t expr2_info = expr_parse(codegen, expr2_dst, second_operand);
            if (!expr2_info.size) {
                return (expr_info_t){ 0, MICRO_TYPE_NULL };
            }

            if (expr_dst.type == MICRO_STORAGE_STACK) {
                push_asm_instr(MICRO_ASM386_INSTR_CMP_S32R32, { .imm = micro_imm_le_gen(expr_dst.stack.ebp_offset) }, { .reg = expr2_dst.reg.reg });
            } else
            if (expr_dst.type == MICRO_STORAGE_REG) {
                push_asm_instr(MICRO_ASM386_INSTR_CMP_R32R32, { .reg = expr_dst.reg.reg }, { .reg = expr2_dst.reg.reg });
            }

            if (need_pop_eax) {
                push_asm_instr(MICRO_ASM386_INSTR_POP_R32, { .reg = MICRO_ASM386_REG32_EAX }, {});
            }
        }
    }
    return (expr_info_t){ 0, MICRO_TYPE_NULL };
}

#endif