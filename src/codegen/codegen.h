#ifndef MICRO_CODEGEN_H
#define MICRO_CODEGEN_H

#include <SCT/string.h>
#include "../common.h"
#include "../lexer.h"

typedef struct {
    micro_tok_vec_t *toks;
    sct_string_t    *outbuf;
    size_t           toks_pos;
    void           (*codegen)();
    void            *ext;
} micro_codegen_t;

#endif