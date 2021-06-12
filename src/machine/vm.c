/*
 * Copyright (c) 2021, Dana Burkart <dana.burkart@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <stdio.h>

#include "bytecode.h"
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
        case VAL_NONE:
            printf("{NONE}\n");
            break;
    }
}

vm_t *vm_create(binary_t *binary)
{
    vm_t *vm = malloc(sizeof(vm_t));

    // Set up main memory
    vm->memory = binary->text;

    // Set up the stack
    vm->stack = memory_create();
    vm->sp = 0;

    vm->code = binary->code;
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
        // Pull off the next instruction
        instruction_t instruction = vm->code->code[vm->pc++];
        value_t result;

        switch (instruction.opcode)
        {
            // Load a value into the specified register
            case OP_LOAD:
                vm->registers[instruction.fields.pair.arg1] = memory_get(vm->memory, instruction.fields.pair.arg2);
                break;

            case OP_LOADV:
                result.type = VAL_INT;
                result.contents.number = instruction.fields.pair.arg2;
                vm->registers[instruction.fields.pair.arg1] = result;
                break;

            case OP_STORE:
                memory_set(vm->memory, instruction.fields.pair.arg2, vm->registers[instruction.fields.pair.arg1]);
                break;

            case OP_MOVE:
                vm->registers[instruction.fields.pair.arg1] = vm->registers[instruction.fields.pair.arg2];
                break;

            case OP_JMP:
                // TODO: Use both arguments to piece together the address (we have an extra 8 bits we're not using)
                vm->pc = instruction.fields.pair.arg2;
                break;

            case OP_ADD:
                // TODO: Don't assume numbers
                result.type = VAL_INT;
                result.contents.number = vm->registers[instruction.fields.triplet.arg2].contents.number +
                                         vm->registers[instruction.fields.triplet.arg3].contents.number;
                vm->registers[instruction.fields.triplet.arg1] = result;
                break;

            case OP_SUBTRACT:
                result.type = VAL_INT;
                result.contents.number = vm->registers[instruction.fields.triplet.arg2].contents.number -
                                         vm->registers[instruction.fields.triplet.arg3].contents.number;
                vm->registers[instruction.fields.triplet.arg1] = result;
                break;

            case OP_MULTIPLY:
                result.type = VAL_INT;
                result.contents.number = vm->registers[instruction.fields.triplet.arg2].contents.number *
                                         vm->registers[instruction.fields.triplet.arg3].contents.number;
                vm->registers[instruction.fields.triplet.arg1] = result;
                break;

            case OP_NEGATE:
                result.type = VAL_INT;
                result.contents.number = -vm->registers[instruction.fields.pair.arg2].contents.number;
                vm->registers[instruction.fields.pair.arg1] = result;
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

    printf("\n[stack contents]\n");
    for (int i = 0; i < vm->stack->capacity; i++)
    {
        printf("   %04d ", i);
        value_print(vm->stack->contents[i]);
    }

    printf("\n[register contents]\n");
    for (int i = 1; i < VM_NUM_REGISTERS; i++)
    {
        if (vm->registers[i].type == VAL_NONE)
            break;

        printf("   %04d ", i);
        value_print(vm->registers[i]);
    }

    printf("\nstack pointer: %d\n", vm->sp);
}
