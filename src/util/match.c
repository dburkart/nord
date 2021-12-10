/*
 * Copyright (c) 2021, Dana Burkart <dana.burkart@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <string.h>

#include "match.h"

/*
 * Return whether or not the specified character is reserved and shouldn't be
 * allowed in an identifier.
 */
bool is_reserved(char c)
{
    switch (c)
    {
        case '(':
        case ')':
        case '{':
        case '}':
        case ':':
        case ',':
        case '*':
        case '+':
        case '/':
        case '-':
        case '=':
            return true;
    }
    return false;
}

/*
 * Return whether or not the specified character is whitespace.
 */
bool is_whitespace(char c)
{
    if (c == ' ' || c == '\t' || c == '\n')
        return true;
    return false;
}

/*
 * Return whether or not the specified character is a token-boundary.
 */
bool is_boundary(char c)
{
    return is_whitespace(c) || is_reserved(c) || c == '\0';
}

/*
 * Match a keyword
 */
int match_keyword(const char *to_match, const char *c, int len)
{
    if (memcmp(to_match, c, len) == 0 && is_boundary(c[len]))
    {
        return len;
    }

    return 0;
}