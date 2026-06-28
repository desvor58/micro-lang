/************************************************
 *               Micro IR language
 *                   by Desvor
 * 
 *    Language created for Simple C Compiler,
 *     you can use it for your own compilers.
 *     All documentation in `docs` directory
 **********************************************/

#include <micro/micro.h>
#include <string.h>
#include <stdio.h>

typedef struct {
    char inputfile[MICRO_MAX_SYMBOL_SIZE];
    char outfile[MICRO_MAX_SYMBOL_SIZE];
    u8   toks_put   : 1;
    u8   instrs_put : 1;
} micro_args_t;


micro_args_t micro_args_parse(int argc, char **argv)
{
    micro_args_t args;
    args.inputfile[0] = 0;
    args.outfile[0] = 0;
    args.toks_put = 0;
    args.instrs_put = 0;

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            if (!strcmp(argv[i], "--output") || !strcmp(argv[i], "-o")) {
                strcpy(args.outfile, argv[++i]);
            } else
            if (!strcmp(argv[i], "--toks-put") || !strcmp(argv[i], "-Tp")) {
                args.toks_put = 1;
            }
            if (!strcmp(argv[i], "--instrs-put") || !strcmp(argv[i], "-Ip")) {
                args.instrs_put = 1;
            }
        } else {
            strcpy(args.inputfile, argv[i]);
        }
    }

    return args;
}

void put_err(char *file, size_t line, size_t chpos, char *msg)
{
    printf("Error:%s:%lu:%lu: %s\n", file, line, chpos, msg);
}

int main(int argc, char **argv)
{
    micro_args_t *args = (micro_args_t*)malloc(sizeof(micro_args_t));

    *args = micro_args_parse(argc, argv);
    // strcpy(args->inputfile, "../test.micro");
    // strcpy(args->outfile,  "../test.bin");

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

    micro_init();
        sct_vector_t toks;
        sct_vector_init(&toks, sizeof(micro_token_t));
        micro_tokenize(text, text_size, &toks);

        for (size_t i = 0; i < micro_err_stk_size; i++) {
            put_err(args->inputfile,
                    micro_err_stk[i].line_ref,
                    micro_err_stk[i].chpos_ref,
                    micro_err_stk[i].msg);
        }
        if (micro_err_stk_size) {
            return 2;
        }

        if (args->toks_put) {
            for (size_t i = 0; i < toks.size; i++) {
                micro_token_t tok = *(micro_token_t*)sct_vector_get(&toks, i);
                printf("%lu. %lu:%lu type:%s, val:%s\n",
                       i,
                       tok.line_ref,
                       tok.chpos_ref,
                       micro_token_type2str[tok.type],
                       tok.val);
            }
        }

        micro_instrgen_t instrgen;
        micro_instrgen_init(&instrgen, &toks);
            micro_instrgen_gen(&instrgen);

            for (size_t i = 0; i < micro_err_stk_size; i++) {
                put_err(args->inputfile,
                        micro_err_stk[i].line_ref,
                        micro_err_stk[i].chpos_ref,
                        micro_err_stk[i].msg);
            }
            if (micro_err_stk_size) {
                return 3;
            }
            
            for (size_t i = 0; i < instrgen.instructions.size; i++) {
                
            }
        micro_instrgen_deinit(&instrgen);
    micro_deinit();

    return 0;
}