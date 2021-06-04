/*
 * Copyright (c) 2021, Dana Burkart <dana.burkart@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <assert.h>
#include <stdio.h>

#include "parse.h"

// Forward declarations
AST *declaration(ScanContext *);
AST *statement(ScanContext *);
AST *variable_decl(ScanContext *);
AST *expression(ScanContext *);
AST *equality(ScanContext *);
AST *assignment(ScanContext *);
AST *comparison(ScanContext *);
AST *term(ScanContext *);
AST *term_md(ScanContext *);
AST *unary(ScanContext *);
AST *primary(ScanContext *);

AST *parse(ScanContext *context)
{
    return declaration(context);
}

void print_ast_internal(ScanContext *context, AST *ast, int indent)
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

void print_ast(ScanContext *context, AST *ast)
{
    print_ast_internal(context, ast, 0);
}

AST* make_assign_expr(Token name, AST *value)
{
    AST *assign_expr = (AST *)malloc(sizeof(AST));
    assign_expr->type = ASSIGN;
    assign_expr->op.assign.name = name;
    assign_expr->op.assign.value = value;

    return assign_expr;
}

AST *make_binary_expr(AST *left, Token operator, AST *right)
{
    AST *binary_expr = (AST *)malloc(sizeof(AST));
    binary_expr->type = BINARY;
    binary_expr->op.binary.operator = operator;
    binary_expr->op.binary.left = left;
    binary_expr->op.binary.right = right;

    return binary_expr;
}

AST *make_declare_expr(Token var_type, Token name, AST *initial_value)
{
    AST *declare_expr = (AST *)malloc(sizeof(AST));
    declare_expr->type = DECLARE;
    declare_expr->op.declare.var_type = var_type;
    declare_expr->op.declare.name = name;
    declare_expr->op.declare.initial_value = initial_value;

    return declare_expr;
}

AST *make_unary_expr(Token operator, AST *operand)
{
    AST *unary_expr = (AST *)malloc(sizeof(AST));
    unary_expr->type = UNARY;
    unary_expr->op.unary.operator = operator;
    unary_expr->op.unary.operand = operand;

    return unary_expr;
}

AST *make_literal_expr(Token literal)
{
    AST *literal_expr = (AST *)malloc(sizeof(AST));
    literal_expr->type = LITERAL;
    literal_expr->op.literal = literal;

    return literal_expr;
}

AST *make_group_expr(AST *expr)
{
    AST *group_expr = (AST *)malloc(sizeof(AST));
    group_expr->type = GROUP;
    group_expr->op.group = expr;

    return group_expr;
}

AST *declaration(ScanContext *context)
{
    AST *left = variable_decl(context);

    if (left == NULL)
        left = statement(context);

    return left;
}

AST *statement(ScanContext *context)
{
    AST *left = expression(context);

    if (peek(context).type == EOL)
    {
        accept(context);
        return left;
    }

    if (peek(context).type == EOF_CHAR)
        return left;

    return NULL;
}

AST *variable_decl(ScanContext *context)
{
    AST *left;

    if (peek(context).type != VAR)
        return NULL;

    Token var_type = accept(context);

    if (peek(context).type != IDENTIFIER)
    {
        backup(context);
        return NULL;
    }

    Token name = accept(context);
    AST *right = NULL;

    if (peek(context).type == EQUAL)
    {
        accept(context);
        right = expression(context);
    }

    left = make_declare_expr(var_type, name, right);

    return left;
}

AST *expression(ScanContext *context)
{
    return assignment(context);
}

AST *assignment(ScanContext *context)
{
    AST *left = NULL;
    Token name;

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

    AST *value = expression(context);
    left = make_assign_expr(name, value);

    return left;
}

AST *equality(ScanContext *context)
{
    AST *left = comparison(context);

    while (match(context, 2, BANG_EQUAL, EQUAL_EQUAL))
    {
        Token operator = accept(context);
        AST *right = comparison(context);
        left = make_binary_expr(left, operator, right);
    }

    return left;
}

AST *comparison(ScanContext *context)
{
    AST *left = term(context);

    while (match(context, 4, GREATER, GREATER_OR_EQUAL, LESS, LESS_OR_EQUAL))
    {
        Token operator = accept(context);
        AST *right = term(context);
        left = make_binary_expr(left, operator, right);
    }

    return left;
}

AST *term(ScanContext *context)
{
    AST *left = term_md(context);

    while (match(context, 2, MINUS, PLUS))
    {
        Token operator = accept(context);
        AST *right = term_md(context);
        left = make_binary_expr(left, operator, right);
    }

    return left;
}

AST *term_md(ScanContext *context)
{
    AST *left = unary(context);

    while (match(context, 2, SLASH, ASTERISK))
    {
        Token operator = accept(context);
        AST *right = unary(context);
        left = make_binary_expr(left, operator, right);
    }

    return left;
}

AST *unary(ScanContext *context)
{
    if (match(context, 2, BANG, MINUS))
    {
        Token operator = accept(context);
        AST *operand = unary(context);
        return make_unary_expr(operator, operand);
    }
    else
    {
        return primary(context);
    }
}

AST *primary(ScanContext *context)
{
    if (match(context, 6, IDENTIFIER, NUMBER, STRING, TRUE, FALSE, NIL))
    {
        return make_literal_expr(accept(context));
    }

    if (peek(context).type == L_PAREN)
    {
        // Consume the parenthesis
        accept(context);

        AST *expr = expression(context);

        Token r_paren = accept(context);
        // FIXME
        assert(r_paren.type == R_PAREN);

        return make_group_expr(expr);
    }

    return NULL;
}
