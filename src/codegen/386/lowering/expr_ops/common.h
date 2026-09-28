#ifndef MICRO_CODEGEN386_EXPR_OPS_COMMON_H
#define MICRO_CODEGEN386_EXPR_OPS_COMMON_H

#include "errno.h"
#include "limits.h"

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
            push_asm_instr(op_tbls->opMI_tbl[dst.datasec.size], operand_addr(micro_imm_le_gen(dst.datasec.address)), operand_imm(MICRO_SIZE_32, micro_imm_le_gen(lit)));
            return;
            
        case MICRO_STORAGE_STACK:
            push_asm_instr(op_tbls->opSI_fn, operand_imm(MICRO_SIZE_32, micro_imm_le_gen(dst.stack.ebp_offset)), operand_imm(MICRO_SIZE_32, micro_imm_le_gen(lit)));
            return;

        case MICRO_STORAGE_REG:
            push_asm_instr(op_tbls->opRI_tbl[dst.reg.size], operand_reg(dst.reg.size, dst.reg.reg), operand_imm(MICRO_SIZE_32, micro_imm_le_gen(lit)));
            return;
    }
}

static expr_info_t op_expr_to_dst(micro_codegen_t *codegen, const op_tbls_t *op_tbls, micro_codegen386_storage_t dst, micro_expr_tok_t *expr)
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

    expr_info_t expr_info = expr_parse(codegen, expr_dst, expr);
    if (!expr_info.size) return (expr_info_t){ 0, MICRO_TYPE_NULL };

    if (expr_dst.type == MICRO_STORAGE_REG) {
        switch (dst.type) {
            case MICRO_STORAGE_DATASEC:
                push_asm_instr(op_tbls->opMR_tbl[dst.datasec.size], operand_addr(micro_imm_le_gen(dst.datasec.address)), operand_reg(MICRO_SIZE_32, expr_dst.reg.reg));
                break;

            case MICRO_STORAGE_STACK:
                push_asm_instr(op_tbls->opSR_fn, operand_imm(MICRO_SIZE_32, micro_imm_le_gen(dst.stack.ebp_offset)), operand_reg(MICRO_SIZE_32, expr_dst.reg.reg));
                break;

            case MICRO_STORAGE_REG:
                push_asm_instr(op_tbls->opRR_tbl[dst.reg.size], operand_reg(dst.reg.size, dst.reg.reg), operand_reg(MICRO_SIZE_32, expr_dst.reg.reg));
                break;
        }
    } else
    if (expr_dst.type == MICRO_STORAGE_STACK) {
        switch (dst.type) {
            case MICRO_STORAGE_DATASEC:
                if (ext->used_regs[0]) {
                    push_asm_instr(MICRO_ASM386_INSTR_MOV_S32R32, operand_imm(MICRO_SIZE_32, micro_imm_le_gen(ext->ebp_top_offset)), operand_reg(MICRO_SIZE_32, MICRO_ASM386_REG32_EAX));
                    ext->max_stack_offset -= 4;
                }
                push_asm_instr(MICRO_ASM386_INSTR_MOV_R32S32, operand_reg(MICRO_SIZE_32, MICRO_ASM386_REG32_EAX), operand_imm(MICRO_SIZE_32, micro_imm_le_gen(free_space)));
                push_asm_instr(op_tbls->opMR_tbl[dst.datasec.size], operand_addr(micro_imm_le_gen(dst.datasec.address)), operand_reg(MICRO_SIZE_32, MICRO_ASM386_REG32_EAX));
                if (ext->used_regs[0]) {
                    push_asm_instr(MICRO_ASM386_INSTR_MOV_R32S32, operand_reg(MICRO_SIZE_32, MICRO_ASM386_REG32_EAX), operand_imm(MICRO_SIZE_32, micro_imm_le_gen(ext->ebp_top_offset)));
                }
                break;

            case MICRO_STORAGE_STACK:
                if (ext->used_regs[0]) {
                    push_asm_instr(MICRO_ASM386_INSTR_MOV_S32R32, operand_imm(MICRO_SIZE_32, micro_imm_le_gen(ext->ebp_top_offset)), operand_reg(MICRO_SIZE_32, MICRO_ASM386_REG32_EAX));
                    ext->max_stack_offset -= 4;
                }
                push_asm_instr(op_tbls->opRS_tbl[MICRO_SIZE_32], operand_reg(MICRO_SIZE_32, MICRO_ASM386_REG32_EAX), operand_imm(MICRO_SIZE_32, micro_imm_le_gen(free_space)));
                push_asm_instr(op_tbls->opSR_fn, operand_imm(MICRO_SIZE_32, micro_imm_le_gen(dst.stack.ebp_offset)), operand_reg(MICRO_SIZE_32, MICRO_ASM386_REG32_EAX));
                if (ext->used_regs[0]) {
                    push_asm_instr(MICRO_ASM386_INSTR_MOV_R32S32, operand_reg(MICRO_SIZE_32, MICRO_ASM386_REG32_EAX), operand_imm(MICRO_SIZE_32, micro_imm_le_gen(ext->ebp_top_offset)));
                }
                break;

            case MICRO_STORAGE_REG:
                push_asm_instr(op_tbls->opRS_tbl[dst.reg.size], operand_reg(dst.reg.size, dst.reg.reg), operand_imm(MICRO_SIZE_32, micro_imm_le_gen(free_space)));
                break;
        }
    }
    return expr_info;
}

static void op_vreg_to_dst(micro_codegen_t *codegen, const op_tbls_t *op_tbls, micro_codegen386_storage_t dst, micro_codegen386_ident_vreg_t vreg)
{
    micro_codegen386_ext_t *ext = _micro_codegen386_ext(codegen);

    if (unlikely(vreg.storage.type == MICRO_STORAGE_DATASEC)) {
        micro_push_err((micro_error_t){
            .err = MICRO_ERROR_EXPR_PARSE,
        });
        return;
    }

    switch (vreg.storage.type) {
        case MICRO_STORAGE_STACK:
            switch (dst.type) {
                case MICRO_STORAGE_DATASEC:
                    if (ext->used_regs[0]) {
                        push_asm_instr(MICRO_ASM386_INSTR_MOV_S32R32, operand_imm(MICRO_SIZE_32, micro_imm_le_gen(ext->ebp_top_offset)), operand_reg(MICRO_SIZE_32, MICRO_ASM386_REG32_EAX));
                        ext->max_stack_offset -= 4;
                    }
                    push_asm_instr(movRS_tbl[micro_type_to_size[vreg.type]], operand_reg(MICRO_SIZE_32, 0), operand_imm(MICRO_SIZE_32, micro_imm_le_gen(vreg.storage.stack.ebp_offset)));
                    push_asm_instr(op_tbls->opMR_tbl[dst.datasec.size], operand_addr(micro_imm_le_gen(dst.datasec.address)), operand_reg(MICRO_SIZE_32, 0));
                    if (ext->used_regs[0]) {
                        push_asm_instr(MICRO_ASM386_INSTR_MOV_R32S32, operand_reg(MICRO_SIZE_32, MICRO_ASM386_REG32_EAX), operand_imm(MICRO_SIZE_32, micro_imm_le_gen(ext->ebp_top_offset)));
                    }
                    break;

                case MICRO_STORAGE_STACK:
                    if (ext->used_regs[0]) {
                        push_asm_instr(MICRO_ASM386_INSTR_MOV_S32R32, operand_imm(MICRO_SIZE_32, micro_imm_le_gen(ext->ebp_top_offset)), operand_reg(MICRO_SIZE_32, MICRO_ASM386_REG32_EAX));
                        ext->max_stack_offset -= 4;
                    }
                    push_asm_instr(movRS_tbl[micro_type_to_size[vreg.type]], operand_reg(MICRO_SIZE_32, 0), operand_imm(MICRO_SIZE_32, micro_imm_le_gen(vreg.storage.stack.ebp_offset)));
                    push_asm_instr(op_tbls->opSR_fn, operand_addr(micro_imm_le_gen(dst.stack.ebp_offset)), operand_reg(MICRO_SIZE_32, 0));
                    if (ext->used_regs[0]) {
                        push_asm_instr(MICRO_ASM386_INSTR_MOV_R32S32, operand_reg(MICRO_SIZE_32, MICRO_ASM386_REG32_EAX), operand_imm(MICRO_SIZE_32, micro_imm_le_gen(ext->ebp_top_offset)));
                    }
                    break;

                case MICRO_STORAGE_REG:
                    push_asm_instr(op_tbls->opRS_tbl[dst.reg.size], operand_reg(dst.reg.size, dst.reg.reg), operand_imm(MICRO_SIZE_32, micro_imm_le_gen(vreg.storage.stack.ebp_offset)));
                    break;
            };
            break;

        case MICRO_STORAGE_REG:
            switch (dst.type) {
                case MICRO_STORAGE_DATASEC:
                    push_asm_instr(op_tbls->opMR_tbl[dst.datasec.size], operand_addr(micro_imm_le_gen(dst.datasec.address)), operand_reg(vreg.storage.reg.size, vreg.storage.reg.reg));
                    break;

                case MICRO_STORAGE_STACK:
                    push_asm_instr(op_tbls->opSR_fn, operand_addr(micro_imm_le_gen(dst.stack.ebp_offset)), operand_reg(vreg.storage.reg.size, vreg.storage.reg.reg));
                    break;

                case MICRO_STORAGE_REG:
                    push_asm_instr(op_tbls->opRR_tbl[dst.reg.size], operand_reg(dst.reg.size, dst.reg.reg), operand_reg(vreg.storage.reg.size, vreg.storage.reg.reg));
                    break;
            };
            break;
    };
}

typedef struct {
    micro_codegen386_ident_t *base;   // may be null
    micro_codegen386_ident_t *index;  // may be null
    i32                        scale;
    i32                        disp;
    size_t                     size;   // consumed tokens
} lea_pattern_t;

static i32 lea_scale_of_lit(i32 lit)
{
    return lit == 2 || lit == 4 || lit == 8 ? lit : 0;
}

static int lea_disp_add_ok(i32 a, i32 b)
{
    return !((b > 0 && a > INT32_MAX - b) || (b < 0 && a < INT32_MIN - b));
}

static int lea_disp_mul_ok(i32 lit, i32 scale)
{
    return lit <= INT32_MAX / scale && lit >= INT32_MIN / scale;
}

static int lea_match_index(micro_codegen_t *codegen, micro_expr_tok_t *tok, lea_pattern_t *out)
{
    micro_codegen386_ext_t *ext = _micro_codegen386_ext(codegen);

    if (tok->type == MICRO_EXPR_TOK_IDENT) {
        micro_codegen386_ident_t *ident = sct_hashmap_get(&ext->idents, tok->val);
        if (!ident || ident->type != MICRO_IDENT_VREG) return 0;
        *out = (lea_pattern_t){ .index = ident, .size = 1 };
        return 1;
    }

    if (unlikely(tok->type != MICRO_EXPR_TOK_PLUS)) return 0;

    micro_expr_tok_t *lhs = tok + 1;
    micro_expr_tok_t *rhs = tok + 2;
    micro_expr_tok_t *ident_tok;

    if (lhs->type == MICRO_EXPR_TOK_IDENT && rhs->type == MICRO_EXPR_TOK_LIT_INT) {
        ident_tok = lhs;
        *out = (lea_pattern_t){ .disp = strtol(rhs->val, NULL, 10), .size = 3 };
    } else
    if (lhs->type == MICRO_EXPR_TOK_LIT_INT && rhs->type == MICRO_EXPR_TOK_IDENT) {
        ident_tok = rhs;
        *out = (lea_pattern_t){ .disp = strtol(lhs->val, NULL, 10), .size = 3 };
    } else {
        return 0;
    }

    micro_codegen386_ident_t *ident = sct_hashmap_get(&ext->idents, ident_tok->val);
    if (!ident || ident->type != MICRO_IDENT_VREG) return 0;

    out->index = ident;
    return 1;
}

static int lea_match_scaled(micro_codegen_t *codegen, micro_expr_tok_t *star_tok, lea_pattern_t *out)
{
    if (unlikely(star_tok->type != MICRO_EXPR_TOK_STAR)) return 0;

    lea_pattern_t pat;
    micro_expr_tok_t *lit_tok;

    if (lea_match_index(codegen, star_tok + 1, &pat)) {
        lit_tok = star_tok + 1 + pat.size;
    } else
    if (star_tok[1].type == MICRO_EXPR_TOK_LIT_INT) {
        lit_tok = star_tok + 1;
        if (!lea_match_index(codegen, star_tok + 2, &pat)) return 0;
    } else {
        return 0;
    }

    if (lit_tok->type != MICRO_EXPR_TOK_LIT_INT) return 0;
    i32 scale = lea_scale_of_lit(strtol(lit_tok->val, NULL, 10));
    if (unlikely(!scale)) return 0;
    if (unlikely(!lea_disp_mul_ok(pat.disp, scale))) return 0;

    pat.scale = scale;
    pat.disp *= scale;
    pat.size += 2;
    *out = pat;
    return 1;
}

static int lea_match_add_scaled(micro_codegen_t *codegen, micro_expr_tok_t *plus_tok, lea_pattern_t *out)
{
    micro_codegen386_ext_t *ext = _micro_codegen386_ext(codegen);

    if (unlikely(plus_tok->type != MICRO_EXPR_TOK_PLUS)) return 0;
    if (unlikely(plus_tok[2].type != MICRO_EXPR_TOK_STAR)) return 0;

    lea_pattern_t pat;
    if (!lea_match_scaled(codegen, plus_tok + 2, &pat)) return 0;

    micro_codegen386_ident_t *base = 0;
    if (plus_tok[1].type == MICRO_EXPR_TOK_LIT_INT) {
        i32 lit = strtol(plus_tok[1].val, NULL, 10);
        if (unlikely(!lea_disp_add_ok(pat.disp, lit))) return 0;
        pat.disp += lit;
    } else
    if (plus_tok[1].type == MICRO_EXPR_TOK_IDENT) {
        base = sct_hashmap_get(&ext->idents, plus_tok[1].val);
        if (!base || base->type != MICRO_IDENT_VREG) return 0;
    } else {
        return 0;
    }

    pat.base = base;
    pat.size += 2;
    *out = pat;
    return 1;
}

static int code_selection_lea(micro_codegen_t *codegen, micro_codegen386_storage_t dst, const lea_pattern_t *pat)
{
    micro_codegen386_ext_t *ext = _micro_codegen386_ext(codegen);

    if (unlikely(!sib_scale_valid(pat->scale))) return 1;

    micro_asm386_reg_t i1_rep = MICRO_ASM386_REG32_NO_BASE;
    if (pat->base) {
        if (unlikely(pat->base->type != MICRO_IDENT_VREG)) return 1;
        if (unlikely(pat->base->vreg.storage.type != MICRO_STORAGE_REG)) return 1;
        i1_rep = pat->base->vreg.storage.reg.reg;
    }

    micro_asm386_reg_t i2_rep = MICRO_ASM386_REG32_NO_INDEX;
    if (pat->index) {
        if (unlikely(pat->index->type != MICRO_IDENT_VREG)) return 1;
        if (unlikely(pat->index->vreg.storage.type != MICRO_STORAGE_REG)) return 1;
        i2_rep = pat->index->vreg.storage.reg.reg;
        if (unlikely(i2_rep == MICRO_ASM386_REG32_ESP)) return 1;
    }

    if (unlikely(i1_rep == MICRO_ASM386_REG32_NO_BASE && i2_rep == MICRO_ASM386_REG32_NO_INDEX)) return 1;

    int need_temp = dst.type != MICRO_STORAGE_REG;
    int temp_reg = 0;
    if (need_temp) {
        temp_reg = get_last_free_space(codegen);
        if (unlikely(temp_reg < 0)) return 1;
        ext->used_regs[temp_reg] = 1;
    }

    micro_asm386_instruction_type_t instr;
    if (i1_rep == MICRO_ASM386_REG32_NO_BASE) {
        instr = MICRO_ASM386_INSTR_LEA_R32SIBABS;
    } else if (pat->disp == 0 && i1_rep != MICRO_ASM386_REG32_EBP) {
        instr = MICRO_ASM386_INSTR_LEA_R32SIB;
    } else if (pat->disp >= -128 && pat->disp <= 127) {
        instr = MICRO_ASM386_INSTR_LEA_R32SIBI8;
    } else {
        instr = MICRO_ASM386_INSTR_LEA_R32SIBI32;
    }

    micro_asm386_reg_t out_reg = need_temp ? (micro_asm386_reg_t)temp_reg : dst.reg.reg;
    push_asm_instr_with_sib(instr,
                            operand_reg(MICRO_SIZE_32, out_reg),
                            operand_imm(MICRO_SIZE_32, micro_imm_le_gen(pat->disp)),
                            pat->scale,
                            i2_rep,
                            i1_rep);

    if (need_temp) {
        switch (dst.type) {
            case MICRO_STORAGE_DATASEC:
                push_asm_instr(movMR_tbl[dst.datasec.size], operand_addr(micro_imm_le_gen(dst.datasec.address)), operand_reg(MICRO_SIZE_32, temp_reg));
                break;

            case MICRO_STORAGE_STACK:
                push_asm_instr(MICRO_ASM386_INSTR_MOV_S32R32, operand_imm(MICRO_SIZE_32, micro_imm_le_gen(dst.stack.ebp_offset)), operand_reg(MICRO_SIZE_32, temp_reg));
                break;

            case MICRO_STORAGE_REG:
                break;
        }
        ext->used_regs[temp_reg] = 0;
    }
    return 0;
}

#endif