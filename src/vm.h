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
} value_t;

typedef struct
{
    size_t capacity;
    value_t *contents;
} memory_t;

typedef struct
{
    memory_t *memory;

    // Stack-related data structures
    memory_t *stack;
    // Stack pointer
    int sp;

    // Code-related data structures
    code_block_t *code;
    // Program counter
    int pc;

    // Registers
    value_t registers[128];
} vm_t;

memory_t *memory_create(void);
void memory_free(memory_t *);
void memory_set(memory_t *, int address, value_t val);
value_t memory_get(memory_t *, int address);

vm_t *vm_create(code_block_t *);
int vm_stack_push(vm_t *, value_t val);
value_t vm_stack_pop(vm_t *);

void vm_execute(vm_t *);
void vm_dump(vm_t *);

#endif
