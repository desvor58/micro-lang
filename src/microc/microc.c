/************************************************
 *               Micro IR compiler
 *                   by Desvor
 * 
 *    Compiler created for Simple C Compiler,
 *     you can use it for your own compilers.
 *     All documentation in `docs` directory
 **********************************************/

#include <micro/micro.h>
#include <string.h>
#include <stdio.h>

#ifdef _WIN32
# define _CRTDBG_MAP_ALLOC
# include <stdlib.h>
# include <crtdbg.h>
#endif

typedef struct {
    char inputfile[MICRO_MAX_SYMBOL_SIZE];
    char outfile[MICRO_MAX_SYMBOL_SIZE];
    u8   toks_put   : 1;
    u8   instrs_put : 1;
    u8   asm_put    : 1;
    enum {
        STOPAFTER_NONE,
        STOPAFTER_FILE_READ,
        STOPAFTER_LEXER,
        STOPAFTER_INSTRGEN,
        STOPAFTER_OPTIMIZER,
    } stop_at;
} micro_args_t;

void print_usage()
{
    printf(
        "micro-lang dev-0.0.1\n"
        "dev: Desvor\n"
        "usage:\n"
        "    microc [flags] <input file>\n"
        "flags:\n"
        "    --help (-h)   - put this menu\n"
        "    --output (-o) - set output file\n"
        "    -P            - put some info\n"
        "      t           - put tokens\n"
        "      i           - put instructions\n"
        "      a           - put assembly\n"
        "    -S            - stop compiling\n"
        "      r           - stop after reading file\n"
        "      l           - stop after lexing\n"
        "      i           - stop after instruction generation\n"
        "      o           - stop after optimization stage (non supported now)\n"
    );
    exit(0);
}

micro_args_t micro_args_parse(int argc, char **argv)
{
    micro_args_t args;
    args.inputfile[0] = 0;
    strcpy(args.outfile, "a");
    args.toks_put = 0;
    args.instrs_put = 0;
    args.stop_at = STOPAFTER_NONE;

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            if (argv[i][1] == 'h' || !strcmp(argv[i], "--help")) {
                print_usage();
            }
            if (!strcmp(argv[i], "--output") || !strcmp(argv[i], "-o")) {
                strcpy(args.outfile, argv[++i]);
            } else
            if (argv[i][1] == 'P') {
                for (size_t j = 2; argv[i][j]; j++) {
                    if (argv[i][j] == 't') {
                        args.toks_put = 1;
                    } else
                    if (argv[i][j] == 'i') {
                        args.instrs_put = 1;
                    } else
                    if (argv[i][j] == 'a') {
                        args.asm_put = 1;
                    } else {
                        printf("Error: Unexpected symbol: '%c' (expected 't', 'i', 'a')", argv[i][j]);
                    }
                }
            } else
            if (argv[i][1] == 'S') {
                if (argv[i][2] == 'r') {
                    args.stop_at = STOPAFTER_FILE_READ;
                } else
                if (argv[i][2] == 'l') {
                    args.stop_at = STOPAFTER_LEXER;
                } else
                if (argv[i][2] == 'i') {
                    args.stop_at = STOPAFTER_INSTRGEN;
                } else
                if (argv[i][2] == 'o') {
                    args.stop_at = STOPAFTER_OPTIMIZER;
                } else {
                    printf("Error: Unexpected symbol: '%c' (expected 'r', 'l', 'i', 'o')", argv[i][2]);
                }
            } else {
                printf("Error: Undefined flag");
                print_usage();
            }
        } else {
            if (args.inputfile[0]) {
                printf("Error: Input file already set");
                print_usage();
            }
            strcpy(args.inputfile, argv[i]);
        }
    }

    return args;
}

char *err_str[] = {
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
    [MICRO_ERROR_LABEL_OUTSIDE_FUNCTION]    = "'lbl' can be only in function body",
    
    [MICRO_ERROR_UNDEFINED_IDENT]           = "Undefined identifier",
    [MICRO_ERROR_UNDEFINED_FUN]             = "Undefined function",
    [MICRO_ERROR_EXPECTED_VREG_RESULT]      = "Expected vreg as call result",
    [MICRO_ERROR_RESULT_TYPE_MISMATCH]      = "Call result type mismatch",
    [MICRO_ERROR_TOO_FEW_ARGS]              = "Too few arguments in call",
    [MICRO_ERROR_TOO_MANY_ARGS]             = "Too many arguments in call",
    [MICRO_ERROR_IDENT_NOT_VREG]            = "Identifier is not a virtual register",
    [MICRO_ERROR_VREG_TYPE_MISMATCH]        = "Type mismatch with declared vreg",
    [MICRO_ERROR_EXPR_PARSE]                = "Expression parse error",
};

void put_err(char *file, micro_error_t err)
{
    printf("Error: %s:%lu:%lu: %s\n", file, err.line_ref, err.chpos_ref, err_str[err.err]);
}

void print_tok(micro_token_t tok)
{
    printf("%lu:%lu type:%s, val:%s\n",
                       tok.line_ref,
                       tok.chpos_ref,
                       micro_token_type2str[tok.type],
                       tok.val);
}

char *str_type[] = {
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

u8 _op_args_num[] = {
    [MICRO_TOK_PLUS]        = 2,
    [MICRO_TOK_MINUS]       = 2,
    [MICRO_TOK_STAR]        = 2,
    [MICRO_TOK_SLASH]       = 2,
    [MICRO_TOK_AMPERSAND]   = 1,
    [MICRO_TOK_DOLLAR]      = 1,
    [MICRO_TOK_HASH]        = 1,
    [MICRO_TOK_APOSTROPHE]  = 1,
    [MICRO_TOK_TILDE]       = 1,
    [MICRO_TOK_EQ]          = 2,
    [MICRO_TOK_EXCLAMATION] = 1,
    [MICRO_TOK_NOT_EQ]      = 2,
    [MICRO_TOK_GREAT]       = 2,
    [MICRO_TOK_LESS]        = 2,
    [MICRO_TOK_GREAT_OR_EQ] = 2,
    [MICRO_TOK_LESS_OR_EQ]  = 2,
};

size_t print_expr(micro_token_t *start, size_t tab)
{
    if (!start) return 0;
    for (size_t i = 0; i < tab; i++) {
        putchar(' ');
    }
    print_tok(*start);
    if (_micro_tok_is_lit(start->type) || start->type == MICRO_TOK_IDENT) {
        return 1;
    }
    if (_micro_tok_is_op(start->type)) {
        u8 num = _op_args_num[start->type];
        size_t offset = 0;
        while (num) {
            offset += print_expr(start + offset + 1, tab);
            num--;
        }
        return offset;
    }
    puts("print_expr error!");
    return 0;
}

void print_instructions(sct_vector_t *instrs, size_t tab)
{
    for (size_t i = 0; i < instrs->size; i++) {
        micro_instruction_t *instr = sct_vector_get(instrs, i);
        for (size_t j = 0; j < tab; j++) {
            putchar(' ');
        }
        switch (instr->type) {
            case MICRO_INSTR_SET:
                printf("SET: type:%s, name:'%s'\n", str_type[instr->set.type], instr->set.reg_name);
                print_expr(instr->set.val_expr, tab + 5);
                break;

            case MICRO_INSTR_DRSET:
                printf("DRSET: type:%s, name:'%s'\n", str_type[instr->drset.type], instr->drset.reg_name);
                print_expr(instr->set.val_expr, tab + 5);
                break;

            case MICRO_INSTR_FUN:
                printf("FUN: ret_type:%s, name:'%s'\n", str_type[instr->fun.ret_type], instr->fun.name);
                printf("     args:\n");
                for (size_t j = 0; j < instr->fun.args.size; j++) {
                    micro_instruction_fun_arg_t *arg = sct_vector_get(&instr->fun.args, j);
                    printf("       type:%s, name:'%s'\n", str_type[arg->type], arg->name);
                }
                printf("     body:\n");
                print_instructions(&instr->fun.body, tab + 7);
                break;

            case MICRO_INSTR_RET:
                printf("RET\n");
                print_expr(instr->ret.val_expr, tab + 5);
                break;

            case MICRO_INSTR_CALL:
                printf("CALL: res_reg:'%s', fun:'%s'\n", instr->call.ret_reg_name, instr->call.fun_name);
                printf("       args:\n");
                for (size_t j = 0; j < instr->call.arg_exprs.size; j++) {
                    print_expr(*(micro_token_t**)sct_vector_get(&instr->call.arg_exprs, j), tab + 6);
                    puts("");
                }
                break;

            case MICRO_INSTR_LBL:
                printf("LBL '%s'\n", instr->lbl.name);
                break;

            case MICRO_INSTR_GOTO:
                printf("GOTO '%s'\n", instr->goto_lbl.lbl);
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
} asm_fmt_t;

#define A(name, k1, s1, k2, s2)  { name, k1, s1, k2, s2 }
#define A1(name, k1, s1)         { name, k1, s1, 0, 0 }
#define A0(name)                 { name, 0, 0, 0, 0 }

static const char *reg8[]  = { "al", "cl", "dl", "bl", "ah", "ch", "dh", "bh" };
static const char *reg16[] = { "ax", "cx", "dx", "bx", "sp", "bp", "si", "di" };
static const char *reg32[] = { "eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi" };

static const asm_fmt_t asm_tbl[] = {
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

    [MICRO_ASM386_INSTR_SETZ_R8]     = A1("setzR8",     'R', 8),
    [MICRO_ASM386_INSTR_SETNZ_R8]    = A1("setnzR8",    'R', 8),
    [MICRO_ASM386_INSTR_SETG_R8]     = A1("setgR8",     'R', 8),
    [MICRO_ASM386_INSTR_SETGE_R8]    = A1("setgeR8",    'R', 8),
    [MICRO_ASM386_INSTR_SETL_R8]     = A1("setlR8",     'R', 8),
    [MICRO_ASM386_INSTR_SETLE_R8]    = A1("setleR8",    'R', 8),

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

    [MICRO_ASM386_INSTR_JZ_S32]      = A1("jzS32",      'V', 0),
    [MICRO_ASM386_INSTR_JNZ_S32]     = A1("jnzS32",     'V', 0),
    [MICRO_ASM386_INSTR_JMP_S32]     = A1("jmpS32",     'V', 0),

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

void put_asm(micro_codegen_t *codegen)
{
    for (size_t i = 0; i < codegen->asm_instrs.size; i++) {
        micro_asm386_instruction_t *instr = sct_vector_get(&codegen->asm_instrs, i);
        if (instr->opcode == MICRO_ASM386_INSTR_NONE ||
            instr->opcode >= sizeof(asm_tbl) / sizeof(asm_tbl[0])) {
            continue;
        }

        if (instr->opcode == MICRO_ASM386_INSTR_LBL) {
            printf("%s:\n", instr->operand1.lbl_name);
            continue;
        }
        if (instr->opcode == MICRO_ASM386_INSTR_MOV_R32L32) {
            printf("movR32L32 %s, %s\n", reg32[instr->operand1.reg], instr->operand2.lbl_name);
            continue;
        }
        if (instr->opcode == MICRO_ASM386_INSTR_CALL_L32) {
            printf("callL32 %s\n", instr->operand1.lbl_name);
            continue;
        }

        const asm_fmt_t *fmt = &asm_tbl[instr->opcode];
        printf("%s", fmt->name);

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

int main(int argc, char **argv)
{
    micro_args_t *args = (micro_args_t*)malloc(sizeof(micro_args_t));

    *args = micro_args_parse(argc, argv);

    if (args->inputfile[0] == 0) {
        puts("Error: Expected input file name");
        return 1;
    }
    if (args->outfile[0] == 0) {
        puts("Error: Expected output file name");
        return 1;
    }

    char *text = (char*)malloc(sizeof(char) * MICRO_MAX_INPUT_CODE_SIZE);
    size_t text_size = 0;

    FILE *infile = fopen(args->inputfile, "r");
    if (!infile) {
        puts("Error: Input file not be opening");
        return 1;
    }
    char c = 0;
    while ((c = getc(infile)) != EOF) {
        if (text_size >= MICRO_MAX_INPUT_CODE_SIZE) {
            puts("Error: Input file to large for reading");
            fclose(infile);
            return 1;
        }
        text[text_size++] = c;
    }
    fclose(infile);

    if (args->stop_at == STOPAFTER_FILE_READ) return 0;

    micro_init();
        sct_vector_t toks;
        sct_vector_init(&toks, sizeof(micro_token_t));
        micro_tokenize(text, text_size, &toks);

        free(text);

        for (size_t i = 0; i < micro_err_stk_size; i++) {
            put_err(args->inputfile, micro_err_stk[i]);
        }
        if (micro_err_stk_size) {
            return 2;
        }

        if (args->toks_put) {
            for (size_t i = 0; i < toks.size; i++) {
                micro_token_t tok = *(micro_token_t*)sct_vector_get(&toks, i);
                printf("%lu. ", i);
                print_tok(tok);
            }
        }

        if (args->stop_at == STOPAFTER_LEXER) return 0;

        micro_instrgen_t instrgen;
        micro_instrgen_init(&instrgen, &toks);
            micro_instrgen_gen(&instrgen);

            for (size_t i = 0; i < micro_err_stk_size; i++) {
                put_err(args->inputfile, micro_err_stk[i]);
            }
            if (micro_err_stk_size) {
                return 3;
            }
            
            if (args->instrs_put) {
                print_instructions(&instrgen.instructions, 0);
            }

            if (args->stop_at == STOPAFTER_INSTRGEN) return 0;

            micro_codegen_t codegen;
            micro_codegen386_init(&codegen);
                codegen.emit(&codegen, &instrgen.instructions);
                for (size_t i = 0; i < micro_err_stk_size; i++) {
                    put_err(args->inputfile, micro_err_stk[i]);
                }

                if (args->asm_put) {
                    put_asm(&codegen);
                }

                micro_asm386_emit(&codegen.asm_instrs, &codegen.outbuf);

                FILE *outfile = fopen(args->outfile, "wb");
                fwrite(codegen.outbuf.data, sizeof(u8), codegen.outbuf.size, outfile);
                fclose(outfile);
            micro_codegen386_deinit(&codegen);
        micro_instrgen_deinit(&instrgen);
        sct_vector_deinit(&toks);
    micro_deinit();

    free(args);

    return 0;
}