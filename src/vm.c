/*
 * Copyright (c) 2021, Dana Burkart <dana.burkart@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <stdio.h>

#include "vm.h"

void value_print(Value v)
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

Memory *memory_create()
{
    Memory *mem = calloc(1, sizeof(Memory));

    return mem;
}

void memory_free(Memory *mem)
{
    // TODO: Free values
    free(mem);
}

void memory_set(Memory *mem, int address, Value val)
{
    // We've not stored anything in this memory block yet, so set us up
    // for a capacity of address + 1
    if (mem->capacity == 0)
    {
        mem->capacity = address + 1;
        mem->contents = calloc(mem->capacity, sizeof(Value));
    }

    if (mem->capacity <= address)
    {
        int difference = address - mem->capacity;
        mem->capacity = mem->capacity * 2;
        mem->contents = realloc(mem->contents, mem->capacity * sizeof(Value));
    }

    mem->contents[address] = val;
}

Value memory_get(Memory *mem, int address)
{
    // TODO: Error checking
    return mem->contents[address];
}

VM *vm_create(CodeBlock *block)
{
    VM *vm = malloc(sizeof(VM));

    // Set up main memory
    vm->memory = memory_create();

    // Set up the stack
    vm->stack = memory_create();
    vm->sp = 0;

    vm->code = block;
    vm->ep = 0;

    return vm;
}

// Push a value onto the stack of a VM, returning the register it is stored in
int vm_stack_push(VM *vm, Value val)
{
    int reg = vm->sp;

    memory_set(vm->stack, vm->sp, val);
    vm->sp++;

    return reg;
}

Value vm_stack_pop(VM *vm)
{
    Value val = memory_get(vm->stack, vm->sp);
    vm->sp--;

    return val;
}

void vm_execute(VM *vm)
{
    while (vm->ep < vm->code->size)
    {
        // Pull off the next opcode
        uint8_t opcode = vm->code->code[vm->ep++];
        uint8_t arg1;
        uint8_t arg2;
        uint8_t arg3;

        Value result;

        switch (opcode)
        {
            // Load a value into the specified register
            case OP_LOAD:
                arg1 = vm->code->code[vm->ep++];
                arg2 = vm->code->code[vm->ep++];

                memory_set(vm->stack, arg2, memory_get(vm->memory, arg1));
                break;
            case OP_ADD:
                arg1 = vm->code->code[vm->ep++];
                arg2 = vm->code->code[vm->ep++];
                arg3 = vm->code->code[vm->ep++];
                result.type = VAL_INT;
                result.contents.number = vm->stack->contents[arg1].contents.number + vm->stack->contents[arg2].contents.number;

                // TODO: Don't assume numbers
                memory_set(vm->stack, arg3, result);
                break;
        }
    }
}

void vm_dump(VM *vm)
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
