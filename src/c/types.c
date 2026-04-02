#include "../types.h"

micro_imm_be_t micro_imm_be_gen(i32 val)
{
    return (micro_imm_be_t){
        .val = val,
        .bytes = {
            (u8)(val >> 24), (u8)(val >> 16), (u8)(val >> 8), (u8)val
        }
    };
}

micro_imm_le_t micro_imm_le_gen(i32 val)
{
    return (micro_imm_le_t){
        .val = val,
        .bytes = {
            (u8)val, (u8)(val >> 8), (u8)(val >> 16), (u8)(val >> 24)
        }
    };
}
