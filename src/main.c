/************************************************
 *               Micro IR language
 *                   by Desvor
 * 
 *    Language created for Simple C Compiler,
 *     you can use it for your own compilers.
 *     All documentation in `docs` directory
 **********************************************/

#include <stdio.h>
#include "micro.h"

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
        micro_tok_vec_t toks;
        micro_create_tok_vec(&toks);
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

        if (args->flags & MICRO_AF_TOKS_PUT) {
            for (size_t i = 0; i < toks.size; i++) {
                printf("%lu. %lu:%lu type:%s, val:%s\n",
                       i,
                       toks.toks[i].line_ref,
                       toks.toks[i].chpos_ref,
                       micro_token_type2str[toks.toks[i].type],
                       toks.toks[i].val);
            }
        }

        micro_codegen_t codegen;
        micro_codegen_386_init(&codegen, &toks);
            micro_codegen_386(&codegen);

            for (size_t i = 0; i < micro_err_stk_size; i++) {
                put_err(args->inputfile,
                        micro_err_stk[i].line_ref,
                        micro_err_stk[i].chpos_ref,
                        micro_err_stk[i].msg);
            }
            if (micro_err_stk_size) {
                return 2;
            }

            FILE *outfile = fopen(args->outfile, "wb");
            fwrite(codegen.outbuf->arr, sizeof(char), codegen.outbuf->size, outfile);
            fclose(outfile);
        micro_codegen_386_deinit(&codegen);
        micro_free_tok_vec(&toks);
    micro_deinit();

    return 0;
}