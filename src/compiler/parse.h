/*
 * Copyright (c) 2021, Dana Burkart <dana.burkart@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef PARSE_H
#define PARSE_H

#include <stdint.h>

#include "token.h"
#include "lex.h"
#include "util/location.h"

// Data structure representing a node in our abstract syntax tree.
typedef struct expr_t
{
    // TODO: Namespace this enum
    enum
    {
        ASSIGN, BINARY, DECLARE, UNARY, LITERAL, GROUP, STATEMENT_LIST, FUNCTION, EXPRESSION_LIST
    } type;

    union
    {
        struct
        {
            token_t token;
            char *value;
        } literal;

        struct
        {
            char *name;
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
            char *name;
            struct expr_t *initial_value;            // NULL if not initialized
        } declare;

        struct
        {
            token_t operator;
            struct expr_t *operand;
        } unary;

        struct
        {
            size_t size;
            size_t capacity;
            struct expr_t **items;
        } list;

        struct
        {
            char *name;
            struct expr_t *args;
            struct expr_t *body;
        } fn;

        struct expr_t *group;
    } op;

    location_t location;
} ast_t;

ast_t *parse(scan_context_t *);

void print_ast(scan_context_t *, ast_t *);

#endif