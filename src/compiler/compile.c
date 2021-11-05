/*
 * Copyright (c) 2021, Dana Burkart <dana.burkart@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <stdio.h>
#include <stdlib.h>

#include "compile.h"
#include "machine/bytecode.h"
#include "machine/value.h"
#include "util/error.h"
#include "util/macros.h"

#define INSTRUCTION(...) VFUNC(INSTRUCTION, __VA_ARGS__)

#define INSTRUCTION4(OP, ARG1, ARG2, ARG3) (instruction_t){ OP, .fields={ .triplet={ARG1, ARG2, ARG3} } }
#define INSTRUCTION3(OP, ARG1, ARG2) (instruction_t){ OP, .fields={ .pair={ARG1, ARG2 } } }
#define INSTRUCTION2(OP, ARG1) INSTRUCTION3(OP, ARG1, 0)

typedef struct
{
    const char *name;
    const char *listing;
    symbol_map_t *symbols;
    binary_t *binary;
    code_block_t *current_code_block;
    uint8_t rp;
    uint64_t mp;
} compile_context_t;

typedef struct
{
    uint8_t location;
    value_type_e type;
    code_block_t *code;
} compile_result_t;

compile_result_t compile_ast(ast_t *ast, compile_context_t *context);

compile_context_t *context_create(const char *name, const char *listing)
{
    compile_context_t *context = malloc(sizeof(compile_context_t));

    context->name = name;
    context->listing = listing;
    context->symbols = symbol_map_create();
    context->binary = binary_create();
    context->binary->data = memory_create(1);
    context->binary->packaged_code = code_collection_create();
    context->binary->main_code = code_collection_create();
    context->binary->symbols = symbol_map_create();
    context->current_code_block = code_block_create();
    code_collection_add_block(context->binary->main_code, context->current_code_block);
    context->rp = 1;

    // Set up true and false
    memory_set(context->binary->data, 0, (value_t){VAL_BOOLEAN, false});
    memory_set(context->binary->data, 1, (value_t){VAL_BOOLEAN, true});

    context->mp = 2;

    return context;
}

void context_destroy(compile_context_t *context)
{
    symbol_map_destroy(context->symbols);
    // NOTE: We don't free the code block, since that is returned (and this is
    //       an internal data structure).
    free(context);
}

compile_result_t compile_literal(ast_t *ast, compile_context_t *context)
{
    value_type_e type = VAL_UNKNOWN;
    switch (ast->op.literal.token.type)
    {
        case TOK_NUMBER:
            code_block_write(context->current_code_block, INSTRUCTION(OP_LOADV, context->rp, atoi(ast->op.literal.value)));
            type = VAL_INT;
            break;

        case TOK_STRING:
            memory_set(context->binary->data, context->mp, value(ast->op.literal.value));
            code_block_write(context->current_code_block, INSTRUCTION(OP_LOAD, context->rp, context->mp));
            context->mp += 1;
            type = VAL_STRING;
            break;

        case TOK_FLOAT:
            memory_set(context->binary->data, context->mp, value(atof(ast->op.literal.value)));
            code_block_write(context->current_code_block, INSTRUCTION(OP_LOAD, context->rp, context->mp));
            context->mp += 1;
            type = VAL_FLOAT;
            break;

        case TOK_TRUE:
        case TOK_FALSE:
            code_block_write(context->current_code_block, INSTRUCTION(OP_LOAD, context->rp, (ast->op.literal.token.type == TOK_TRUE) ? 1 : 0));
            type = VAL_BOOLEAN;
            break;

        case TOK_NIL:
            code_block_write(context->current_code_block, INSTRUCTION(OP_NIL, context->rp));
            type = VAL_NIL;
            break;

        default:
            break;
    }

    return (compile_result_t){ .location=context->rp, .type=type, .code=NULL };
}

compile_result_t compile_unary(ast_t *ast, compile_context_t *context)
{
    compile_result_t right = compile_ast(ast->op.unary.operand, context);

    switch (ast->op.unary.operator.type)
    {
        case TOK_MINUS:
            code_block_write(context->current_code_block, INSTRUCTION(OP_NEGATE, context->rp, right.location));
            break;

        case TOK_BANG:
            code_block_write(context->current_code_block, INSTRUCTION(OP_NOT, context->rp, right.location));
            break;

        case TOK_RETURN:
            code_block_write(context->current_code_block, INSTRUCTION(OP_RETURN, right.location));
            break;

        default:
            ;
    }

    return (compile_result_t){ .location=context->rp, .type=right.type, .code=NULL };
}

static inline compile_result_t compile_binary_comparison(uint8_t opcode, uint8_t base_register, uint8_t condition, uint8_t left, uint8_t right)
{
    code_block_t *block = code_block_create();

    // This is the false case
    code_block_write(block, INSTRUCTION(OP_LOAD, base_register, 0));

    // This is the comparison instruction
    code_block_write(block, INSTRUCTION(opcode, condition, left, right));

    // This is the true case
    code_block_write(block, INSTRUCTION(OP_LOAD, base_register, 1));

    return (compile_result_t){ .location=base_register, .type=VAL_BOOLEAN, .code=block };
}

compile_result_t compile_binary(ast_t *ast, compile_context_t *context)
{
    compile_result_t intermediate_result = {0, VAL_ABSENT, NULL};
    compile_result_t left = compile_ast(ast->op.binary.left, context);
    context->rp += 1;
    compile_result_t right = compile_ast(ast->op.binary.right, context);
    context->rp -= 1;

    value_type_e type = VAL_UNKNOWN;
    instruction_t instruction;
    switch (ast->op.binary.operator.type)
    {
        //-- Arithmetic
        case TOK_PLUS:
            instruction = INSTRUCTION(OP_ADD, context->rp, left.location, right.location);
            type = arithmetic_cast(left.type, right.type);
            break;

        case TOK_MINUS:
            instruction = INSTRUCTION(OP_SUBTRACT, context->rp, left.location, right.location);
            type = arithmetic_cast(left.type, right.type);
            break;

        case TOK_ASTERISK:
            instruction = INSTRUCTION(OP_MULTIPLY, context->rp, left.location, right.location);
            type = arithmetic_cast(left.type, right.type);
            break;

        case TOK_SLASH:
            instruction = INSTRUCTION(OP_DIVIDE, context->rp, left.location, right.location);
            type = VAL_FLOAT;
            break;

        //-- Logic
        case TOK_AND:
            instruction = INSTRUCTION(OP_AND, context->rp, left.location, right.location);
            type = VAL_BOOLEAN;
            break;

        case TOK_OR:
            instruction = INSTRUCTION(OP_OR, context->rp, left.location, right.location);
            type = VAL_BOOLEAN;
            break;

        case TOK_EQUAL_EQUAL:
            intermediate_result = compile_binary_comparison(OP_EQUAL, context->rp + 2, 1, left.location, right.location);
            instruction = INSTRUCTION(OP_MOVE, context->rp, intermediate_result.location);
            type = VAL_BOOLEAN;
            break;

        case TOK_BANG_EQUAL:
            intermediate_result = compile_binary_comparison(OP_EQUAL, context->rp + 2, 0, left.location, right.location);
            instruction = INSTRUCTION(OP_MOVE, context->rp, intermediate_result.location);
            type = VAL_BOOLEAN;
            break;

        case TOK_LESS:
            intermediate_result = compile_binary_comparison(OP_LESSTHAN, context->rp + 2, 1, left.location, right.location);
            instruction = INSTRUCTION(OP_MOVE, context->rp, intermediate_result.location);
            type = VAL_BOOLEAN;
            break;

        case TOK_LESS_EQUAL:
            intermediate_result = compile_binary_comparison(OP_LESSTHAN, context->rp + 2, 1, left.location, right.location);
            code_block_merge(context->current_code_block, intermediate_result.code);
            code_block_free(intermediate_result.code);
            intermediate_result = compile_binary_comparison(OP_EQUAL, context->rp + 3, 1, left.location, right.location);
            instruction = INSTRUCTION(OP_OR, context->rp, context->rp + 2, context->rp + 3);
            type = VAL_BOOLEAN;
            break;

        case TOK_GREATER:
            intermediate_result = compile_binary_comparison(OP_LESSTHAN, context->rp + 2, 0, left.location, right.location);
            instruction = INSTRUCTION(OP_MOVE, context->rp, intermediate_result.location);
            type = VAL_BOOLEAN;
            break;

        case TOK_GREATER_EQUAL:
            intermediate_result = compile_binary_comparison(OP_LESSTHAN, context->rp + 2, 0, left.location, right.location);
            code_block_merge(context->current_code_block, intermediate_result.code);
            code_block_free(intermediate_result.code);
            intermediate_result = compile_binary_comparison(OP_EQUAL, context->rp + 3, 1, left.location, right.location);
            instruction = INSTRUCTION(OP_OR, context->rp, context->rp + 2, context->rp + 3);
            type = VAL_BOOLEAN;
            break;

        default:
            ;
    }

    if (intermediate_result.code != NULL)
    {
        code_block_merge(context->current_code_block, intermediate_result.code);
        code_block_free(intermediate_result.code);
    }

    code_block_write(context->current_code_block, instruction);
    return (compile_result_t){ .location=context->rp, .type=type, .code=NULL };
}

compile_result_t compile_assign(ast_t *ast, compile_context_t *context)
{
    // First, check our symbol to make sure it's been defined
    symbol_t symbol = symbol_map_get(context->symbols, ast->op.assign.name);

    // Error handling
    if (symbol.location.type == LOC_UNDEF)
    {
        char *error;
        location_t loc = {ast->location.start, ast->location.end};
        asprintf(&error, "Use of undeclared identifier \"%s\"", ast->op.assign.name);
        printf("%s", format_error(context->name, context->listing, error, loc));
        exit(1);
    }

    if (symbol.type == SYM_CONSTANT)
    {
        char *error;
        location_t loc = {ast->location.start, ast->location.end};
        asprintf(&error, "Cannot assign to constant \"%s\", value is immutable", ast->op.assign.name);
        printf("%s", format_error(context->name, context->listing, error, loc));
        exit(1);
    }

    compile_result_t rvalue = compile_ast(ast->op.assign.value, context);

    switch (rvalue.type)
    {
        case VAL_FUNCTION:
            // TODO: Handle functions
            break;

        default:
            code_block_write(context->current_code_block, INSTRUCTION(OP_MOVE, symbol.location.address, rvalue.location));
            break;
    }

    return (compile_result_t){ .location=symbol.location.address, .type=rvalue.type, NULL};
}

compile_result_t compile_declare(ast_t *ast, compile_context_t *context)
{
    value_type_e type = VAL_ABSENT;
    symbol_t symbol = (symbol_t){ .location={ .address=0, .type=LOC_NONE }, .name=ast->op.declare.name };
    symbol.type = (ast->op.declare.var_type.type == TOK_VAR) ? SYM_VAR : SYM_CONSTANT;

    if (ast->op.declare.initial_value != NULL)
    {
        compile_result_t initial_value = compile_ast(ast->op.declare.initial_value, context);
        type = initial_value.type;

        symbol.location.address = initial_value.location;
        symbol.location.type = LOC_REGISTER;
        context->rp += 1;
    }

    symbol_map_set(context->symbols, symbol);
    return (compile_result_t){ .location=symbol.location.address, .type=type, .code=NULL};
}

compile_result_t compile_ast(ast_t *ast, compile_context_t *context)
{
    compile_result_t result;
    switch (ast->type)
    {
        case AST_STMT_LIST:
            for (int i = 0; i < ast->op.list.size; i++)
            {
                result = compile_ast(ast->op.list.items[i], context);
            }
            break;

        case AST_LITERAL:
            result = compile_literal(ast, context);
            break;

        case AST_UNARY:
            result = compile_unary(ast, context);
            break;

        case AST_BINARY:
            result = compile_binary(ast, context);
            break;

        case AST_DECLARE:
            result = compile_declare(ast, context);
            break;

        case AST_ASSIGN:
            result = compile_assign(ast, context);
            break;

    }
    return result;
}

binary_t *compile(const char *name, const char *listing, ast_t *ast)
{
    compile_context_t *context = context_create(name, listing);
    compile_ast(ast, context);
    binary_t *binary = context->binary;
    context_destroy(context);
    return binary;
}