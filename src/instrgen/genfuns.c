#include <micro/instrgen.h>

int micro_instrgen_gen_set(sct_vector_t *instrs, micro_type_t type, const char *name, sct_vector_t *expr)
{
    if (!instrs || type == MICRO_TYPE_NULL || !name || strlen(name) > 63 || !expr) {
        return 1;
    }

    size_t expr_size = micro_scroll_expr(expr, 0);

    micro_instruction_set_t instr_set;
    strcpy(instr_set.reg_name, name);
    instr_set.type = type;
    instr_set.val_expr = &expr->data;

    sct_vector_push(instrs, &(micro_instruction_t){
        .type = MICRO_INSTR_SET,
        .start_tok = 0,
        .set = instr_set,
    });
}