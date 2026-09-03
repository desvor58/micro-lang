/*************************************************
 *              micro-debug library
 *
 * Optional debug/print helpers for the micro
 * toolchain. Built separately as libmicro-debug.a
 * and is not required by libmicro.
 ************************************************/

#include <microdebug/microdebug.h>
#include <stdio.h>

const char *micro_debug_err_str[] = {
    [MICRO_ERROR_NONE]                      = "",

    [MICRO_ERROR_EXPECTED_COMMENT_CLOSE]    = "Expected comment close",
    [MICRO_ERROR_EXPECTED_STRING_CLOSE]     = "Expected string close",

    [MICRO_ERROR_EXPECTED_EXPRESSION]       = "Expected expression",
    [MICRO_ERROR_EXPECTED_SEMICOLON]        = "Expected ';'",
    [MICRO_ERROR_EXPECTED_TYPE_NAME]        = "Expected type name",
    [MICRO_ERROR_UNDEFINED_TYPE_NAME]       = "Undefined type name",
    [MICRO_ERROR_UNEXPECTED_TOKEN]          = "Unexpected token",
    [MICRO_ERROR_UNEXPECTED_END_KW]         = "Unexpected 'end' keyword",

    [MICRO_ERROR_EXPECTED_SET_KW]           = "Expected 'set' keyword",
    [MICRO_ERROR_EXPECTED_FUN_KW]           = "Expected 'fun' keyword",
    [MICRO_ERROR_EXPECTED_RET_KW]           = "Expected 'ret' keyword",
    [MICRO_ERROR_EXPECTED_CALL_KW]          = "Expected 'call' keyword",
    [MICRO_ERROR_EXPECTED_GOTO_KW]          = "Expected 'goto' keyword",
    [MICRO_ERROR_EXPECTED_START_KW]         = "Expected 'start' keyword",
    [MICRO_ERROR_EXPECTED_END_KW]           = "Expected 'end' keyword",
    [MICRO_ERROR_EXPECTED_IF_KW]            = "Expected 'if' keyword",
    [MICRO_ERROR_EXPECTED_ELSE_KW]          = "Expected 'else' keyword",

    [MICRO_ERROR_EXPECTED_FUN_NAME]         = "Expected function name",
    [MICRO_ERROR_EXPECTED_ARG_TYPE]         = "Expected argument type",
    [MICRO_ERROR_EXPECTED_ARG_NAME]         = "Expected argument name",
    [MICRO_ERROR_EXPECTED_RET_TYPE]         = "Expected return type",
    [MICRO_ERROR_EXPECTED_VREG_NAME]        = "Expected vreg name",
    [MICRO_ERROR_EXPECTED_RESULT_REG]       = "Expected result register",
    [MICRO_ERROR_EXPECTED_LABEL_NAME]       = "Expected label name",
    [MICRO_ERROR_EXPECTED_COLON]            = "Expected ':'",

    [MICRO_ERROR_SET_OUTSIDE_FUNCTION]      = "'set' can be only in function body",
    [MICRO_ERROR_FUN_INSIDE_FUNCTION]       = "'fun' can be only outside function",
    [MICRO_ERROR_RET_OUTSIDE_FUNCTION]      = "'ret' can be only in function body",
    [MICRO_ERROR_CALL_OUTSIDE_FUNCTION]     = "'call' can be only in function body",
    [MICRO_ERROR_GOTO_OUTSIDE_FUNCTION]     = "'goto' can be only in function body",
    [MICRO_ERROR_LBL_OUTSIDE_FUNCTION]      = "label can be declared only in function body",
    [MICRO_ERROR_IF_OUTSIDE_FUNCTION]       = "'if' can be only in function body",

    [MICRO_ERROR_UNDEFINED_IDENT]           = "Undefined identifier",
    [MICRO_ERROR_UNDEFINED_FUN]             = "Undefined function",
    [MICRO_ERROR_EXPECTED_VREG_RESULT]      = "Expected vreg as call result",
    [MICRO_ERROR_RESULT_TYPE_MISMATCH]      = "Call result type mismatch",
    [MICRO_ERROR_TOO_FEW_ARGS]              = "Too few arguments in call",
    [MICRO_ERROR_TOO_MANY_ARGS]             = "Too many arguments in call",
    [MICRO_ERROR_VREG_TYPE_MISMATCH]        = "Type mismatch with declared vreg",
    [MICRO_ERROR_EXPR_PARSE]                = "Expression parse error",

    [MICRO_ERROR_IDENT_NOT_VREG]            = "Identifier is not a virtual register",
    [MICRO_ERROR_IDENT_NOT_LBL]             = "Identifier is not a label",

    [MICRO_ERROR_UNDEFINED_LBL]             = "Undefined label",
};

const char *micro_debug_instr_str[] = {
    [MICRO_INSTR_NONE]   = "",
    [MICRO_INSTR_SET]    = "set",
    [MICRO_INSTR_DRSET]  = "drset",
    [MICRO_INSTR_FUN]    = "fun",
    [MICRO_INSTR_RET]    = "ret",
    [MICRO_INSTR_CALL]   = "call",
    [MICRO_INSTR_LBL]    = "label",
    [MICRO_INSTR_GOTO]   = "goto",
    [MICRO_INSTR_IF]     = "if",
};

const char *micro_debug_str_type[] = {
    [MICRO_TYPE_NULL] = "null",
    [MICRO_TYPE_I8]   = "i8",
    [MICRO_TYPE_U8]   = "u8",
    [MICRO_TYPE_I16]  = "i16",
    [MICRO_TYPE_U16]  = "u16",
    [MICRO_TYPE_I32]  = "i32",
    [MICRO_TYPE_U32]  = "u32",
    [MICRO_TYPE_F32]  = "f32",
    [MICRO_TYPE_PTR]  = "ptr"
};

u8 micro_debug_op_args_num[] = {
    [MC_TOK_PLUS]        = 2,
    [MC_TOK_MINUS]       = 2,
    [MC_TOK_STAR]        = 2,
    [MC_TOK_SLASH]       = 2,
    [MC_TOK_AMPERSAND]   = 1,
    [MC_TOK_DOLLAR]      = 1,
    [MC_TOK_HASH]        = 1,
    [MC_TOK_APOSTROPHE]  = 1,
    [MC_TOK_TILDE]       = 1,
    [MC_TOK_EQ]          = 2,
    [MC_TOK_EXCLAMATION] = 1,
    [MC_TOK_GREAT]       = 2,
    [MC_TOK_LESS]        = 2,
    [MC_TOK_GREAT_OR_EQ] = 2,
    [MC_TOK_LESS_OR_EQ]  = 2,
};

void micro_debug_put_err(const char *file, micro_error_t err)
{
    printf("Error: %s[%s]: %s\n", file, micro_debug_instr_str[err.instr], micro_debug_err_str[err.err]);
}

void micro_debug_put_tok(mc_token_t tok)
{
    printf("%lu:%lu type:%s, val:%s\n",
                       tok.line_ref,
                       tok.chpos_ref,
                       mc_token_type2str[tok.type],
                       tok.val);
}

size_t micro_debug_print_expr(micro_expr_tok_t *start, size_t tab)
{
    if (!start) return 0;
    for (size_t i = 0; i < tab; i++) {
        putchar(' ');
    }
    micro_debug_put_tok(*(mc_token_t*)start);
    if (_micro_expr_is_lit(start->type) || start->type == MICRO_EXPR_TOK_IDENT) {
        return 1;
    }
    if (_micro_expr_is_op(start->type)) {
        u8 num = micro_debug_op_args_num[start->type];
        size_t offset = 0;
        while (num) {
            offset += micro_debug_print_expr(start + offset + 1, tab);
            num--;
        }
        return offset;
    }
    puts("micro_debug_print_expr error!");
    return 0;
}

void micro_debug_print_instructions(sct_vector_t *instrs, size_t tab)
{
    for (size_t i = 0; i < instrs->size; i++) {
        micro_instruction_t *instr = sct_vector_get(instrs, i);
        for (size_t j = 0; j < tab; j++) {
            putchar(' ');
        }
        switch (instr->type) {
            case MICRO_INSTR_SET:
                printf("SET: type:%s, name:'%s'\n", micro_debug_str_type[instr->set.type], instr->set.reg_name);
                micro_debug_print_expr(instr->set.val_expr, tab + 5);
                break;

            case MICRO_INSTR_DRSET:
                printf("DRSET: type:%s, name:'%s'\n", micro_debug_str_type[instr->drset.type], instr->drset.reg_name);
                micro_debug_print_expr(instr->set.val_expr, tab + 5);
                break;

            case MICRO_INSTR_FUN:
                printf("FUN: ret_type:%s, name:'%s'\n", micro_debug_str_type[instr->fun.ret_type], instr->fun.name);
                printf("     args:\n");
                for (size_t j = 0; j < instr->fun.args.size; j++) {
                    micro_instruction_fun_arg_t *arg = sct_vector_get(&instr->fun.args, j);
                    printf("       type:%s, name:'%s'\n", micro_debug_str_type[arg->type], arg->name);
                }
                printf("     body:\n");
                micro_debug_print_instructions(&instr->fun.body, tab + 7);
                break;

            case MICRO_INSTR_RET:
                printf("RET\n");
                micro_debug_print_expr(instr->ret.val_expr, tab + 5);
                break;

            case MICRO_INSTR_CALL:
                printf("CALL: res_reg:'%s', fun:'%s'\n", instr->call.ret_reg_name, instr->call.fun_name);
                printf("       args:\n");
                for (size_t j = 0; j < instr->call.arg_exprs.size; j++) {
                    micro_debug_print_expr(*(micro_expr_tok_t**)sct_vector_get(&instr->call.arg_exprs, j), tab + 6);
                    puts("");
                }
                break;

            case MICRO_INSTR_LBL:
                printf("LBL '%s'\n", instr->lbl.name);
                break;

            case MICRO_INSTR_GOTO:
                printf("GOTO '%s'\n", instr->goto_lbl.lbl);
                break;

            case MICRO_INSTR_IF:
                printf("IF -> %s\n", instr->if_goto.lbl_name);
                micro_debug_print_expr(instr->if_goto.cond_expr, tab + 3);
                break;

            default:
                puts("wrong instr->type");
                break;
        }
    }
}

typedef struct {
    const char *name;
    u8  kind1, size1;
    u8  kind2, size2;
} micro_debug_asm_fmt_t;

#define A(name, k1, s1, k2, s2)  { name, k1, s1, k2, s2 }
#define A1(name, k1, s1)         { name, k1, s1, 0, 0 }
#define A0(name)                 { name, 0, 0, 0, 0 }

static const char *reg8[]  = { "al", "cl", "dl", "bl", "ah", "ch", "dh", "bh" };
static const char *reg16[] = { "ax", "cx", "dx", "bx", "sp", "bp", "si", "di" };
static const char *reg32[] = { "eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi" };

static const micro_debug_asm_fmt_t asm_tbl[] = {
    [MICRO_ASM386_INSTR_MOV_R8R8]    = A("movR8R8",     'R', 8,  'R', 8),
    [MICRO_ASM386_INSTR_MOV_R16R16]  = A("movR16R16",   'R', 16, 'R', 16),
    [MICRO_ASM386_INSTR_MOV_R32R32]  = A("movR32R32",   'R', 32, 'R', 32),
    [MICRO_ASM386_INSTR_MOV_R8I8]    = A("movR8I8",     'R', 8,  'V', 0),
    [MICRO_ASM386_INSTR_MOV_R16I16]  = A("movR16I16",   'R', 16, 'V', 0),
    [MICRO_ASM386_INSTR_MOV_R32I32]  = A("movR32I32",   'R', 32, 'V', 0),
    [MICRO_ASM386_INSTR_MOV_M8R8]    = A("movM8R8",     'V', 0,  'R', 8),
    [MICRO_ASM386_INSTR_MOV_M16R16]  = A("movM16R16",   'V', 0,  'R', 16),
    [MICRO_ASM386_INSTR_MOV_M32R32]  = A("movM32R32",   'V', 0,  'R', 32),
    [MICRO_ASM386_INSTR_MOV_R8M8]    = A("movR8M8",     'R', 8,  'V', 0),
    [MICRO_ASM386_INSTR_MOV_R16M16]  = A("movR16M16",   'R', 16, 'V', 0),
    [MICRO_ASM386_INSTR_MOV_R32M32]  = A("movR32M32",   'R', 32, 'V', 0),
    [MICRO_ASM386_INSTR_MOV_M8I8]    = A("movM8I8",     'V', 0,  'V', 0),
    [MICRO_ASM386_INSTR_MOV_M16I16]  = A("movM16I16",   'V', 0,  'V', 0),
    [MICRO_ASM386_INSTR_MOV_M32I32]  = A("movM32I32",   'V', 0,  'V', 0),
    [MICRO_ASM386_INSTR_MOV_S32I32]  = A("movS32I32",   'V', 0,  'V', 0),
    [MICRO_ASM386_INSTR_MOV_S32I16]  = A("movS32I16",   'V', 0,  'V', 0),
    [MICRO_ASM386_INSTR_MOV_S32I8]   = A("movS32I8",    'V', 0,  'V', 0),
    [MICRO_ASM386_INSTR_MOV_S32R32]  = A("movS32R32",   'V', 0,  'R', 32),
    [MICRO_ASM386_INSTR_MOV_S32R16]  = A("movS32R16",   'V', 0,  'R', 16),
    [MICRO_ASM386_INSTR_MOV_S32R8]   = A("movS32R8",    'V', 0,  'R', 8),
    [MICRO_ASM386_INSTR_MOV_R32S32]  = A("movR32S32",   'R', 32, 'V', 0),
    [MICRO_ASM386_INSTR_MOV_R16S32]  = A("movR16S32",   'R', 16, 'V', 0),
    [MICRO_ASM386_INSTR_MOV_R8S32]   = A("movR8S32",    'R', 8,  'V', 0),
    [MICRO_ASM386_INSTR_MOV_R32L32]  = A("movR32L32",   'R', 32, 'L', 0),

    [MICRO_ASM386_INSTR_ADD_R32R32]  = A("addR32R32",   'R', 32, 'R', 32),
    [MICRO_ASM386_INSTR_ADD_R16R16]  = A("addR16R16",   'R', 16, 'R', 16),
    [MICRO_ASM386_INSTR_ADD_R8R8]    = A("addR8R8",     'R', 8,  'R', 8),
    [MICRO_ASM386_INSTR_ADD_R32I32]  = A("addR32I32",   'R', 32, 'V', 0),
    [MICRO_ASM386_INSTR_ADD_R16I16]  = A("addR16I16",   'R', 16, 'V', 0),
    [MICRO_ASM386_INSTR_ADD_R8I8]    = A("addR8I8",     'R', 8,  'V', 0),
    [MICRO_ASM386_INSTR_ADD_M32R32]  = A("addM32R32",   'V', 0,  'R', 32),
    [MICRO_ASM386_INSTR_ADD_M16R16]  = A("addM16R16",   'V', 0,  'R', 16),
    [MICRO_ASM386_INSTR_ADD_M8R8]    = A("addM8R8",     'V', 0,  'R', 8),

    [MICRO_ASM386_INSTR_RET]         = A0("ret"),
    [MICRO_ASM386_INSTR_CALL_S32]    = A1("callS32",    'V', 0),
    [MICRO_ASM386_INSTR_PRELUDE]     = A0("prelude"),
    [MICRO_ASM386_INSTR_EPILOGUE]    = A0("epilogue"),

    [MICRO_ASM386_INSTR_CMP_R32R32]  = A("cmpR32R32",   'R', 32, 'R', 32),
    [MICRO_ASM386_INSTR_CMP_R16R16]  = A("cmpR16R16",   'R', 16, 'R', 16),
    [MICRO_ASM386_INSTR_CMP_R8R8]    = A("cmpR8R8",     'R', 8,  'R', 8),

    [MICRO_ASM386_INSTR_CMP_M32I32]  = A("cmpM32I32",   'V', 32, 'V', 32),
    [MICRO_ASM386_INSTR_CMP_M16I16]  = A("cmpM16I16",   'V', 16, 'V', 16),
    [MICRO_ASM386_INSTR_CMP_M8I8]    = A("cmpM8I8",     'V', 8,  'V', 8),

    [MICRO_ASM386_INSTR_CMP_R32I32]  = A("cmpR32I32",   'R', 32, 'V', 32),
    [MICRO_ASM386_INSTR_CMP_R16I16]  = A("cmpR16I16",   'R', 16, 'V', 16),
    [MICRO_ASM386_INSTR_CMP_R8I8]    = A("cmpR8I8",     'R', 8,  'V', 8),

    [MICRO_ASM386_INSTR_CMP_S32I32]  = A("cmpS32I32",   'V', 32, 'V', 32),
    [MICRO_ASM386_INSTR_CMP_S32I16]  = A("cmpS32I16",   'V', 16, 'V', 16),
    [MICRO_ASM386_INSTR_CMP_S32I8]   = A("cmpS32I8",    'V', 8,  'V', 8),

    [MICRO_ASM386_INSTR_MOVZX_R32R8] = A("movzxR32R8",  'R', 32, 'R', 8),
    [MICRO_ASM386_INSTR_MOVZX_R16R8] = A("movzxR16R8",  'R', 16, 'R', 8),

    [MICRO_ASM386_INSTR_TEST_R32R32] = A("testR32R32",  'R', 32, 'R', 32),
    [MICRO_ASM386_INSTR_TEST_R16R16] = A("testR16R16",  'R', 16, 'R', 16),
    [MICRO_ASM386_INSTR_TEST_R8R8]   = A("testR8R8",    'R', 8,  'R', 8),
    [MICRO_ASM386_INSTR_TEST_M32I32] = A("testM32I32",  'V', 0,  'V', 0),
    [MICRO_ASM386_INSTR_TEST_M16I16] = A("testM16I16",  'V', 0,  'V', 0),
    [MICRO_ASM386_INSTR_TEST_M8I8]   = A("testM8I8",    'V', 0,  'V', 0),
    [MICRO_ASM386_INSTR_TEST_S32I32] = A("testS32I32",  'V', 0,  'V', 0),
    [MICRO_ASM386_INSTR_TEST_S32I16] = A("testS32I16",  'V', 0,  'V', 0),
    [MICRO_ASM386_INSTR_TEST_S32I8]  = A("testS32I8",   'V', 0,  'V', 0),

    [MICRO_ASM386_INSTR_JZ_L32]      = A1("jzL32",      'L', 0),
    [MICRO_ASM386_INSTR_JNZ_L32]     = A1("jnzL32",     'L', 0),

    [MICRO_ASM386_INSTR_JL_L32]      = A1("jlL32",      'L', 0),
    [MICRO_ASM386_INSTR_JNL_L32]     = A1("jnlL32",     'L', 0),
    [MICRO_ASM386_INSTR_JG_L32]      = A1("jgL32",      'L', 0),
    [MICRO_ASM386_INSTR_JNG_L32]     = A1("jngL32",     'L', 0),
    [MICRO_ASM386_INSTR_JB_L32]      = A1("jbL32",      'L', 0),
    [MICRO_ASM386_INSTR_JNB_L32]     = A1("jnbL32",     'L', 0),
    [MICRO_ASM386_INSTR_JA_L32]      = A1("jaL32",      'L', 0),
    [MICRO_ASM386_INSTR_JNA_L32]     = A1("jnaL32",     'L', 0),

    [MICRO_ASM386_INSTR_JLE_L32]     = A1("jle",        'L', 0),
    [MICRO_ASM386_INSTR_JNLE_L32]    = A1("jnle",       'L', 0),
    [MICRO_ASM386_INSTR_JGE_L32]     = A1("jge",        'L', 0),
    [MICRO_ASM386_INSTR_JNGE_L32]    = A1("jnge",       'L', 0),
    [MICRO_ASM386_INSTR_JBE_L32]     = A1("jbe",        'L', 0),
    [MICRO_ASM386_INSTR_JNBE_L32]    = A1("jnbe",       'L', 0),
    [MICRO_ASM386_INSTR_JAE_L32]     = A1("jae",        'L', 0),
    [MICRO_ASM386_INSTR_JNAE_L32]    = A1("jnae",       'L', 0),

    [MICRO_ASM386_INSTR_SETZ_R8]     = A1("setz",       'R', 8),
    [MICRO_ASM386_INSTR_SETNZ_R8]    = A1("setnz",      'R', 8),

    [MICRO_ASM386_INSTR_SETL_R8]     = A1("setl",       'R', 8),
    [MICRO_ASM386_INSTR_SETNL_R8]    = A1("setnl",      'R', 8),
    [MICRO_ASM386_INSTR_SETG_R8]     = A1("setg",       'R', 8),
    [MICRO_ASM386_INSTR_SETNG_R8]    = A1("setng",      'R', 8),
    [MICRO_ASM386_INSTR_SETB_R8]     = A1("setb",       'R', 8),
    [MICRO_ASM386_INSTR_SETNB_R8]    = A1("setnb",      'R', 8),
    [MICRO_ASM386_INSTR_SETA_R8]     = A1("seta",       'R', 8),
    [MICRO_ASM386_INSTR_SETNA_R8]    = A1("setna",      'R', 8),
    
    [MICRO_ASM386_INSTR_SETLE_R8]    = A1("setle",      'R', 8),
    [MICRO_ASM386_INSTR_SETNLE_R8]   = A1("setnle",     'R', 8),
    [MICRO_ASM386_INSTR_SETGE_R8]    = A1("setge",      'R', 8),
    [MICRO_ASM386_INSTR_SETNGE_R8]   = A1("setnge",     'R', 8),
    [MICRO_ASM386_INSTR_SETBE_R8]    = A1("setbe",      'R', 8),
    [MICRO_ASM386_INSTR_SETNBE_R8]   = A1("setnbe",     'R', 8),
    [MICRO_ASM386_INSTR_SETAE_R8]    = A1("setae",      'R', 8),
    [MICRO_ASM386_INSTR_SETNAE_R8]   = A1("setnae",     'R', 8),

    [MICRO_ASM386_INSTR_JMP_L32]     = A1("jmpL32",     'L', 0),

    [MICRO_ASM386_INSTR_XCHG_R32R32] = A("xchgR32R32",  'R', 32, 'R', 32),
    [MICRO_ASM386_INSTR_XCHG_R16R16] = A("xchgR16R16",  'R', 16, 'R', 16),
    [MICRO_ASM386_INSTR_XCHG_R8R8]   = A("xchgR8R8",    'R', 8,  'R', 8),

    [MICRO_ASM386_INSTR_PUSH_R32]    = A1("pushR32",    'R', 32),
    [MICRO_ASM386_INSTR_PUSH_R16]    = A1("pushR16",    'R', 16),
    [MICRO_ASM386_INSTR_POP_R32]     = A1("popR32",     'R', 32),
    [MICRO_ASM386_INSTR_POP_R16]     = A1("popR16",     'R', 16),

    [MICRO_ASM386_INSTR_MOV_R32MR32] = A("movR32MR32",  'R', 32, 'R', 32),
    [MICRO_ASM386_INSTR_MOV_R16MR16] = A("movR16MR16",  'R', 16, 'R', 16),
    [MICRO_ASM386_INSTR_MOV_R8MR8]   = A("movR8MR8",    'R', 8,  'R', 8),

    [MICRO_ASM386_INSTR_ADD_M32I32]  = A("addM32I32",   'V', 0,  'V', 0),
    [MICRO_ASM386_INSTR_ADD_M16I16]  = A("addM16I16",   'V', 0,  'V', 0),
    [MICRO_ASM386_INSTR_ADD_M8I8]    = A("addM8I8",     'V', 0,  'V', 0),
    [MICRO_ASM386_INSTR_ADD_S32I32]  = A("addS32I32",   'V', 0,  'V', 0),
    [MICRO_ASM386_INSTR_ADD_S32I16]  = A("addS32I16",   'V', 0,  'V', 0),
    [MICRO_ASM386_INSTR_ADD_S32I8]   = A("addS32I8",    'V', 0,  'V', 0),
    [MICRO_ASM386_INSTR_ADD_S32R32]  = A("addS32R32",   'V', 0,  'R', 32),
    [MICRO_ASM386_INSTR_ADD_S32R16]  = A("addS32R16",   'V', 0,  'R', 16),
    [MICRO_ASM386_INSTR_ADD_S32R8]   = A("addS32R8",    'V', 0,  'R', 8),
    [MICRO_ASM386_INSTR_ADD_R32S32]  = A("addR32S32",   'R', 32, 'V', 0),
    [MICRO_ASM386_INSTR_ADD_R16S32]  = A("addR16S32",   'R', 16, 'V', 0),
    [MICRO_ASM386_INSTR_ADD_R8S32]   = A("addR8S32",    'R', 8,  'V', 0),

    [MICRO_ASM386_INSTR_SUB_R32R32]  = A("subR32R32",   'R', 32, 'R', 32),
    [MICRO_ASM386_INSTR_SUB_R16R16]  = A("subR16R16",   'R', 16, 'R', 16),
    [MICRO_ASM386_INSTR_SUB_R8R8]    = A("subR8R8",     'R', 8,  'R', 8),
    [MICRO_ASM386_INSTR_SUB_M32I32]  = A("subM32I32",   'V', 0,  'V', 0),
    [MICRO_ASM386_INSTR_SUB_M16I16]  = A("subM16I16",   'V', 0,  'V', 0),
    [MICRO_ASM386_INSTR_SUB_M8I8]    = A("subM8I8",     'V', 0,  'V', 0),
    [MICRO_ASM386_INSTR_SUB_M32R32]  = A("subM32R32",   'V', 0,  'R', 32),
    [MICRO_ASM386_INSTR_SUB_M16R16]  = A("subM16R16",   'V', 0,  'R', 16),
    [MICRO_ASM386_INSTR_SUB_M8R8]    = A("subM8R8",     'V', 0,  'R', 8),
    [MICRO_ASM386_INSTR_SUB_S32I32]  = A("subS32I32",   'V', 0,  'V', 0),
    [MICRO_ASM386_INSTR_SUB_S32I16]  = A("subS32I16",   'V', 0,  'V', 0),
    [MICRO_ASM386_INSTR_SUB_S32I8]   = A("subS32I8",    'V', 0,  'V', 0),
    [MICRO_ASM386_INSTR_SUB_S32R32]  = A("subS32R32",   'V', 0,  'R', 32),
    [MICRO_ASM386_INSTR_SUB_S32R16]  = A("subS32R16",   'V', 0,  'R', 16),
    [MICRO_ASM386_INSTR_SUB_S32R8]   = A("subS32R8",    'V', 0,  'R', 8),
    [MICRO_ASM386_INSTR_SUB_R32I32]  = A("subR32I32",   'R', 32, 'V', 0),
    [MICRO_ASM386_INSTR_SUB_R16I16]  = A("subR16I16",   'R', 16, 'V', 0),
    [MICRO_ASM386_INSTR_SUB_R8I8]    = A("subR8I8",     'R', 8,  'V', 0),
    [MICRO_ASM386_INSTR_SUB_R32S32]  = A("subR32S32",   'R', 32, 'V', 0),
    [MICRO_ASM386_INSTR_SUB_R16S32]  = A("subR16S32",   'R', 16, 'V', 0),
    [MICRO_ASM386_INSTR_SUB_R8S32]   = A("subR8S32",    'R', 8,  'V', 0),

    [MICRO_ASM386_INSTR_MUL_R32]     = A1("mulR32",     'R', 32),
    [MICRO_ASM386_INSTR_MUL_R16]     = A1("mulR16",     'R', 16),
    [MICRO_ASM386_INSTR_MUL_R8]      = A1("mulR8",      'R', 8),
    [MICRO_ASM386_INSTR_IMUL_R32R32] = A("imulR32R32",  'R', 32, 'R', 32),
    [MICRO_ASM386_INSTR_IMUL_R16R16] = A("imulR16R16",  'R', 16, 'R', 16),
    [MICRO_ASM386_INSTR_IMUL_R8R8]   = A("imulR8R8",    'R', 8,  'R', 8),

    [MICRO_ASM386_INSTR_DIV_R32]     = A1("divR32",     'R', 32),
    [MICRO_ASM386_INSTR_DIV_R16]     = A1("divR16",     'R', 16),
    [MICRO_ASM386_INSTR_DIV_R8]      = A1("divR8",      'R', 8),
    [MICRO_ASM386_INSTR_IDIV_R32]    = A1("idivR32",    'R', 32),
    [MICRO_ASM386_INSTR_IDIV_R16]    = A1("idivR16",    'R', 16),
    [MICRO_ASM386_INSTR_IDIV_R8]     = A1("idivR8",     'R', 8),

    [MICRO_ASM386_INSTR_NEG_R32]     = A1("negR32",     'R', 32),
    [MICRO_ASM386_INSTR_NEG_R16]     = A1("negR16",     'R', 16),
    [MICRO_ASM386_INSTR_NEG_R8]      = A1("negR8",      'R', 8),

    [MICRO_ASM386_INSTR_LEA_R32S32]  = A("leaR32S32",   'R', 32, 'V', 0),
    [MICRO_ASM386_INSTR_LEA_R16S32]  = A("leaR16S32",   'R', 16, 'V', 0),

    [MICRO_ASM386_INSTR_CALL_L32]    = A1("lbl", 'L', 0),

    [MICRO_ASM386_INSTR_LBL]         = A1("lbl", 'L', 0),
};

#undef A
#undef A1
#undef A0

void micro_debug_put_asm(sct_vector_t *asm_instrs)
{
    for (size_t i = 0; i < asm_instrs->size; i++) {
        micro_asm386_instruction_t *instr = sct_vector_get(asm_instrs, i);
        if (instr->opcode == MICRO_ASM386_INSTR_NONE ||
            instr->opcode >= sizeof(asm_tbl) / sizeof(asm_tbl[0])) {
            continue;
        }

        char tab[] = "    ";

        if (instr->opcode == MICRO_ASM386_INSTR_LBL) {
            printf("%s:\n", instr->operand1.lbl_name);
            continue;
        }
        if (instr->opcode == MICRO_ASM386_INSTR_CALL_L32) {
            printf("%scallL32 %s\n", tab, instr->operand1.lbl_name);
            continue;
        }

        const micro_debug_asm_fmt_t *fmt = &asm_tbl[instr->opcode];
        printf("%s%s", tab, fmt->name);

        const u8 kinds[2] = { fmt->kind1, fmt->kind2 };
        const u8 sizes[2] = { fmt->size1, fmt->size2 };
        const micro_asm386_instruction_operand_t *ops[2] = { &instr->operand1, &instr->operand2 };

        for (int n = 0; n < 2; n++) {
            if (!kinds[n]) break;
            printf(n == 0 ? " " : ", ");
            if (kinds[n] == 'R') {
                const char **tbl = (sizes[n] == 32) ? reg32
                                 : (sizes[n] == 16) ? reg16
                                 :                    reg8;
                printf("%s", tbl[ops[n]->reg]);
            } else
            if (kinds[n] == 'L') {
                printf("%s", ops[n]->lbl_name);
            } else {
                printf("%d", ops[n]->imm.val);
            }
        }
        puts("");
    }
}