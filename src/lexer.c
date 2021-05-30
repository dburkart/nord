/*
 * Copyright (c) 2021, Dana Burkart <dana.burkart@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "lexer.h"

bool is_reserved(char c)
{
    if (c == '(' || c == ')' || c == ':' || c == ',')
        return true;
    return false;
}

bool is_whitespace(char c)
{
    if (c == ' ' || c == '\t' || c == '\n')
        return true;
    return false;
}

int match_var(const char *c, TokenList *list)
{
    if (*c != 'v') return 0;
    if (*(c+1) != 'a') return 0;
    if (*(c+2) != 'r') return 0;

    Token t = { VAR };
    token_list_add(list, t);

    return 3;
}

int match_fn(const char *c, TokenList *list)
{
    if (*c != 'f') return 0;
    if (*(c+1) != 'n') return 0;

    Token t = { FUNCTION };
    token_list_add(list, t);

    return 2;
}

int match_identifier(const char *c, TokenList *list)
{
    int len = 0;

    // Identifiers must start with a letter, so we do some bounds checking
    if (*c < 'A')
        return 0;

    if (*c > 'Z' && *c < 'a')
        return 0;

    if (*c > 'z')
        return 0;

    while (*c != ' ' && *c != '\t' && *c != '\n' && *c != '\0')
    {
        if (is_reserved(*c))
            break;

        len = len + 1;
        c = c + 1;
    }

    Token t = { IDENTIFIER };
    token_list_add(list, t);

    return len;
}

int match_numeral(const char *c, TokenList *list)
{
    int len = 0;

    while (*c != ' ' && *c != '\t' && *c != '\n' && *c != '\0')
    {
        if (*c < '0' || *c > '9')
            return 0;

        len = len + 1;
        c = c + 1;
    }

    Token t = { NUMERAL };
    token_list_add(list, t);

    return len;
}

int match_right_arrow(const char *c, TokenList *list)
{
    if (*c != '-') return 0;
    if (*(c+1) != '>') return 0;

    Token t = { R_ARROW };
    token_list_add(list, t);

    return 2;
}

TokenList scan(char *input)
{
    // We just create an arbitrarily-sized token list to begin with
    TokenList tokens = token_list_create(2);

    int pos = 0;
    char *c = input;
    while (*c != '\0')
    {
        int start = pos;
        int advance = 0;
        bool token_added = true;
        bool error = false;
        Token t;
        switch (*c)
        {
            case ' ':
            case '\t':
                advance = 1;
                token_added = false;
                break;
            case '\n':
                t.type = EOL;
                token_list_add(&tokens, t);
                advance = 1;
                break;
            case '=':
                t.type = EQUAL;
                token_list_add(&tokens, t);
                advance = 1;
                break;
            case '(':
                t.type = L_PAREN;
                token_list_add(&tokens, t);
                advance = 1;
                break;
            case ')':
                t.type = R_PAREN;
                token_list_add(&tokens, t);
                advance = 1;
                break;
            case ':':
                t.type = COLON;
                token_list_add(&tokens, t);
                advance = 1;
                break;
            case ',':
                t.type = COMMA;
                token_list_add(&tokens, t);
                advance = 1;
                break;
            case '-':
                advance = match_right_arrow(c, &tokens);
                if (advance)
                    break;
            case 'f':
                advance = match_fn(c, &tokens);
                if (advance)
                    break;
            case 'v':
                advance = match_var(c, &tokens);
                if (advance)
                    break;
            default:
                advance = match_identifier(c, &tokens);
                if (advance)
                    break;

                advance = match_numeral(c, &tokens);
                if (advance)
                    break;

                // We didn't match anything, so consume until we hit whitespace, and set error = true
                advance = 1;
                while (!is_whitespace(*(c+advance)))
                {
                    advance = advance + 1;
                }
                error = true;
        }

        // Advance our position
        pos = pos + advance;

        // Record the location of the token
        if (token_added)
        {
            tokens.tokens[tokens.size - 1].start = start;
            tokens.tokens[tokens.size - 1].end = pos;
        }

        if (error)
        {
            printf("Unknown token found while scanning: \"%.*s\"\n", advance, c);
            exit(2);
        }

        c += advance;
    }

    return tokens;
}

