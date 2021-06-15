/*
 * Copyright (c) 2021, Dana Burkart <dana.burkart@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <stdio.h>
#include <string.h>

#include "bytecode.h"
#include "vm.h"

// Defines to make handling type information less verbose
#define REG_TYPE3(a, t) vm->registers[instruction.fields.triplet.a].type == t
#define BOOL2(a) vm->registers[instruction.fields.pair.a].contents.boolean
#define FLOAT2(a) vm->registers[instruction.fields.pair.a].contents.real
#define NUM2(a) vm->registers[instruction.fields.pair.a].contents.number
#define STR2(a) vm->registers[instruction.fields.pair.a].contents.string
#define NUM3(a) vm->registers[instruction.fields.triplet.a].contents.number
#define NUM_OR_FLOAT3(a) ((vm->registers[instruction.fields.triplet.a].type == VAL_FLOAT) ? vm->registers[instruction.fields.triplet.a].contents.real : vm->registers[instruction.fields.triplet.a].contents.number)
#define NUM_OR_FLOAT_OR_BOOL3(a) ((vm->registers[instruction.fields.triplet.a].type == VAL_FLOAT) ? vm->registers[instruction.fields.triplet.a].contents.real : \
                                 ((vm->registers[instruction.fields.triplet.a].type == VAL_INT) ? vm->registers[instruction.fields.triplet.a].contents.number : \
                                   vm->registers[instruction.fields.triplet.a].contents.boolean))
#define IS_NUMBERISH3(a) (vm->registers[instruction.fields.triplet.a].type == VAL_INT || vm->registers[instruction.fields.triplet.a].type == VAL_FLOAT || vm->registers[instruction.fields.triplet.a].type == VAL_BOOLEAN)
#define IS_NUMBERISH2(a) (vm->registers[instruction.fields.pair.a].type == VAL_INT || vm->registers[instruction.fields.pair.a].type == VAL_FLOAT || vm->registers[instruction.fields.pair.a].type == VAL_BOOLEAN)
#define STRING3(a) vm->registers[instruction.fields.triplet.a].contents.string

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
        case VAL_FLOAT:
            printf("{FLOAT:%f}\n", v.contents.real);
            break;
        case VAL_BOOLEAN:
            printf("{BOOLEAN:%s}\n", (v.contents.boolean) ? "true" : "false");
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
    vm->memory = binary->data;

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

            case OP_EQUAL:
                if (IS_NUMBERISH3(arg2) && IS_NUMBERISH3(arg3))
                {
                    result.contents.boolean = (NUM_OR_FLOAT_OR_BOOL3(arg2) == NUM_OR_FLOAT_OR_BOOL3(arg3));
                }
                // If both aren't number-like things, than differing types naturally mean they're not equal
                else if (vm->registers[instruction.fields.triplet.arg2].type != vm->registers[instruction.fields.triplet.arg3].type)
                {
                    result.contents.boolean = false;
                }
                else if (REG_TYPE3(arg2, VAL_STRING) && REG_TYPE3(arg3, VAL_STRING))
                {
                    result.contents.boolean = !strcmp(vm->registers[instruction.fields.triplet.arg2].contents.string,
                                                      vm->registers[instruction.fields.triplet.arg3].contents.string);
                }

                if (result.contents.boolean != instruction.fields.triplet.arg1)
                    vm->pc += 1;

                break;

            case OP_LESSTHAN:
                if (IS_NUMBERISH3(arg2) && IS_NUMBERISH3(arg3))
                {
                    result.contents.boolean = NUM_OR_FLOAT3(arg2) < NUM_OR_FLOAT3(arg3);
                }
                else
                {
                    // Error?
                }

                if (result.contents.boolean != instruction.fields.triplet.arg1)
                    vm->pc += 1;

                break;

            case OP_OR:
                if (IS_NUMBERISH3(arg2) && IS_NUMBERISH3(arg3))
                {
                    result.contents.boolean = NUM_OR_FLOAT3(arg2) || NUM_OR_FLOAT3(arg3);
                }
                else
                {
                    // Error?
                }
                result.type = VAL_BOOLEAN;

                vm->registers[instruction.fields.triplet.arg1] = result;
                break;

            case OP_ADD:
                // TODO: Don't assume numbers
                if (REG_TYPE3(arg2, VAL_FLOAT) || REG_TYPE3(arg3, VAL_FLOAT))
                {
                    result.type = VAL_FLOAT;
                    result.contents.real = NUM_OR_FLOAT3(arg2) + NUM_OR_FLOAT3(arg3);
                }
                else if (REG_TYPE3(arg2, VAL_STRING) || REG_TYPE3(arg3, VAL_STRING))
                {
                    result.type = VAL_STRING;
                    asprintf(&result.contents.string, "%s%s", STRING3(arg2), STRING3(arg3));
                }
                else
                {
                    result.type = VAL_INT;
                    result.contents.number = NUM3(arg2) + NUM3(arg3);
                }
                vm->registers[instruction.fields.triplet.arg1] = result;
                break;

            case OP_SUBTRACT:
                if (REG_TYPE3(arg2, VAL_FLOAT) || REG_TYPE3(arg3, VAL_FLOAT))
                {
                    result.type = VAL_FLOAT;
                    result.contents.real = NUM_OR_FLOAT3(arg2) - NUM_OR_FLOAT3(arg3);
                }
                else
                {
                    result.type = VAL_INT;
                    result.contents.number = NUM3(arg2) - NUM3(arg3);
                }
                vm->registers[instruction.fields.triplet.arg1] = result;
                break;

            case OP_MULTIPLY:
                if (REG_TYPE3(arg2, VAL_FLOAT) || REG_TYPE3(arg3, VAL_FLOAT))
                {
                    result.type = VAL_FLOAT;
                    result.contents.real = (NUM_OR_FLOAT3(arg2)) * (NUM_OR_FLOAT3(arg3));
                }
                else
                {
                    result.type = VAL_INT;
                    result.contents.number = NUM3(arg2) * NUM3(arg3);
                }
                vm->registers[instruction.fields.triplet.arg1] = result;
                break;

            case OP_DIVIDE:
                result.type = VAL_FLOAT;
                result.contents.real = NUM_OR_FLOAT3(arg2) / (float) NUM_OR_FLOAT3(arg3);
                vm->registers[instruction.fields.triplet.arg1] = result;
                break;

            case OP_NEGATE:
                result.type = VAL_INT;
                result.contents.number = -vm->registers[instruction.fields.pair.arg2].contents.number;
                vm->registers[instruction.fields.pair.arg1] = result;
                break;

            case OP_NOT:
                result.type = VAL_BOOLEAN;
                switch(vm->registers[instruction.fields.pair.arg2].type)
                {
                    case VAL_INT:
                        result.contents.boolean = !NUM2(arg2);
                        break;
                    case VAL_FLOAT:
                        result.contents.boolean = !FLOAT2(arg2);
                        break;
                    case VAL_BOOLEAN:
                        result.contents.boolean = !BOOL2(arg2);
                        break;
                    case VAL_NONE:
                        result.contents.boolean = true;
                        break;
                    case VAL_STRING:
                        result.contents.boolean = (bool)!strlen(STR2(arg2));
                        break;
                }
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
        if (vm->memory->contents[i].type == VAL_NONE)
            break;

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
