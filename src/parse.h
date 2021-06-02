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
        BINARY, UNARY, LITERAL, GROUP
    } type;

    union
    {
        Token literal;

        struct
        {
            Token operator;
            struct Expr *left;
            struct Expr *right;
        } binary;

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
