#include "../internal.h"

int lowering_fun(micro_codegen_t *codegen, micro_instruction_t *instr)
{
    micro_codegen386_ext_t *ext = _micro_codegen386_ext(codegen);

    micro_instruction_fun_t instr_fun = instr->fun;

    char *fun_lbl_name = sct_arena_alloc(ext->arena, sizeof(char) * strlen(instr_fun.name) + 1);
    strcpy(fun_lbl_name, instr_fun.name);
    push_asm_instr(MICRO_ASM386_INSTR_LBL, { .lbl_name = fun_lbl_name }, {});

    ext->curent_function_name = fun_lbl_name;

    push_asm_instr(MICRO_ASM386_INSTR_PRELUDE, {0}, {0});

    size_t sub_instr_addr = codegen->asm_instrs->size;

    push_asm_instr(MICRO_ASM386_INSTR_SUB_R32I32, { .reg = MICRO_ASM386_REG32_ESP }, { .imm = micro_imm_le_gen(0) });
    size_t callee_save_addr = codegen->asm_instrs->size - 1;

    char *start_lbl_name = sct_arena_alloc(ext->arena, sizeof(char) * strlen(instr_fun.name) + 1 + 6);
    strcpy(start_lbl_name, instr_fun.name);
    strcat(start_lbl_name, ".start");
    push_asm_instr(MICRO_ASM386_INSTR_LBL, { .lbl_name = start_lbl_name }, {});

    ext->ebp_top_offset = -4;
    ext->max_stack_offset = 0;
    ext->in_function = 1;
    ext->use_callee_save_regs = 0;
    for (int i = 0; i < 8; i++) {
        ext->used_regs[i] = 0;
    }
    ext->used_regs[4] = 1;
    ext->used_regs[5] = 1;

    micro_codegen386_ident_fun_t fun = {
        .instr_info = instr_fun,
    };

    sct_hashmap_add(&ext->idents, instr_fun.name, &(micro_codegen386_ident_t){
        .type = MICRO_IDENT_FUN,
        .fun  = fun,
    });

    sct_vector_t save_idents;
    sct_vector_init(&save_idents, sizeof(char*));
    for (size_t i = 0; i < ext->idents.keys.size; i++) {
        sct_vector_push(&save_idents, sct_vector_get(&ext->idents.keys, i));
    }

    for (size_t i = 0; i < instr_fun.args.size; i++) {
        micro_instruction_fun_arg_t *arg = sct_vector_get(&instr_fun.args, i);
        
        micro_codegen386_ident_t ident = { .type = MICRO_IDENT_VREG };
        strcpy(ident.vreg.name, arg->name);
        ident.vreg.type = arg->type;
        ident.vreg.storage.type = MICRO_STORAGE_STACK;
        ident.vreg.storage.stack.ebp_offset = 8 + i * 4;

        sct_hashmap_add(&ext->idents, arg->name, &ident);
    }

    sct_vector_t *instrs_save = codegen->instrs;
    size_t pos_save = codegen->pos;
        codegen->emit(codegen, &instr_fun.body);
    codegen->instrs = instrs_save;
    codegen->pos = pos_save;

    sct_vector_t remove_idents;
    sct_vector_init(&remove_idents, sizeof(char*));
    for (size_t i = 0; i < ext->idents.keys.size; i++) {
        char *ident_name = *(char**)sct_vector_get(&ext->idents.keys, i);

        int keep = 0;
        for (size_t j = 0; j < save_idents.size; j++) {
            char *save_name = *(char**)sct_vector_get(&save_idents, j);
            if (!strcmp(ident_name, save_name)) {
                keep = 1;
                break;
            }
        }
        if (!keep) {
            sct_vector_push(&remove_idents, &ident_name);
        }
    }

    for (size_t i = 0; i < remove_idents.size; i++) {
        char *ident_name = *(char**)sct_vector_get(&remove_idents, i);
        sct_hashmap_remove(&ext->idents, ident_name);
    }

    sct_vector_deinit(&remove_idents);
    sct_vector_deinit(&save_idents);

    micro_imm_le_t imm = micro_imm_le_gen((i32)(0x100000000 - ext->max_stack_offset));
    ((micro_asm386_instruction_t*)sct_vector_get(codegen->asm_instrs, sub_instr_addr))->operand2.imm = imm;

    char *exit_lbl_name = sct_arena_alloc(ext->arena, sizeof(char) * strlen(instr_fun.name) + 1 + 4);
    strcpy(exit_lbl_name, instr_fun.name);
    strcat(exit_lbl_name, ".end");
    push_asm_instr(MICRO_ASM386_INSTR_LBL, { .lbl_name = exit_lbl_name }, {});

    if (ext->use_callee_save_regs) {
        sct_vector_insert(codegen->asm_instrs, callee_save_addr, &(micro_asm386_instruction_t){
            .opcode = MICRO_ASM386_INSTR_PUSH_R32,
            .operand1 = MICRO_ASM386_REG32_EBX
        });
        sct_vector_insert(codegen->asm_instrs, callee_save_addr, &(micro_asm386_instruction_t){
            .opcode = MICRO_ASM386_INSTR_PUSH_R32,
            .operand1 = MICRO_ASM386_REG32_ESI
        });
        sct_vector_insert(codegen->asm_instrs, callee_save_addr, &(micro_asm386_instruction_t){
            .opcode = MICRO_ASM386_INSTR_PUSH_R32,
            .operand1 = MICRO_ASM386_REG32_EDI
        });

        sct_vector_push(codegen->asm_instrs, &(micro_asm386_instruction_t){
            .opcode = MICRO_ASM386_INSTR_POP_R32,
            .operand1 = MICRO_ASM386_REG32_EBX
        });
        sct_vector_push(codegen->asm_instrs, &(micro_asm386_instruction_t){
            .opcode = MICRO_ASM386_INSTR_POP_R32,
            .operand1 = MICRO_ASM386_REG32_ESI
        });
        sct_vector_push(codegen->asm_instrs, &(micro_asm386_instruction_t){
            .opcode = MICRO_ASM386_INSTR_POP_R32,
            .operand1 = MICRO_ASM386_REG32_EDI
        });
    }

    push_asm_instr(MICRO_ASM386_INSTR_ADD_R32I32, { .reg = MICRO_ASM386_REG32_ESP }, { .imm = micro_imm_le_gen((i32)(0x100000000 - ext->max_stack_offset)) });

    push_asm_instr(MICRO_ASM386_INSTR_EPILOGUE, {0}, {0});

    for (size_t i = 0; i < ext->goto_unfound_labels.size; i++) {
        micro_codegen386_goto_unfound_lbl_t *lbl = sct_vector_get(&ext->goto_unfound_labels, i);
        micro_push_err((micro_error_t){
            .err = MICRO_ERROR_UNDEFINED_LBL,
            .line_ref = lbl->lbl_tok->line_ref,
            .chpos_ref = lbl->lbl_tok->chpos_ref
        });
        return 1;
    }
    sct_vector_deinit(&ext->goto_unfound_labels);
    sct_vector_init(&ext->goto_unfound_labels, sizeof(micro_codegen386_goto_unfound_lbl_t));

    return 0;
}
