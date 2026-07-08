#ifndef MICRO_CODEGEN386_H
#define MICRO_CODEGEN386_H

#include <micro/codegen/codegen.h>

#define _micro_codegen386_ext(Cg) ((micro_codegen386_ext_t*)Cg->ext)

typedef struct {
    int in_function;
    // ...
} micro_codegen386_ext_t;

void micro_codegen386_init(micro_codegen_t *codegen);

void micro_codegen386_deinit(micro_codegen_t *codegen);

#endif