/*
 * Copyright (c) 2021, Dana Burkart <dana.burkart@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

/*
 * This test runner takes any number of files as inputs, and prints out a
 * single scanned token per line.
 */

#include <stdio.h>

#include "compiler/lex.h"

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

        token_list_t list = scan_input(argv[i], input);
        token_list_print(list);
        token_list_destroy(list);

        free(input);
    }

done:
    fflush(stdout);
    fflush(stderr);
    return status;
}
