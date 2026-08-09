#include "../internal.h"

int lowering_set(micro_codegen_t *codegen, micro_instruction_t *instr)
{
    micro_codegen386_ext_t *ext = _micro_codegen386_ext(codegen);

    if (!ext->in_function) {
        micro_push_err((micro_error_t){
            .msg = "'set' instruction can be only in function body",
            .line_ref = instr->start_tok->line_ref,
            .chpos_ref = instr->start_tok->chpos_ref
        });
        return 1;
    }

    micro_instruction_set_t instr_set = instr->set;
    
    micro_codegen386_ident_t *ident = sct_hashmap_get(&ext->idents, instr_set.reg_name);

    micro_codegen386_storage_t dst;
    if (ident) {
        if (ident->type != MICRO_IDENT_VREG) {
            micro_push_err((micro_error_t){
                .msg = "This ident already defined as not a virtual register",
                .line_ref = instr->start_tok->line_ref,
                .chpos_ref = instr->start_tok->chpos_ref
            });
            return 1;
        }
        if (ident->vreg.type != instr_set.type) {
            micro_push_err((micro_error_t){
                .msg = "Type mismatch with the already declared virtual register",
                .line_ref = instr->start_tok->line_ref,
                .chpos_ref = instr->start_tok->chpos_ref
            });
            return 1;
        }

        dst = ident->vreg.storage;
    } else {
        int space = get_last_free_space(codegen);

        if (space < 0) {
            dst.type = MICRO_STORAGE_STACK;
            dst.stack.ebp_offset = space;
            ext->max_stack_offset -= 4;
            ext->ebp_top_offset -= 4;
        } else {
            dst.type = MICRO_STORAGE_REG;
            dst.reg.reg = space;
            dst.reg.size = micro_type_to_size[instr_set.type];
        }

        micro_codegen386_ident_t new_ident = {
            .type = MICRO_IDENT_VREG,
            .vreg = (micro_codegen386_ident_vreg_t){
                .storage = dst,
                .type = instr_set.type,
            }
        };
        strcpy(new_ident.vreg.name, instr_set.reg_name);
        sct_hashmap_add(&ext->idents, instr_set.reg_name, &new_ident);
    }

    int expr_size = expr_parse(codegen, dst, instr_set.val_expr);
    if (!expr_size) {
        micro_push_err((micro_error_t){
            .msg = "expression parse error",
            .line_ref = instr_set.val_expr->line_ref,
            .chpos_ref = instr_set.val_expr->chpos_ref
        });
        return 1;
    }
    return 0;
}
