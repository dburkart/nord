/*
 * Copyright (c) 2021, Dana Burkart <dana.burkart@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "compile.h"

#include "parse.h"
#include "symbol.h"

typedef struct
{
    symbol_map_t *symbols;
    code_block_t *block;
    uint8_t rp;
} compile_context_t;

compile_context_t *context_create(void)
{
    compile_context_t *context = malloc(sizeof(compile_context_t));

    context->symbols = symbol_map_create();
    context->block = code_block_create();
    context->rp = 1;

    return context;
}

void context_destroy(compile_context_t *context)
{
    symbol_map_destroy(context->symbols);
    // NOTE: We don't free the code block, since that is returned (and this is
    //       an internal data structure).
    free(context);
}

uint8_t compile_internal(ast_t *ast, compile_context_t *context)
{
    uint8_t result = 0, left, right;
    location_t loc;
    instruction_t instruction;

    switch (ast->type)
    {
        case BINARY:
            // First get our left and right values
            left = compile_internal(ast->op.binary.left, context);
            context->rp += 1;
            right = compile_internal(ast->op.binary.right, context);

            switch (ast->op.binary.operator.type)
            {
                case PLUS:
                    instruction.opcode = OP_ADD;
                    instruction.fields.triplet.arg1 = context->rp - 1;
                    instruction.fields.triplet.arg2 = left;
                    instruction.fields.triplet.arg3 = right;
                default:
                    ;
            }

            code_block_write(context->block, instruction);

            context->rp -= 1;
            break;

        case DECLARE:
            // Handle declaration with no assignment
            if (ast->op.declare.initial_value == NULL)
            {
                loc.type = LOC_NONE;
            }
            else
            {
                result = compile_internal(ast->op.declare.initial_value, context);
                loc.type = LOC_REGISTER;
                loc.address = result;
                context->rp += 1;
            }

            symbol_map_set(context->symbols, ast->op.declare.name, loc);
            break;

        case LITERAL:
            // For now, only handle small ints, because we don't have memory yet
            if (ast->op.literal.type.type == NUMBER)
            {
                result = context->rp;
                instruction.opcode = OP_LOADV;
                instruction.fields.pair.arg1 = result;
                instruction.fields.pair.arg2 = atoi(ast->op.literal.value);

                code_block_write(context->block, instruction);
            }
            break;

        default:
            ;
    }

    return result;
}

code_block_t *compile(ast_t *ast)
{
    compile_context_t *context = context_create();
    compile_internal(ast, context);
    code_block_t *block = context->block;
    context_destroy(context);
    return block;
}
