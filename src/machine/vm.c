/*
 * Copyright (c) 2021, Dana Burkart <dana.burkart@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <stdio.h>

#include "vm.h"

void value_print(value_t v)
{
    switch (v.type)
    {
        case VAL_INT:
            printf("{INT:%d}\n", v.contents.number);
            break;
        case VAL_STRING:
            printf("{STRING:%s}\n", v.contents.string);
            break;
    }
}

vm_t *vm_create(code_block_t *block)
{
    vm_t *vm = malloc(sizeof(vm_t));

    // Set up main memory
    vm->memory = memory_create();

    // Set up the stack
    vm->stack = memory_create();
    vm->sp = 0;

    vm->code = block;
    vm->pc = 0;

    return vm;
}

// Push a value onto the stack of a vm_t, returning the register it is stored in
int vm_stack_push(vm_t *vm, value_t val)
{
    int reg = vm->sp;

    memory_set(vm->stack, vm->sp, val);
    vm->sp++;

    return reg;
}

value_t vm_stack_pop(vm_t *vm)
{
    value_t val = memory_get(vm->stack, vm->sp);
    vm->sp--;

    return val;
}

void vm_execute(vm_t *vm)
{
    while (vm->pc < vm->code->size)
    {
        // Pull off the next opcode
        instruction_t instruction = vm->code->code[vm->pc++];
        value_t result;

        switch (instruction.opcode)
        {
            // Load a value into the specified register
            case OP_LOAD:

                vm->registers[instruction.fields.pair.arg1] = memory_get(vm->memory, instruction.fields.pair.arg2);
                break;
            case OP_ADD:
                // TODO: Don't assume numbers
                result.type = VAL_INT;
                result.contents.number = vm->registers[instruction.fields.triplet.arg2].contents.number +
                                         vm->registers[instruction.fields.triplet.arg3].contents.number;
                memory_set(vm->stack, instruction.fields.triplet.arg1, result);
                break;
        }
    }
}

void vm_dump(vm_t *vm)
{
    printf("[memory contents]\n");
    for (int i = 0; i < vm->memory->capacity; i++)
    {
        printf("   %04d ", i);
        value_print(vm->memory->contents[i]);
    }

    printf("\n[register contents]\n");
    for (int i = 0; i < vm->stack->capacity; i++)
    {
        printf("   %04d ", i);
        value_print(vm->stack->contents[i]);
    }

    printf("\nstack pointer: %d", vm->sp);
}
