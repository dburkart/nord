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

void token_list_print(TokenList list)
{
    for (int i = 0; i < list.size; i++)
    {
        Token t = list.tokens[i];
        switch(t.type)
        {
            // Single character tokens
            case EQUAL:
                printf("[EQUAL");
                break;
            case L_PAREN:
                printf("[L_PAREN");
                break;
            case R_PAREN:
                printf("[R_PAREN");
                break;
            case COLON:
                printf("[COLON");
                break;
            case COMMA:
                printf("[COMMA");
                break;
            case EOL:
                printf("[EOL");
                break;

            // Arrows
            case R_ARROW:
                printf("[R_ARROW");
                break;

            // Literals
            case IDENTIFIER:
                printf("[IDENTIFIER");
                break;
            case STRING:
                printf("[STRING");
                break;
            case NUMERAL:
                printf("[NUMERAL");
                break;

            // Keywords
            case VAR:
                printf("[VAR");
                break;
            case FUNCTION:
                printf("[FUNCTION");
                break;
            // EOF
            case EOF_CHAR:
                printf("[EOF");
                break;
            default:
                break;
        }
        printf(":%d-%d]\n", t.start, t.end);
    }

    printf("\n");
}
