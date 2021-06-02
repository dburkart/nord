/*
 * Copyright (c) 2021, Dana Burkart <dana.burkart@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <stdio.h>

#include "token.h"

TokenList token_list_create(size_t capacity)
{
    TokenList list;

    list.size = 0;
    list.capacity = capacity;
    if (capacity)
    {
        list.tokens = calloc(capacity, sizeof(Token));
    }
    else
    {
        list.tokens = NULL;
    }

    return list;
}

void token_list_destroy(TokenList list)
{
    if (list.tokens)
    {
        free(list.tokens);
    }
}

void token_list_add(TokenList *list, Token t)
{
    if (list->size == list->capacity)
    {
        // Grow our capacity by 2x
        list->capacity = list->capacity * 2;
        list->tokens = realloc(list->tokens, sizeof(Token) * list->capacity);
    }

    list->tokens[list->size] = t;
    list->size = list->size + 1;
}

const char *token_name(Token t)
{
    switch(t.type)
    {
        // Single character tokens
        case EQUAL:
            return "EQUAL";
        case L_PAREN:
            return "L_PAREN";
        case R_PAREN:
            return "R_PAREN";
        case COLON:
            return "COLON";
        case COMMA:
            return "COMMA";
        case EOL:
            return "EOL";

        // COMPARATORS
        case BANG:
            return "BANG";
        case BANG_EQUAL:
            return "BANG_EQUAL";
        case EQUAL_EQUAL:
            return "EQUAL_EQUAL";

        // MATH
        case MINUS:
            return "MINUS";
        case PLUS:
            return "PLUS";
        case ASTERISK:
            return "ASTERISK";
        case SLASH:
            return "SLASH";

        // Arrows
        case R_ARROW:
            return "R_ARROW";

        // Literals
        case IDENTIFIER:
            return "IDENTIFIER";
        case STRING:
            return "STRING";
        case NUMBER:
            return "NUMBER";

        // Keywords
        case TRUE:
            return "TRUE";
        case FALSE:
            return "FALSE";
        case NIL:
            return "NIL";
        case VAR:
            return "VAR";
        case FUNCTION:
            return "FUNCTION";
        case INVALID:
            return "INVALID";

        // EOF
        case EOF_CHAR:
            return "EOF";
        default:
            break;
    }
    // This shouldn't be reachable
    return "UNKNOWN";
}

void token_list_print(TokenList list)
{
    for (int i = 0; i < list.size; i++)
    {
        Token t = list.tokens[i];
        printf("[%s:%d-%d]\n", token_name(t), t.start, t.end);
    }

    printf("\n");
}
