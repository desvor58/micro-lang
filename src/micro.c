/************************************************
 *               Micro IR language
 *                   by Desvor
 * 
 *    Language created for Simple C Compiler,
 *     you can use it for your own compiers.
 *     All documentation in `docs` directory
 **********************************************/

#include <stdio.h>

#include "args.h"
#include "types.h"
#include "lexer.h"

#include "codegen/codegen_386.h"

int main(int argc, char **argv)
{
    args_t *args = (args_t*)malloc(sizeof(args_t));
    *args = args_parse(argc, argv);

    if (args->inputfile[0] == 0) {
        puts("Error: Expected input file name");
        return 1;
    }
    if (args->outfile[0] == 0) {
        puts("Error: Expected output file name");
        return 1;
    }

    char *text = (char*)malloc(sizeof(char) * MAX_INPUT_CODE_SIZE);
    size_t text_size = 0;

    FILE *infile = fopen(args->inputfile, "r");
    if (!infile) {
        puts("Error: Input file not be opening");
        return 1;
    }
    char c = 0;
    while ((c = getc(infile)) != EOF) {
        if (text_size >= MAX_INPUT_CODE_SIZE) {
            puts("Error: Input file to large for reading");
            fclose(infile);
            return 1;
        }
        text[text_size++] = c;
    }
    fclose(infile);

    lexer_init();
        lexing(text, text_size);

        for (size_t i = 0; i < lexer_err_stk_size; i++) {
            printf("Error:%s:%lu:%lu: %s",
                     args->inputfile,
                     lexer_err_stk[i].line_ref,
                     lexer_err_stk[i].chpos_ref,
                     lexer_err_stk[i].msg);
        }
        if (lexer_err_stk_size) {
            return 2;
        }

        if (args->flags & AF_TOKS_PUT) {
            for (size_t i = 0; i < toks_size; i++) {
                printf("%lu. %lu:%lu type:%u, val:%s\n",
                        i,
                        toks[i].line_ref,
                        toks[i].chpos_ref,
                        toks[i].type,
                        toks[i].val);
            }
        }

        codegen_368_init();
            codegen_368();

            for (size_t i = 0; i < codegen_368_err_stk_size; i++) {
                printf("Error:%s:%lu:%lu: %s\n",
                        args->inputfile,
                        codegen_368_err_stk[i].line_ref,
                        codegen_368_err_stk[i].chpos_ref,
                        codegen_368_err_stk[i].msg);
            }
            if (codegen_368_err_stk_size) {
                return 2;
            }

            FILE *outfile = fopen(args->outfile, "w");
            fwrite(outbuf->str, sizeof(char), outbuf->size, outfile);
            fclose(outfile);
        codegen_368_delete();
    lexer_delete();

    return 0;
}