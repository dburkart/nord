/*
 * Copyright (c) 2021, Dana Burkart <dana.burkart@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef BYTECODE_H
#define BYTECODE_H

#include <stdint.h>
#include <stdlib.h>

typedef enum
{
    // load <address> <register>
    OP_LOAD,
    // add <register-in> <register-in> <register-out>
    OP_ADD,
    OP_RETURN
} OpCode;

typedef struct
{
    size_t size;
    size_t capacity;
    uint8_t *code;
} CodeBlock;

CodeBlock *code_block_create();
void code_block_write(CodeBlock *, uint8_t);
void code_block_free(CodeBlock *);

void code_block_print(CodeBlock *);

#endif
