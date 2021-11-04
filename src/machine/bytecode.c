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

void code_block_merge(code_block_t *into, code_block_t *from)
{
    for (int i = 0; i < from->size; i++)
    {
        code_block_write(into, from->code[i]);
    }
}
