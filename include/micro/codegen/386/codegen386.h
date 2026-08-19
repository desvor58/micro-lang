#ifndef MICRO_CODEGEN386_H
#define MICRO_CODEGEN386_H

#include <micro/codegen/codegen.h>
#include <micro/asm/asm386.h>

#define _micro_codegen386_ext(Cg) ((micro_codegen386_ext_t*)Cg->ext)

typedef enum {
    MICRO_STORAGE_DATASEC,
    MICRO_STORAGE_STACK,
    MICRO_STORAGE_REG,
} micro_codegen386_storage_type_t;

typedef struct {
    size_t       address;
    micro_size_t size;
} micro_codegen386_storage_datasec_t;

typedef struct {
    ptrdiff_t ebp_offset;
    // all at stack have size = 4 bytes
} micro_codegen386_storage_stack_t;

typedef struct {
    micro_asm386_reg_t reg;
    micro_size_t       size;
} micro_codegen386_storage_reg_t;

typedef struct {
    micro_codegen386_storage_type_t type;
    union {
        micro_codegen386_storage_datasec_t datasec;
        micro_codegen386_storage_stack_t   stack;
        micro_codegen386_storage_reg_t     reg;
    };
} micro_codegen386_storage_t;

typedef enum {
    MICRO_IDENT_FUN,
    MICRO_IDENT_VREG,
    MICRO_IDENT_LBL,
} micro_codegen386_ident_type_t;

typedef struct {
    micro_instruction_fun_t instr_info;
    u32                     address;
} micro_codegen386_ident_fun_t;

typedef struct {
    char                       name[MICRO_MAX_SYMBOL_SIZE];
    micro_type_t               type;
    micro_codegen386_storage_t storage;
} micro_codegen386_ident_vreg_t;

typedef struct {
    char name[MICRO_MAX_SYMBOL_SIZE];
    u32  address;
} micro_codegen386_ident_lbl_t;

typedef struct {
    micro_codegen386_ident_type_t type;
    union {
        micro_codegen386_ident_fun_t  fun;
        micro_codegen386_ident_vreg_t vreg;
        micro_codegen386_ident_lbl_t  lbl;
    };
} micro_codegen386_ident_t;

typedef struct {
    sct_arena_t arena;
    struct {
        u32 in_function : 1;
        u32 use_callee_save_regs : 1;
    };
    ptrdiff_t     ebp_top_offset;
    ptrdiff_t     max_stack_offset;
    sct_hashmap_t idents;
    int           used_regs[8];
    char         *curent_function_label;
} micro_codegen386_ext_t;

void micro_codegen386_init(micro_codegen_t *codegen);

void micro_codegen386_deinit(micro_codegen_t *codegen);

#endif