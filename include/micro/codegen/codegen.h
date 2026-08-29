#ifndef MICRO_CODEGEN_CODEGEN_H
#define MICRO_CODEGEN_CODEGEN_H

#include <micro/common.h>
#include <micro/instr.h>
#include <SCT/hashmap.h>

typedef struct _micro_codegen_t micro_codegen_t;

typedef void (*micro_codegen_emit_fun_t)(micro_codegen_t *codegen, sct_vector_t *instrs);

typedef struct {
    u32 no_err_outside_fun : 1;
} micro_codegen_flags_t;

struct _micro_codegen_t {
    sct_vector_t             *instrs;
    size_t                    pos;
    sct_vector_t             *asm_instrs;
    micro_codegen_emit_fun_t  emit;
    void                     *ext;
    micro_codegen_flags_t     flags;
};

#endif