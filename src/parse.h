/*
 * Copyright (c) 2021, Dana Burkart <dana.burkart@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef PARSE_H
#define PARSE_H

#include "token.h"
#include "lex.h"

typedef struct expr_t
{
    enum
    {
        ASSIGN, BINARY, DECLARE, UNARY, LITERAL, GROUP
    } type;

    union
    {
        token_t literal;

        struct
        {
            token_t name;
            struct expr_t* value;
        } assign;

        struct
        {
            token_t operator;
            struct expr_t *left;
            struct expr_t *right;
        } binary;

        struct
        {
            token_t var_type;
            token_t name;
            struct expr_t *initial_value;            // NULL if not initialized
        } declare;

        struct
        {
            token_t operator;
            struct expr_t *operand;
        } unary;

        struct expr_t *group;
    } op;
} ast_t;

ast_t *parse(scan_context_t *);

void print_ast(scan_context_t *, ast_t *);

#endif
