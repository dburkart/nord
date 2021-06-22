/*
 * Copyright (c) 2021, Dana Burkart <dana.burkart@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"

char *format_error(const char *listing_name, const char *listing, const char *str, location_t loc)
{
    char *error;
    int lineno = 1, position = 1, nl = 0;
    for (int i = 0; i < loc.start; i++)
    {
        if (listing[i] == '\n')
        {
            lineno++;
            position = i + 1;
        }
    }

    for (int i = position; nl == 0; i++)
    {
        if (listing[i] == '\n')
            nl = i;
    }

    char *line = malloc(nl - position + 2);
    memcpy(line, listing + position - 1, nl - position + 1);
    line[nl - position + 1] = 0;

    char *spacing;

    if (loc.start > position)
    {
        spacing = malloc(loc.start - position + 2);
        for (int i = 0; i <= loc.start - position; i++)
        {
            spacing[i] = ' ';
        }
        spacing[loc.start - position + 1] = 0;
    }
    else
    {
        spacing = "";
    }

    char *caret_pointer = malloc(loc.end - loc.start + 2);
    caret_pointer[0] = '^';
    for (int i = 1; i < loc.end - loc.start; i++)
    {
        caret_pointer[i] = '~';
    }
    caret_pointer[loc.end - loc.start] = 0;

    asprintf(&error,
             "%s:%d:%llu: %s\n\n%s\n%s%s Found here.\n",
             listing_name,
             lineno,
             loc.start - (position - 1),
             str,
             line,
             spacing,
             caret_pointer
            );
    return error;
}
