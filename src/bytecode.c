/*
 * Copyright (c) 2021, Dana Burkart <dana.burkart@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <stdio.h>

#include "bytecode.h"

CodeBlock *code_block_create()
{
    return calloc(1, sizeof(CodeBlock));
}

void code_block_free(CodeBlock *block)
{
    // First, free code
    free(block->code);
    // Free block
    free(block);
}

void code_block_write(CodeBlock *block, Instruction val)
{
    // First, handle an empty code block
    if (block->capacity == 0)
    {
        block->capacity = 2;
        block->code = calloc(block->capacity, sizeof(Instruction));
    }

    // Grow our capacity if necessary
    if (block->size >= (block->capacity - 1))
    {
        block->capacity = block->capacity * 2;
        block->code = realloc(block->code, sizeof(Instruction) * block->capacity);
    }

    block->code[block->size] = val;
    block->size = block->size + 1;
}

void code_block_print(CodeBlock *block)
{
    int num = 1;
    for (int i = 0; i < block->size; i++)
    {
        Instruction instruction = block->code[i];
        int arg1;
        int arg2;
        int arg3;

        printf("%04d ", num);

        switch(instruction.opcode)
        {
            case OP_LOAD:
                printf("LOAD %04d %04d\n", instruction.fields.pair.arg1, instruction.fields.pair.arg2);
                break;
            case OP_ADD:
                printf("ADD %04d %04d %04d\n", instruction.fields.triplet.arg1, instruction.fields.triplet.arg2, instruction.fields.triplet.arg3);
                break;
            case OP_RETURN:
                printf("RETURN\n");
                break;
        }

        num += 1;
    }
}
