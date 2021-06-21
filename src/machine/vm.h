/*
 * Copyright (c) 2021, Dana Burkart <dana.burkart@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef VM_H
#define VM_H

#include "memory.h"
#include "binary.h"

#define VM_NUM_REGISTERS 128

typedef struct
{
    memory_t *memory;

    // Stack-related data structures
    int stack_size;
    value_t *stack;
    // Stack pointer
    int sp;

    int cstack_size;
    value_t *call_stack;
    int csp;

    // Code-related data structures
    code_block_t *code;
    // Program counter
    int pc;

    // Registers
    value_t registers[VM_NUM_REGISTERS];
} vm_t;

vm_t *vm_create(binary_t *);

void vm_execute(vm_t *);
void vm_dump(vm_t *);

void vm_stack_push(vm_t *, value_t val);
value_t vm_stack_pop(vm_t *);

#endif
