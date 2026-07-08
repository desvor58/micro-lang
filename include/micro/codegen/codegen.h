#ifndef MICRO_CODEGEN_CODEGEN_H
#define MICRO_CODEGEN_CODEGEN_H

#include <micro/common.h>
#include <micro/instrgen.h>
#include <micro/asm/asm386.h>

typedef struct _micro_codegen_t micro_codegen_t;

typedef void (*micro_codegen_emit_fun_t)(micro_codegen_t *codegen, sct_vector_t *instrs);

struct _micro_codegen_t {
    sct_vector_t            *instrs;
    size_t                   pos;
    sct_vector_t             outbuf;
    micro_codegen_emit_fun_t emit;
    void                    *ext;
};

#endif