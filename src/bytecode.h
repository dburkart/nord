/*
 * Copyright (c) 2021, Dana Burkart <dana.burkart@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef BYTECODE_H
#define BYTECODE_H

#include <stdint.h>
#include <stdlib.h>

// Opcodes for VM instructions
typedef enum
{
    // load <register> <address>
    OP_LOAD,
    // add <register-out> <register-in> <register-in>
    OP_ADD,
    OP_RETURN
} opcode_t;

// An instruction is an opcode paired with several operands
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
} instruction_t;

// A code block is a series of instructions
typedef struct
{
    size_t size;
    size_t capacity;
    instruction_t *code;
} code_block_t;

code_block_t *code_block_create(void);
void code_block_write(code_block_t *, instruction_t);
void code_block_free(code_block_t *);

void code_block_print(code_block_t *);

#endif
