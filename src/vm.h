/*
 * Copyright (c) 2021, Dana Burkart <dana.burkart@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef VM_H
#define VM_H

#include "bytecode.h"

typedef struct
{
    enum
    {
        VAL_INT, VAL_STRING,
    } type;

    union
    {
        int number;
        char *string;
    } contents;
} Value;

typedef struct
{
    size_t capacity;
    Value *contents;
} Memory;

typedef struct
{
    Memory *memory;

    // Stack-related data structures
    Memory *stack;
    // Stack pointer
    int sp;

    // Code-related data structures
    CodeBlock *code;
    // Program counter
    int pc;

    // Registers
    Value registers[128];
} VM;

Memory *memory_create();
void memory_free(Memory *);
void memory_set(Memory *, int address, Value val);
Value memory_get(Memory *, int address);

VM *vm_create(CodeBlock *);
int vm_stack_push(VM *, Value val);
Value vm_stack_pop(VM *);

void vm_execute(VM *);
void vm_dump(VM *);

#endif
