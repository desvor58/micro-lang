#ifndef MICRO_CODEGEN_H
#define MICRO_CODEGEN_H

#include <SCT/string.h>
#include "../common.h"
#include "../lexer.h"

typedef struct {
    u8    *arr;
    size_t size;
    size_t real_size;
} micro_codegen_outbuf_t;

void micro_codegen_outbuf_init(micro_codegen_outbuf_t *outbuf);
void micro_codegen_outbuf_free(micro_codegen_outbuf_t *outbuf);
void micro_codegen_outbuf_push(micro_codegen_outbuf_t *outbuf, u8 b);

typedef enum {
    MICRO_CE_386,
} micro_codegen_ext_type;

typedef struct {
    micro_tok_vec_t        *toks;
    micro_codegen_outbuf_t *outbuf;
    size_t                  toks_pos;
    void                   *ext;
    micro_codegen_ext_type  ext_type;
} micro_codegen_t;

#endif