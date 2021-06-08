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
    // load <register> <address>
    OP_LOAD,
    // add <register-out> <register-in> <register-in>
    OP_ADD,
    OP_RETURN
} OpCode;

typedef struct __attribute__((__packed__))
{
    uint8_t opcode;
    union {
        // Represents an instruction of the form OP A B
        struct {
            uint8_t arg1;
            uint16_t arg2;
        } pair;
        // Represents an instruction of the form OP A B C
        struct {
            uint8_t arg1;
            uint8_t arg2;
            uint8_t arg3;
        } triplet;
    } fields;
} Instruction;

typedef struct
{
    size_t size;
    size_t capacity;
    Instruction *code;
} CodeBlock;

CodeBlock *code_block_create();
void code_block_write(CodeBlock *, Instruction);
void code_block_free(CodeBlock *);

void code_block_print(CodeBlock *);

#endif
