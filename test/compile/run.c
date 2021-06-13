/*
 * Copyright (c) 2021, Dana Burkart <dana.burkart@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#define _BSD_SRC

#include <stdio.h>

#include "machine/binary.h"
#include "machine/disassemble.h"
#include "machine/vm.h"
#include "compile.h"
#include "lex.h"
#include "parse.h"

int main(int argc, char *argv[])
{
    int status = 0;

    if (argc == 1)
    {
        printf("Usage: %s <file-1> <file-2> ...\n", argv[0]);
        goto done;
    }

    for (int i = 1; i < argc; i++)
    {
        FILE *fp = fopen(argv[i], "r");

        fseek(fp, 0, SEEK_END);
        long fsize = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        char *input = malloc(fsize + 1);
        fread(input, 1, fsize, fp);
        fclose(fp);

        input[fsize] = 0;

        scan_context_t context;
        context.buffer = input;
        context.position = 0;

        ast_t *syntax_tree = parse(&context);
        binary_t *binary = compile(syntax_tree);

        char *listing = disassemble(binary);
        printf("%s", listing);

        free(listing);
        free(input);
    }

done:
    fflush(stdout);
    fflush(stderr);
    return status;
}
