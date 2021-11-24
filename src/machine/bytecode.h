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
    // -- Constants

    // nil <register>
    OP_NIL,

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
    // modulo <register-out> <register-in> <register-in>
    OP_MODULO,

    // -- Logic

    // and <register-out> <register-in> <register-in>
    OP_AND,
    // or <register-out> <register-in> <register-in>
    OP_OR,

    // equal <value-desired> <register-in> <register-in>
    OP_EQUAL,
    // lt <value-desired> <register-in> <register-in>
    OP_LESSTHAN,
    // not <register-out> <register-in>
    OP_NOT,

    // -- Iteration

    // deref <register-out> <register-in> <register-post-advance>
    OP_DEREF,

    // -- Functions

    // call
    OP_CALL,

    // calld @<memory-addr>
    OP_CALL_DYNAMIC,

    // return <register>
    OP_RETURN,

    // -- Modules

    // import @<memory-addr>
    OP_IMPORT,
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
        struct {
            uint8_t arg1;
            int16_t arg2;
        } pair_signed;
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

typedef struct
{
    size_t size;
    size_t capacity;
    code_block_t **blocks;
} code_collection_t;

code_block_t *code_block_create(void);
void code_block_write(code_block_t *, instruction_t);
void code_block_merge(code_block_t *, code_block_t *);
void code_block_free(code_block_t *);

code_collection_t *code_collection_create(void);
void code_collection_add_block(code_collection_t *, code_block_t *);
void code_collection_free(code_collection_t *);

#endif
