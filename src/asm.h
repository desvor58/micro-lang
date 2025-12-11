#ifndef ASM_H
#define ASM_H

#include <malloc.h>
#include "types.h"

/// size for every realocation mcbuf
#define __mcbuf_aloc_size 64

/// machine code buffer
u8 *mcbuf = 0;
/// used size for mcbuf (accessible size)
size_t mcbuf_size = 0;
/// alocated size for mcbuf (real size)
size_t __mcbuf_real_size = 0;

/// @brief init the asemmbler
/// @return 0 - ok, !0 - err
int asm_init()
{
    mcbuf = (u8*)malloc(sizeof(u8)*(__mcbuf_real_size += __mcbuf_aloc_size));
    return !mcbuf;
}

void asm_free()
{
    free(mcbuf);
}

void __asm_realoc_check(size_t ExSz)
{
    if (mcbuf_size + ExSz >= __mcbuf_real_size) {
        size_t old_real_size = __mcbuf_real_size;
        u8 *new_mcbuf = (u8*)malloc(sizeof(u8)*(__mcbuf_real_size += ((ExSz / __mcbuf_aloc_size + 1) * __mcbuf_aloc_size)));
        for (size_t i = 0; i < mcbuf_size; i++) {
            new_mcbuf[i] = mcbuf[i];
        }
        free(mcbuf);
        mcbuf = new_mcbuf;
    }
}

/// @brief put to the buffer 16 bits immediante value from size_t
/// @param buf buf with size 2 to put the immVal
/// @param imm size_t value for put
void gen16imm(u8 *buf, size_t imm)
{
    buf[1] = (u8)imm;
    buf[0] = (u8)(imm >> 8);
}

/// @brief put to the buffer 32 bits immediante value from size_t
/// @param buf buf with size 4 to put the immVal
/// @param imm size_t value for put
void gen32imm(u8 *buf, size_t imm)
{
    buf[3] = (u8)imm;
    buf[2] = (u8)(imm >> 8);
    buf[1] = (u8)(imm >> 16);
    buf[0] = (u8)(imm >> 24);
}

#endif