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

code_collection_t *code_collection_create(void)
{
    return calloc(1, sizeof(code_collection_t));
}

void code_collection_add_block(code_collection_t *collection, code_block_t *block)
{
    if (collection->capacity == 0)
    {
        collection->capacity = 2;
        collection->blocks = calloc(collection->capacity, sizeof(code_block_t *));
    }

    if (collection->size >= (collection->capacity - 1))
    {
        collection->capacity = collection->capacity * 2;
        collection->blocks = realloc(collection->blocks, sizeof(code_block_t *) * collection->capacity);
    }

    collection->blocks[collection->size] = block;
    collection->size = collection->size + 1;
}

void code_collection_free(code_collection_t *collection)
{
    for (int i = 0; i < collection->size; i++)
    {
        code_block_free(collection->blocks[i]);
    }
    free(collection->blocks);
}
