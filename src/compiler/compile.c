/*
 * Copyright (c) 2021, Dana Burkart <dana.burkart@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <stdlib.h>

#include "compile.h"
#include "machine/bytecode.h"
#include "machine/value.h"
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
    switch (ast->op.literal.token.type)
    {
        case TOK_NUMBER:
            code_block_write(context->current_code_block, INSTRUCTION(OP_LOADV, context->rp, atoi(ast->op.literal.value)));
            break;

        case TOK_STRING:
        case TOK_FLOAT:
            memory_set(context->binary->data, context->mp, value(atof(ast->op.literal.value)));
            code_block_write(context->current_code_block, INSTRUCTION(OP_LOAD, context->rp, context->mp));
            context->mp += 1;
            break;

        case TOK_TRUE:
        case TOK_FALSE:
            code_block_write(context->current_code_block, INSTRUCTION(OP_LOAD, context->rp, (ast->op.literal.token.type == TOK_TRUE) ? 1 : 0));
            break;

        case TOK_NIL:
            code_block_write(context->current_code_block, INSTRUCTION(OP_NIL, context->rp));
            break;

        default:
            break;
    }

    return (compile_result_t){ .location=context->rp, .code=NULL };
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

    return (compile_result_t){ .location=context->rp, .code=NULL };
}

compile_result_t compile_binary(ast_t *ast, compile_context_t *context)
{
    compile_result_t left = compile_ast(ast->op.binary.left, context);
    context->rp += 1;
    compile_result_t right = compile_ast(ast->op.binary.right, context);
    context->rp -= 1;

    instruction_t instruction;
    switch (ast->op.binary.operator.type)
    {
        case TOK_PLUS:
            instruction = INSTRUCTION(OP_ADD, context->rp, left.location, right.location);
            break;

        case TOK_MINUS:
            instruction = INSTRUCTION(OP_SUBTRACT, context->rp, left.location, right.location);
            break;

        case TOK_ASTERISK:
            instruction = INSTRUCTION(OP_MULTIPLY, context->rp, left.location, right.location);
            break;

        case TOK_SLASH:
            instruction = INSTRUCTION(OP_DIVIDE, context->rp, left.location, right.location);
            break;

        default:
            ;
    }

    code_block_write(context->current_code_block, instruction);
    return (compile_result_t){ .location=context->rp, .code=NULL };
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