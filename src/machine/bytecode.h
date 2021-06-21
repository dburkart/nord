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
    // -- Loading to and from memory and registers

    // load <register> <address>
    OP_LOAD,
    // load <register> <value>
    OP_LOADV,
    // store <register> <address>
    OP_STORE,
    // move <register-out> <register-in>
    OP_MOVE,

    // -- Stack manipulation

    // push <register>
    OP_PUSH,
    // pop <register>
    OP_POP,
    // save <register>
    OP_SAVE,
    // restore <num-registers>
    OP_RESTORE,

    // -- Jumps

    // jump <register>
    OP_JMP,

    // -- Arithmetic operations

    // add <register-out> <register-in> <register-in>
    OP_ADD,
    // subtract <register-out> <register-in> <register-in>
    OP_SUBTRACT,
    // multiply <register-out> <register-in> <register-in>
    OP_MULTIPLY,
    // divide <register-out> <register-in> <register-in>
    OP_DIVIDE,
    // negate <register-out> <register-in>
    OP_NEGATE,

    // -- Logic

    // equal <value-desired> <register-in> <register-in>
    OP_EQUAL,
    // lt <value-desired> <register-in> <register-in>
    OP_LESSTHAN,
    // or <register-out> <register-in> <register-in>
    OP_OR,
    // not <register-out> <register-in>
    OP_NOT,

    // -- Functions

    // call <register-addr>
    OP_CALL,

    // return <register>
    OP_RETURN
} opcode_t;

// An instruction is an opcode paired with several operands
typedef struct
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

#endif
