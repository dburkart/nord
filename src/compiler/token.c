/*
 * Copyright (c) 2021, Dana Burkart <dana.burkart@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <stdio.h>

#include "token.h"

token_list_t token_list_create(size_t capacity)
{
    token_list_t list;

    list.size = 0;
    list.capacity = capacity;
    if (capacity)
    {
        list.tokens = calloc(capacity, sizeof(token_t));
    }
    else
    {
        list.tokens = NULL;
    }

    return list;
}

void token_list_destroy(token_list_t list)
{
    if (list.tokens)
    {
        free(list.tokens);
    }
}

void token_list_add(token_list_t *list, token_t t)
{
    if (list->size == list->capacity)
    {
        // Grow our capacity by 2x
        list->capacity = list->capacity * 2;
        list->tokens = realloc(list->tokens, sizeof(token_t) * list->capacity);
    }

    list->tokens[list->size] = t;
    list->size = list->size + 1;
}

const char *token_name(token_t t)
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
        case L_BRACE:
            return "L_BRACE";
        case R_BRACE:
            return "R_BRACE";
        case COLON:
            return "COLON";
        case COMMA:
            return "COMMA";
        case EOL:
            return "EOL";

        // BRANCHING
        case IF:
            return "IF";

        // COMPARATORS
        case BANG:
            return "BANG";
        case BANG_EQUAL:
            return "BANG_EQUAL";
        case EQUAL_EQUAL:
            return "EQUAL_EQUAL";
        case LESS:
            return "LESS";
        case LESS_EQUAL:
            return "LESS_EQUAL";
        case GREATER:
            return "GREATER";
        case GREATER_EQUAL:
            return "GREATER_EQUAL";

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
        case FLOAT:
            return "FLOAT";

        // Keywords
        case TRUE:
            return "TRUE";
        case FALSE:
            return "FALSE";
        case NIL:
            return "NIL";
        case VAR:
            return "VAR";
        case FN:
            return "FN";
        case FOR:
            return "FOR";
        case IN:
            return "IN";
        case RETURN:
            return "RETURN";
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

void token_list_print(token_list_t list)
{
    for (int i = 0; i < list.size; i++)
    {
        token_t t = list.tokens[i];
        printf("[%s:%llu-%llu]\n", token_name(t), t.start, t.end);
    }

    printf("\n");
}
