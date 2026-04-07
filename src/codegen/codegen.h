#ifndef MICRO_CODEGEN_H
#define MICRO_CODEGEN_H

#include <SCT/string.h>
#include "../common.h"
#include "../lexer.h"

typedef enum {
    MICRO_CE_386,
} micro_codegen_ext_type;

typedef struct {
    micro_tok_vec_t       *toks;
    sct_string_t          *outbuf;
    size_t                 toks_pos;
    void                  *ext;
    micro_codegen_ext_type ext_type;
} micro_codegen_t;

#endif