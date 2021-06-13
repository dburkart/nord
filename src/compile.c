/*
 * Copyright (c) 2021, Dana Burkart <dana.burkart@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <stdio.h>

#include "compile.h"

#include "memory.h"
#include "parse.h"
#include "symbol.h"

typedef struct
{
    symbol_map_t *symbols;
    binary_t *binary;
    uint8_t rp;
    uint64_t mp;
} compile_context_t;

compile_context_t *context_create(void)
{
    compile_context_t *context = malloc(sizeof(compile_context_t));

    context->symbols = symbol_map_create();
    context->binary = binary_create();
    context->binary->text = memory_create();
    context->binary->code = code_block_create();
    context->rp = 1;
    context->mp = 0;

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
    value_t val;

    switch (ast->type)
    {
        case UNARY:
            right = compile_internal(ast->op.unary.operand, context);

            switch (ast->op.unary.operator.type)
            {
                case MINUS:
                    instruction.opcode = OP_NEGATE;
                    instruction.fields.pair.arg1 = context->rp;
                    instruction.fields.pair.arg2 = right;
                    break;
                default:
                    ;
            }

            code_block_write(context->binary->code, instruction);
            result = context->rp;
            break;
        case BINARY:
            // First get our left and right values
            left = compile_internal(ast->op.binary.left, context);
            context->rp += 1;
            right = compile_internal(ast->op.binary.right, context);
            context->rp -= 1;

            switch (ast->op.binary.operator.type)
            {
                case PLUS:
                    instruction.opcode = OP_ADD;
                    instruction.fields.triplet.arg1 = context->rp;
                    instruction.fields.triplet.arg2 = left;
                    instruction.fields.triplet.arg3 = right;
                    break;
                case MINUS:
                    instruction.opcode = OP_SUBTRACT;
                    instruction.fields.triplet.arg1 = context->rp;
                    instruction.fields.triplet.arg2 = left;
                    instruction.fields.triplet.arg3 = right;
                    break;
                case ASTERISK:
                    instruction.opcode = OP_MULTIPLY;
                    instruction.fields.triplet.arg1 = context->rp;
                    instruction.fields.triplet.arg2 = left;
                    instruction.fields.triplet.arg3 = right;
                    break;
                case SLASH:
                    instruction.opcode = OP_DIVIDE;
                    instruction.fields.triplet.arg1 = context->rp;
                    instruction.fields.triplet.arg2 = left;
                    instruction.fields.triplet.arg3 = right;
                default:
                    ;
            }

            code_block_write(context->binary->code, instruction);
            result = context->rp;
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

                if (result < context->rp)
                {
                    instruction.opcode = OP_MOVE;
                    instruction.fields.pair.arg1 = context->rp;
                    instruction.fields.pair.arg2 = result;
                    code_block_write(context->binary->code, instruction);
                    loc.address = context->rp;
                }
                else
                {
                    loc.address = result;
                }

                loc.type = LOC_REGISTER;
                context->rp += 1;
            }

            symbol_map_set(context->symbols, ast->op.declare.name, loc);
            break;

        case ASSIGN:
            result = compile_internal(ast->op.assign.value, context);
            // Right now, since we assign registers in a stack-like manner, we
            // use a move instruction for assignment so that register usage
            // stays compact. If we used a different method for register
            // assignment, a more efficient (at runtime) approach would be to
            // simply change the location of the variable in the symbol map.
            //
            // TODO: Handle variables in memory
            loc = symbol_map_get(context->symbols, ast->op.assign.name);
            instruction.opcode = OP_MOVE;
            instruction.fields.pair.arg1 = loc.address;
            instruction.fields.pair.arg2 = result;

            code_block_write(context->binary->code, instruction);
            break;

        case LITERAL:
            // TODO: Support ints which are larger than 16-bit
            if (ast->op.literal.type.type == NUMBER)
            {
                result = context->rp;
                instruction.opcode = OP_LOADV;
                instruction.fields.pair.arg1 = result;
                instruction.fields.pair.arg2 = atoi(ast->op.literal.value);

                code_block_write(context->binary->code, instruction);
            }

            if (ast->op.literal.type.type == FLOAT)
            {
                result = context->rp;

                // First, set the constant in the text section of our binary
                val.type = VAL_FLOAT;
                val.contents.real = atof(ast->op.literal.value);
                memory_set(context->binary->text, context->mp, val);

                instruction.opcode = OP_LOAD;
                instruction.fields.pair.arg1 = result;
                instruction.fields.pair.arg2 = context->mp;

                code_block_write(context->binary->code, instruction);
                context->mp += 1;
            }

            if (ast->op.literal.type.type == STRING)
            {
                result = context->rp;

                // First, set the constant in the text section of our binary
                val.type = VAL_STRING;
                val.contents.string = ast->op.literal.value;
                memory_set(context->binary->text, context->mp, val);

                // Now, write out an instruction to load it into a register
                instruction.opcode = OP_LOAD;
                instruction.fields.pair.arg1 = result;
                instruction.fields.pair.arg2 = context->mp;

                code_block_write(context->binary->code, instruction);
                context->mp += 1;
            }

            if (ast->op.literal.type.type == IDENTIFIER)
            {
                loc = symbol_map_get(context->symbols, ast->op.literal.value);
                // TODO: Handle memory addresses
                result = loc.address;
            }
            break;

        case GROUP:
            result = compile_internal(ast->op.group, context);
            break;

        case STATEMENT_LIST:
            for (int i = 0; i < ast->op.list.size; i++)
            {
                compile_internal(ast->op.list.statements[i], context);
            }
            break;

        default:
            ;
    }

    return result;
}

binary_t *compile(ast_t *ast)
{
    compile_context_t *context = context_create();
    compile_internal(ast, context);
    binary_t *binary = context->binary;
    context_destroy(context);
    return binary;
}
