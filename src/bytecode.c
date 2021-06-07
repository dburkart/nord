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

void code_block_write(CodeBlock *block, uint8_t val)
{
    // First, handle an empty code block
    if (block->capacity == 0)
    {
        block->capacity = 2;
        block->code = calloc(block->capacity, sizeof(uint8_t));
    }

    // Grow our capacity if necessary
    if (block->size >= (block->capacity - 1))
    {
        block->capacity = block->capacity * 2;
        block->code = realloc(block->code, sizeof(uint8_t) * block->capacity);
    }

    block->code[block->size] = val;
    block->size = block->size + 1;
}

void code_block_print(CodeBlock *block)
{
    int num = 1;
    for (int i = 0; i < block->size; i++)
    {
        uint8_t instruction = block->code[i];
        int arg1;
        int arg2;
        int arg3;

        printf("%04d ", num);

        switch(instruction)
        {
            case OP_LOAD:
                arg1 = block->code[++i];
                arg2 = block->code[++i];
                printf("LOAD %04d %04d\n", arg1, arg2);
                break;
            case OP_ADD:
                arg1 = block->code[++i];
                arg2 = block->code[++i];
                arg3 = block->code[++i];
                printf("ADD %04d %04d %04d\n", arg1, arg2, arg3);
                break;
            case OP_RETURN:
                printf("RETURN\n");
                break;
        }

        num += 1;
    }
}
