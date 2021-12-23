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
    OP_NONE,

    // -- Constants

    // nil <register>
    OP_NIL,

    // -- Loading to and from memory and registers

    // load <register> <address>
    //      Loads the value at the specified address into a register
    OP_LOAD,
    // load <register> <value>
    //      Loads a 16-bit value into the specified register
    OP_LOADV,
    // store <register> <address>
    //      Stores the value in a register to the specified address
    OP_STORE,
    // move <register-out> <register-in>
    //      Move a value from one register to another
    OP_MOVE,

    // -- Stack manipulation

    // push <register>
    //      Push the value in a given register onto the stack
    OP_PUSH,
    // pop <register>
    //      Pop the next value on the stack into the given register
    OP_POP,
    // restore <num-registers>
    OP_RESTORE,

    // -- Jumps

    // jump <register>
    //      Jump a given number of instructions based on the value in the
    //      target register
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
    // not <register-out> <register-in>
    OP_NOT,

    // equal <value-desired> <register-in> <register-in>
    //      If the result comparing the two in-registers for equality results
    //      in the value specified in arg1, execute the next instruction.
    //      Otherwise, jump over it.
    OP_EQUAL,
    // lt <value-desired> <register-in> <register-in>
    //      If the result of comparing the first in-register to the second by
    //      '<' martches the desired value specified in arg1, execute the next
    //      instruction. Otherwise, jump over it.
    OP_LESSTHAN,

    // -- Iteration

    // deref <register-out> <register-in> <register-post-advance>
    //      TODO: This instruction should be renamed
    //      Dereference the value pointed to by the iterator in register-in,
    //      storing the value at that index in register-out. Finally, advance
    //      the iterator by the value in register-post-advance.
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
