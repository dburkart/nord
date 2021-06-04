/*
 * Copyright (c) 2021, Dana Burkart <dana.burkart@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef PARSE_H
#define PARSE_H

#include "token.h"
#include "lex.h"

typedef struct Expr
{
    enum
    {
        ASSIGN, BINARY, DECLARE, UNARY, LITERAL, GROUP
    } type;

    union
    {
        Token literal;

        struct
        {
            Token name;
            struct Expr* value;
        } assign;

        struct
        {
            Token operator;
            struct Expr *left;
            struct Expr *right;
        } binary;

        struct
        {
            Token var_type;
            Token name;
            struct Expr *initial_value;            // NULL if not initialized
        } declare;

        struct
        {
            Token operator;
            struct Expr *operand;
        } unary;

        struct Expr *group;
    } op;
} AST;

AST *parse(ScanContext *);

void print_ast(ScanContext *, AST *);

#endif
