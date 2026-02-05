#include "../types.h"

void micro_gen16imm_le(u8 *buf, offset_t imm)
{
    buf[0] = (u8)imm;
    buf[1] = (u8)(imm >> 8);
}

void micro_gen32imm_le(u8 *buf, offset_t imm)
{
    buf[0] = (u8)imm;
    buf[1] = (u8)(imm >> 8);
    buf[2] = (u8)(imm >> 16);
    buf[3] = (u8)(imm >> 24);
}

void micro_gen16imm_be(u8 *buf, offset_t imm)
{
    buf[1] = (u8)imm;
    buf[0] = (u8)(imm >> 8);
}

void micro_gen32imm_be(u8 *buf, offset_t imm)
{
    buf[3] = (u8)imm;
    buf[2] = (u8)(imm >> 8);
    buf[1] = (u8)(imm >> 16);
    buf[0] = (u8)(imm >> 24);
}