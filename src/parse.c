/*
 * Copyright (c) 2021, Dana Burkart <dana.burkart@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <assert.h>
#include <stdio.h>

#include "parse.h"

// Forward declarations
ast_t *declaration(scan_context_t *);
ast_t *statement(scan_context_t *);
ast_t *variable_decl(scan_context_t *);
ast_t *expression(scan_context_t *);
ast_t *equality(scan_context_t *);
ast_t *assignment(scan_context_t *);
ast_t *comparison(scan_context_t *);
ast_t *term(scan_context_t *);
ast_t *term_md(scan_context_t *);
ast_t *unary(scan_context_t *);
ast_t *primary(scan_context_t *);

ast_t *parse(scan_context_t *context)
{
    return declaration(context);
}

void print_ast_internal(scan_context_t *context, ast_t *ast, int indent)
{
    char *token_val;

    for (int i = 0; i < indent; i++)
    {
        if (i > 0 && i % 2 == 0)
            printf("⋅");
        else
            printf(" ");
    }

    if (indent > 0)
        printf("↳ ");

    switch(ast->type)
    {
        case ASSIGN:
            token_val = token_value(context, ast->op.assign.name);
            printf("ASSIGN(%s) -> %s\n", token_name(ast->op.assign.name), token_val);
            free(token_val);
            print_ast_internal(context, ast->op.assign.value, indent + 2);
            break;
        case BINARY:
            printf("BINARY(%s)\n", token_name(ast->op.binary.operator));
            print_ast_internal(context, ast->op.binary.left, indent + 2);
            print_ast_internal(context, ast->op.binary.right, indent + 2);
            break;
        case DECLARE:
            token_val = token_value(context, ast->op.declare.name);
            printf("DECLARE(%s) -> %s\n", token_name(ast->op.declare.name), token_val);
            free(token_val);
            if (ast->op.declare.initial_value)
            {
                print_ast_internal(context, ast->op.declare.initial_value, indent + 2);
            }
            break;
        case UNARY:
            printf("UNARY(%s)\n", token_name(ast->op.unary.operator));
            print_ast_internal(context, ast->op.unary.operand, indent + 2);
            break;
        case LITERAL:
            token_val = token_value(context, ast->op.literal);
            printf("LITERAL(%s) -> %s\n", token_name(ast->op.literal), token_val);
            free(token_val);
            break;
        case GROUP:
            printf("GROUP\n");
            print_ast_internal(context, ast->op.group, indent + 2);
            break;
    }
}

void print_ast(scan_context_t *context, ast_t *ast)
{
    print_ast_internal(context, ast, 0);
}

ast_t* make_assign_expr(token_t name, ast_t *value)
{
    ast_t *assign_expr = (ast_t *)malloc(sizeof(ast_t));
    assign_expr->type = ASSIGN;
    assign_expr->op.assign.name = name;
    assign_expr->op.assign.value = value;

    return assign_expr;
}

ast_t *make_binary_expr(ast_t *left, token_t operator, ast_t *right)
{
    ast_t *binary_expr = (ast_t *)malloc(sizeof(ast_t));
    binary_expr->type = BINARY;
    binary_expr->op.binary.operator = operator;
    binary_expr->op.binary.left = left;
    binary_expr->op.binary.right = right;

    return binary_expr;
}

ast_t *make_declare_expr(token_t var_type, token_t name, ast_t *initial_value)
{
    ast_t *declare_expr = (ast_t *)malloc(sizeof(ast_t));
    declare_expr->type = DECLARE;
    declare_expr->op.declare.var_type = var_type;
    declare_expr->op.declare.name = name;
    declare_expr->op.declare.initial_value = initial_value;

    return declare_expr;
}

ast_t *make_unary_expr(token_t operator, ast_t *operand)
{
    ast_t *unary_expr = (ast_t *)malloc(sizeof(ast_t));
    unary_expr->type = UNARY;
    unary_expr->op.unary.operator = operator;
    unary_expr->op.unary.operand = operand;

    return unary_expr;
}

ast_t *make_literal_expr(token_t literal)
{
    ast_t *literal_expr = (ast_t *)malloc(sizeof(ast_t));
    literal_expr->type = LITERAL;
    literal_expr->op.literal = literal;

    return literal_expr;
}

ast_t *make_group_expr(ast_t *expr)
{
    ast_t *group_expr = (ast_t *)malloc(sizeof(ast_t));
    group_expr->type = GROUP;
    group_expr->op.group = expr;

    return group_expr;
}

ast_t *declaration(scan_context_t *context)
{
    ast_t *left = variable_decl(context);

    if (left == NULL)
        left = statement(context);

    return left;
}

ast_t *statement(scan_context_t *context)
{
    ast_t *left = expression(context);

    if (peek(context).type == EOL)
    {
        accept(context);
        return left;
    }

    if (peek(context).type == EOF_CHAR)
        return left;

    return NULL;
}

ast_t *variable_decl(scan_context_t *context)
{
    ast_t *left;

    if (peek(context).type != VAR)
        return NULL;

    token_t var_type = accept(context);

    if (peek(context).type != IDENTIFIER)
    {
        backup(context);
        return NULL;
    }

    token_t name = accept(context);
    ast_t *right = NULL;

    if (peek(context).type == EQUAL)
    {
        accept(context);
        right = expression(context);
    }

    left = make_declare_expr(var_type, name, right);

    return left;
}

ast_t *expression(scan_context_t *context)
{
    return assignment(context);
}

ast_t *assignment(scan_context_t *context)
{
    ast_t *left = NULL;
    token_t name;

    if (peek(context).type != IDENTIFIER)
        return equality(context);

    name = accept(context);

    if (peek(context).type != EQUAL)
    {
        free(left);
        backup(context);
        return equality(context);
    }

    // Consume the '='
    accept(context);

    ast_t *value = expression(context);
    left = make_assign_expr(name, value);

    return left;
}

ast_t *equality(scan_context_t *context)
{
    ast_t *left = comparison(context);

    while (match(context, 2, BANG_EQUAL, EQUAL_EQUAL))
    {
        token_t operator = accept(context);
        ast_t *right = comparison(context);
        left = make_binary_expr(left, operator, right);
    }

    return left;
}

ast_t *comparison(scan_context_t *context)
{
    ast_t *left = term(context);

    while (match(context, 4, GREATER, GREATER_OR_EQUAL, LESS, LESS_OR_EQUAL))
    {
        token_t operator = accept(context);
        ast_t *right = term(context);
        left = make_binary_expr(left, operator, right);
    }

    return left;
}

ast_t *term(scan_context_t *context)
{
    ast_t *left = term_md(context);

    while (match(context, 2, MINUS, PLUS))
    {
        token_t operator = accept(context);
        ast_t *right = term_md(context);
        left = make_binary_expr(left, operator, right);
    }

    return left;
}

ast_t *term_md(scan_context_t *context)
{
    ast_t *left = unary(context);

    while (match(context, 2, SLASH, ASTERISK))
    {
        token_t operator = accept(context);
        ast_t *right = unary(context);
        left = make_binary_expr(left, operator, right);
    }

    return left;
}

ast_t *unary(scan_context_t *context)
{
    if (match(context, 2, BANG, MINUS))
    {
        token_t operator = accept(context);
        ast_t *operand = unary(context);
        return make_unary_expr(operator, operand);
    }
    else
    {
        return primary(context);
    }
}

ast_t *primary(scan_context_t *context)
{
    if (match(context, 6, IDENTIFIER, NUMBER, STRING, TRUE, FALSE, NIL))
    {
        return make_literal_expr(accept(context));
    }

    if (peek(context).type == L_PAREN)
    {
        // Consume the parenthesis
        accept(context);

        ast_t *expr = expression(context);

        token_t r_paren = accept(context);
        // FIXME
        assert(r_paren.type == R_PAREN);

        return make_group_expr(expr);
    }

    return NULL;
}