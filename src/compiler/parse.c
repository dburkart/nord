/*
 * Copyright (c) 2021, Dana Burkart <dana.burkart@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <assert.h>
#include <stdio.h>

#include "parse.h"
#include "util/error.h"

// Forward declarations
ast_t *statement_block(scan_context_t *);
ast_t *statement_list(scan_context_t *);
ast_t *statement(scan_context_t *);
ast_t *function_decl(scan_context_t *);
ast_t *variable_decl(scan_context_t *);
ast_t *expression_list(scan_context_t *);
ast_t *expression(scan_context_t *);
ast_t *equality(scan_context_t *);
ast_t *assignment(scan_context_t *);
ast_t *comparison(scan_context_t *);
ast_t *term(scan_context_t *);
ast_t *term_md(scan_context_t *);
ast_t *unary(scan_context_t *);
ast_t *primary(scan_context_t *);
ast_t *function_call(scan_context_t *);

ast_t *parse(scan_context_t *context)
{
    return statement_list(context);
}

void print_ast_internal(scan_context_t *context, ast_t *ast, int indent)
{
    char *token_val;

    if (ast == NULL)
        return;

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
            token_val = ast->op.assign.name;
            printf("ASSIGN(IDENTIFIER) -> %s\n", token_val);
            print_ast_internal(context, ast->op.assign.value, indent + 2);
            break;
        case BINARY:
            printf("BINARY(%s)\n", token_name(ast->op.binary.operator));
            print_ast_internal(context, ast->op.binary.left, indent + 2);
            print_ast_internal(context, ast->op.binary.right, indent + 2);
            break;
        case DECLARE:
            token_val = ast->op.declare.name;
            printf("DECLARE(IDENTIFIER) -> %s\n", token_val);
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
            printf("LITERAL(%s) -> %s\n", token_name(ast->op.literal.token), ast->op.literal.value);
            break;
        case GROUP:
            printf("GROUP\n");
            print_ast_internal(context, ast->op.group, indent + 2);
            break;
        case STATEMENT_LIST:
            printf("STMT LIST\n");
            for (int i = 0; i < ast->op.list.size; i++)
            {
                print_ast_internal(context, ast->op.list.items[i], indent + 2);
            }
            break;
        case FUNCTION_DECL:
            printf("FUNCTION_DECL(%s)\n", ast->op.fn.name);
            if (ast->op.fn.args != NULL)
            {
                print_ast_internal(context, ast->op.fn.args, indent + 2);
            }
            print_ast_internal(context, ast->op.fn.body, indent + 2);
            break;
        case FUNCTION_CALL:
            printf("CALL_FN(%s)\n", ast->op.call.name);
            if (ast->op.call.args != NULL)
            {
                print_ast_internal(context, ast->op.call.args, indent + 2);
            }
            break;
        case EXPRESSION_LIST:
            printf("ARGUMENTS\n");
            for (int i = 0; i < ast->op.list.size; i++)
            {
                print_ast_internal(context, ast->op.list.items[i], indent + 2);
            }
            break;

    }
}

void print_ast(scan_context_t *context, ast_t *ast)
{
    print_ast_internal(context, ast, 0);
}

ast_t* make_assign_expr(char *name, ast_t *value)
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

ast_t *make_declare_expr(token_t var_type, char *name, ast_t *initial_value)
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
    literal_expr->op.literal.token = literal;

    return literal_expr;
}

ast_t *make_group_expr(ast_t *expr)
{
    ast_t *group_expr = (ast_t *)malloc(sizeof(ast_t));
    group_expr->type = GROUP;
    group_expr->op.group = expr;

    return group_expr;
}

ast_t *make_fn_expr(char *name, ast_t *args, ast_t *body)
{
    ast_t *fn_expr = (ast_t *)malloc(sizeof(ast_t));
    fn_expr->type = FUNCTION_DECL;
    fn_expr->op.fn.name = name;
    fn_expr->op.fn.args = args;
    fn_expr->op.fn.body = body;
    return fn_expr;
}

ast_t *make_call_expr(char *name, ast_t *args)
{
    ast_t *call_expr = (ast_t *)malloc(sizeof(ast_t));
    call_expr->type = FUNCTION_CALL;
    call_expr->op.call.name = name;
    call_expr->op.call.args = args;
    return call_expr;
}

// List handling
ast_t *make_list_expr(size_t capacity)
{
    ast_t *list_expr = (ast_t *)malloc(sizeof(ast_t));
    list_expr->type = STATEMENT_LIST;
    list_expr->op.list.size = 0;
    list_expr->op.list.capacity = capacity;
    list_expr->op.list.items = (ast_t **)malloc(sizeof(ast_t) * capacity);
    return list_expr;
}

void list_expr_append(ast_t *list, ast_t *item)
{
    if (list->op.list.size >= list->op.list.capacity - 1)
    {
        list->op.list.capacity *= 2;
        list->op.list.items = (ast_t **)realloc(list->op.list.items, sizeof(ast_t) * list->op.list.capacity);
    }

    list->op.list.items[list->op.list.size++] = item;
}

ast_t *statement_block(scan_context_t* context)
{
    ast_t *left;

    // TODO: Proper error handling
    assert(accept(context).type == L_BRACE);

    while (peek(context).type == EOL)
        accept(context);

    left = statement_list(context);

    // TODO: Proper error handling
    assert(left != NULL);
    assert(accept(context).type == R_BRACE);

    return left;
}

ast_t *statement_list(scan_context_t* context)
{
    ast_t *statements = make_list_expr(10);

    ast_t *current = statement(context);

    if (current == NULL)
    {
        return statements;
    }

    list_expr_append(statements, current);

    while (peek(context).type != EOF_CHAR && peek(context).type != EOF_CHAR)
    {
        // If the next token is and EOL, consume it
        if (peek(context).type == EOL)
            accept(context);

        // Now pull off the next statement
        current = statement(context);

        if (current != NULL)
            list_expr_append(statements, current);
        else
            break;
    }

    return statements;
}

ast_t *statement(scan_context_t *context)
{
    ast_t *left = variable_decl(context);

    if (left == NULL)
        left = expression(context);

    if (left == NULL)
        left = function_decl(context);

    if (peek(context).type == EOL)
    {
        accept(context);
        return left;
    }

    if (peek(context).type == EOF_CHAR)
        return left;

    return left;
}

ast_t *function_decl(scan_context_t *context)
{
    ast_t *left, *args = NULL, *body;
    char *name;

    if (peek(context).type != FN)
        return NULL;

    accept(context);

    if (peek(context).type == IDENTIFIER)
    {
        name = token_value(context, accept(context));
    }
    else
    {
        name = "__anonymous";
    }

    if (peek(context).type == L_PAREN)
    {
        accept(context);
        args = expression_list(context);
        // TODO: Handle error
        assert(accept(context).type == R_PAREN);
    }

    body = statement_block(context);
    // TODO: Handle error
    assert(body != NULL);

    left = make_fn_expr(name, args, body);

    return left;
}

ast_t *variable_decl(scan_context_t *context)
{
    ast_t *left;

    if (peek(context).type != VAR)
        return NULL;

    token_t var_type = accept(context);

    if (peek(context).type != IDENTIFIER)
    {
        char *error;
        token_t invalid = accept(context);
        location_t loc = {invalid.start, invalid.end};
        asprintf(&error, "Expected identifier in declaration, but found \"%s\".", token_value(context, invalid));
        printf("%s", format_error(context->name, context->buffer, error, loc));
        exit(1);
    }

    token_t name = accept(context);
    ast_t *right = NULL;

    if (peek(context).type == EQUAL)
    {
        accept(context);
        right = expression(context);
    }

    left = make_declare_expr(var_type, token_value(context, name), right);
    left->location.start = var_type.start;
    left->location.end = (right) ? right->location.end : name.end;

    return left;
}

ast_t *expression_list(scan_context_t *context)
{
    ast_t *expr = expression(context);

    if (expr == NULL)
        return NULL;

    ast_t *left = make_list_expr(10);

    list_expr_append(left, expr);
    left->type = EXPRESSION_LIST;

    while (peek(context).type == COMMA)
    {
        // Pull off the comma
        accept(context);

        expr = expression(context);
        // TODO: Handle error
        assert(expr != NULL);
        list_expr_append(left, expr);
    }

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
    left = make_assign_expr(token_value(context, name), value);
    left->location.start = name.start;
    left->location.end = name.end;

    return left;
}

ast_t *equality(scan_context_t *context)
{
    ast_t *left = comparison(context);

    while (match(context, 2, BANG_EQUAL, EQUAL_EQUAL))
    {
        token_t operator = accept(context);
        ast_t *right = comparison(context);
        ast_t *new_left = make_binary_expr(left, operator, right);
        new_left->location.start = left->location.start;
        new_left->location.end = right->location.end;
        left = new_left;
    }

    return left;
}

ast_t *comparison(scan_context_t *context)
{
    ast_t *left = term(context);

    while (match(context, 4, GREATER, GREATER_EQUAL, LESS, LESS_EQUAL))
    {
        token_t operator = accept(context);
        ast_t *right = term(context);
        ast_t *new_left = make_binary_expr(left, operator, right);
        new_left->location.start = left->location.start;
        new_left->location.end = right->location.end;
        left = new_left;
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
        ast_t *new_left = make_binary_expr(left, operator, right);
        new_left->location.start = left->location.start;
        new_left->location.end = right->location.end;
        left = new_left;
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
        ast_t *new_left = make_binary_expr(left, operator, right);
        new_left->location.start = left->location.start;
        new_left->location.end = right->location.end;
        left = new_left;
    }

    return left;
}

ast_t *unary(scan_context_t *context)
{
    if (match(context, 2, BANG, MINUS))
    {
        token_t operator = accept(context);
        ast_t *operand = unary(context);
        ast_t *unary = make_unary_expr(operator, operand);
        unary->location.start = operator.start;
        unary->location.end = operand->location.end;
        return unary;
    }
    else
    {
        ast_t *p = primary(context);
        if (p == NULL && peek(context).type == INVALID)
        {
            char *error;
            location_t loc = {peek(context).start, peek(context).end};
            asprintf(&error, "Unexpected token. Expected keyword, number, string, or identifier, but found \"%s\"", token_value(context, peek(context)));
            printf("%s", format_error(context->name, context->buffer, error, loc));
            exit(1);
        }
        return p;
    }
}

ast_t *primary(scan_context_t *context)
{
    ast_t *left = function_call(context);
    if (left != NULL)
        return left;

    if (match(context, 6, IDENTIFIER, NUMBER, FLOAT, STRING, TRUE, FALSE, NIL))
    {
        token_t tok = accept(context);
        ast_t *literal = make_literal_expr(tok);
        literal->op.literal.value = token_value(context, literal->op.literal.token);
        literal->location.start = tok.start;
        literal->location.end = tok.end;
        return literal;
    }

    if (peek(context).type == L_PAREN)
    {
        // Consume the parenthesis
        token_t paren = accept(context);

        ast_t *expr = expression(context);
        expr->location.start = paren.start;
        paren = accept(context);
        expr->location.end = paren.end;

        if (paren.type != R_PAREN)
        {
            char *error;
            location_t loc = {paren.start, paren.end};
            asprintf(&error, "Mismatched parenthesis. Expected \")\", but found \"%s\".", token_value(context, paren));
            printf("%s", format_error(context->name, context->buffer, error, loc));
            exit(1);
        }

        return make_group_expr(expr);
    }

    return NULL;
}

ast_t *function_call(scan_context_t *context)
{
    ast_t *left;
    ast_t *args;
    char *fn_name;

    if (peek(context).type != IDENTIFIER)
        return NULL;

    fn_name = token_value(context, accept(context));

    if (peek(context).type != L_PAREN)
    {
        backup(context);
        return NULL;
    }

    accept(context);

    args = expression_list(context);

    // TODO: Error handling
    assert(accept(context).type == R_PAREN);

    left = make_call_expr(fn_name, args);

    return left;
}
