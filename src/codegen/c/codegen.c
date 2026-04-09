#include "../codegen.h"

void micro_codegen_outbuf_init(micro_codegen_outbuf_t *outbuf)
{
    outbuf->arr = malloc(MICRO_OUTBUF_EXTEND_SIZE * sizeof(u8));
    outbuf->size = 0;
    outbuf->real_size = 0;
}

void micro_codegen_outbuf_free(micro_codegen_outbuf_t *outbuf)
{
    free(outbuf->arr);
}

void micro_codegen_outbuf_push(micro_codegen_outbuf_t *outbuf, u8 b)
{
    if (outbuf->size + 1 >= outbuf->real_size) {
        u8 *new_arr = malloc((outbuf->real_size += MICRO_TOKEN_VEC_EXTEND_SIZE) * sizeof(u8));
        memcpy(new_arr, outbuf->arr, sizeof(u8) * outbuf->size);
        free(outbuf->arr);
        outbuf->arr = new_arr;
    }
    outbuf->arr[outbuf->size++] = b;
}
