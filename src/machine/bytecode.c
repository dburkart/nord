/*
 * Copyright (c) 2021, Dana Burkart <dana.burkart@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <stdio.h>

#include "bytecode.h"

code_block_t *code_block_create(void)
{
    return calloc(1, sizeof(code_block_t));
}

void code_block_free(code_block_t *block)
{
    // First, free code
    free(block->code);
    // Free block
    free(block);
}

void code_block_write(code_block_t *block, instruction_t val)
{
    // First, handle an empty code block
    if (block->capacity == 0)
    {
        block->capacity = 2;
        block->code = calloc(block->capacity, sizeof(instruction_t));
    }

    // Grow our capacity if necessary
    if (block->size >= (block->capacity - 1))
    {
        block->capacity = block->capacity * 2;
        block->code = realloc(block->code, sizeof(instruction_t) * block->capacity);
    }

    block->code[block->size] = val;
    block->size = block->size + 1;
}

void code_block_print(code_block_t *block)
{
    int num = 1;
    for (int i = 0; i < block->size; i++)
    {
        instruction_t instruction = block->code[i];
        int arg1;
        int arg2;
        int arg3;

        printf("%#06x ", num);

        switch(instruction.opcode)
        {
            case OP_LOAD:
                printf("LOAD     %#04x %#04x\n", instruction.fields.pair.arg1, instruction.fields.pair.arg2);
                break;
            case OP_LOADV:
                printf("LOADV    %#04x %d\n", instruction.fields.pair.arg1, instruction.fields.pair.arg2);
                break;
            case OP_MOVE:
                printf("MOVE     %#04x %#04x\n", instruction.fields.pair.arg1, instruction.fields.pair.arg2);
                break;
            case OP_ADD:
                printf("ADD      %#04x %#04x %#04x\n", instruction.fields.triplet.arg1, instruction.fields.triplet.arg2, instruction.fields.triplet.arg3);
                break;
            case OP_SUBTRACT:
                printf("SUBTRACT %#04x %#04x %#04x\n", instruction.fields.triplet.arg1, instruction.fields.triplet.arg2, instruction.fields.triplet.arg3);
                break;
            case OP_MULTIPLY:
                printf("MULTIPLY %#04x %#04x %#04x\n", instruction.fields.triplet.arg1, instruction.fields.triplet.arg2, instruction.fields.triplet.arg3);
                break;
            case OP_NEGATE:
                printf("NEGATE   %#04x %#04x\n", instruction.fields.pair.arg1, instruction.fields.pair.arg2);
                break;
            case OP_RETURN:
                printf("RETURN\n");
                break;
        }

        num += 1;
    }
}
