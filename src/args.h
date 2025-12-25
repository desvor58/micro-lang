#ifndef MICRO_ARGS_H
#define MICRO_ARGS_H

#include <string.h>
#include "config.h"

typedef struct
{
    char   inputfile[MICRO_MAX_SYMBOL_SIZE];
    char   outfile[MICRO_MAX_SYMBOL_SIZE];
    size_t flags;
} micro_args_t;

typedef enum {
    MICRO_AF_TOKS_PUT = 1 << 0,
} micro_args_flags;

micro_args_t micro_args_parse(int argc, char **argv)
{
    micro_args_t args;
    args.inputfile[0] = 0;
    args.outfile[0] = 0;
    args.flags = 0;

    for (size_t i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            if (!strcmp(argv[i], "--output") || !strcmp(argv[i], "-o")) {
                strcpy(args.outfile, argv[++i]);
            } else
            if (!strcmp(argv[i], "--toks-put") || !strcmp(argv[i], "-T")) {
                args.flags |= MICRO_AF_TOKS_PUT;
            }
        } else {
            strcpy(args.inputfile, argv[i]);
        }
    }

    return args;
}

#endif