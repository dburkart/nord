/*
 * Copyright (c) 2021, Dana Burkart <dana.burkart@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <assert.h>
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>

#include "bytecode.h"
#include "vm.h"
#include "value.h"

// Defines to make handling type information less verbose
#define REG_TYPE3(a, t) vm->registers[instruction.fields.triplet.a].type == t
#define BOOL2(a) vm->registers[instruction.fields.pair.a].contents.boolean
#define FLOAT2(a) vm->registers[instruction.fields.pair.a].contents.real
#define NUM2(a) vm->registers[instruction.fields.pair.a].contents.number
#define STR2(a) ((string_t *)vm->registers[instruction.fields.pair.a].contents.object)->string
#define NUM3(a) vm->registers[instruction.fields.triplet.a].contents.number
#define NUM_OR_FLOAT3(a) ((vm->registers[instruction.fields.triplet.a].type == VAL_FLOAT) ? vm->registers[instruction.fields.triplet.a].contents.real : vm->registers[instruction.fields.triplet.a].contents.number)
#define NUM_OR_FLOAT_OR_BOOL3(a) ((vm->registers[instruction.fields.triplet.a].type == VAL_FLOAT) ? vm->registers[instruction.fields.triplet.a].contents.real : \
                                 ((vm->registers[instruction.fields.triplet.a].type == VAL_INT) ? vm->registers[instruction.fields.triplet.a].contents.number : \
                                   vm->registers[instruction.fields.triplet.a].contents.boolean))
#define IS_NUMBERISH3(a) (vm->registers[instruction.fields.triplet.a].type == VAL_INT || vm->registers[instruction.fields.triplet.a].type == VAL_FLOAT || vm->registers[instruction.fields.triplet.a].type == VAL_BOOLEAN)
#define IS_NUMBERISH2(a) (vm->registers[instruction.fields.pair.a].type == VAL_INT || vm->registers[instruction.fields.pair.a].type == VAL_FLOAT || vm->registers[instruction.fields.pair.a].type == VAL_BOOLEAN)
#define STRING3(a) ((string_t *)vm->registers[instruction.fields.triplet.a].contents.object)->string

void vm_stack_create(vm_t *);
void vm_cstack_create(vm_t *);

void value_print(value_t v)
{
    string_t *s;
    switch (v.type)
    {
        case VAL_INT:
            printf("{INT:%d}\n", v.contents.number);
            break;
        case VAL_STRING:
            s = (string_t *)v.contents.object;
            printf("{STRING:%s}\n", s->string);
            break;
        case VAL_FLOAT:
            printf("{FLOAT:%f}\n", v.contents.real);
            break;
        case VAL_BOOLEAN:
            printf("{BOOLEAN:%s}\n", (v.contents.boolean) ? "true" : "false");
            break;
        case VAL_ABSENT:
            printf("{NONE}\n");
            break;
        case VAL_TUPLE:
            printf("{TUPLE}\n");
            break;
    }
}

vm_t *vm_create(binary_t *binary)
{
    vm_t *vm = malloc(sizeof(vm_t));

    // Set up main memory
    vm->memory = binary->data;

    // Set up the stack
    vm_stack_create(vm);

    // Set up the call stack
    vm_cstack_create(vm);

    vm->code = binary->code;
    vm->pc = 0;

    memset(&vm->registers, 0, 128 * sizeof(value_t));

    return vm;
}

void vm_stack_create(vm_t *vm)
{
    vm->stack = memory_create(VM_STACK_SIZE);
    vm->sp = 0;
}

// Push a value onto the stack of a vm_t, returning the register it is stored in
void vm_stack_push(vm_t *vm, value_t val)
{
    memory_set(vm->stack, vm->sp++, val);
}

value_t vm_stack_pop(vm_t *vm)
{
    vm->sp--;
    return memory_get(vm->stack, vm->sp);
}

void vm_cstack_create(vm_t *vm)
{
    vm->call_stack = memory_create(VM_STACK_SIZE);
    vm->csp = 0;
}

void vm_cstack_push(vm_t *vm, value_t val)
{
    memory_set(vm->call_stack, vm->csp++, val);
}

value_t vm_cstack_pop(vm_t *vm)
{
    vm->csp--;
    return memory_get(vm->call_stack, vm->csp);
}

void vm_execute(vm_t *vm)
{
    while (vm->pc < vm->code->size)
    {
        // Pull off the next instruction
        instruction_t instruction = vm->code->code[vm->pc++];
        value_t ret;
        value_t result;
        string_t *s1, *s2;
        char *stmp;
        memory_t *mem;

        switch (instruction.opcode)
        {
            // Load a value into the specified register
            case OP_LOAD:
                if (instruction.fields.pair.arg1 & 0x70)
                {
                    mem = vm->stack;
                }
                else
                {
                    mem = vm->memory;
                }

                vm->registers[instruction.fields.pair.arg1] = memory_get(mem, instruction.fields.pair.arg2);
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

            case OP_PUSH:
                vm_stack_push(vm, vm->registers[instruction.fields.pair.arg1]);
                break;

            case OP_POP:
                vm->registers[instruction.fields.pair.arg1] = vm_stack_pop(vm);
                break;

            case OP_SAVE:
                // Key
                ret.type = VAL_INT;
                ret.contents.number = instruction.fields.pair.arg1;
                vm_stack_push(vm, ret);

                // Value
                vm_stack_push(vm, vm->registers[instruction.fields.pair.arg1]);

                break;

            case OP_RESTORE:
                for (int i = 0; i < instruction.fields.pair.arg2; i++)
                {
                    // Value
                    result = vm_stack_pop(vm);

                    // Key
                    ret = vm_stack_pop(vm);

                    vm->registers[ret.contents.number] = result;
                }
                break;

            case OP_JMP:
                vm->pc = vm->registers[instruction.fields.pair.arg1].contents.number;
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
                    s1 = (string_t *)vm->registers[instruction.fields.triplet.arg2].contents.object;
                    s2 = (string_t *)vm->registers[instruction.fields.triplet.arg3].contents.object;
                    result.contents.boolean = !strcmp(s1->string, s2->string);
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
                    asprintf(&stmp, "%s%s", STRING3(arg2), STRING3(arg3));
                    result = string_create(stmp);
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
                    case VAL_ABSENT:
                        result.contents.boolean = true;
                        break;
                    case VAL_STRING:
                        result.contents.boolean = (bool)!strlen(STR2(arg2));
                        break;
                    default:
                        ;
                }
                vm->registers[instruction.fields.pair.arg1] = result;
                break;

            case OP_CALL:
                // First, push the return address down
                ret.type = VAL_INT;
                ret.contents.number = vm->pc;
                vm_cstack_push(vm, ret);
                vm->pc = vm->registers[instruction.fields.pair.arg1].contents.number;
                break;

            case OP_CALL_DYNAMIC:
                ret = memory_get(vm->memory, instruction.fields.pair.arg2);

                // Function names must be string values. Not sure how they wouldn't
                // be, so we assert here.
                assert(ret.type == VAL_STRING);

                s1 = (string_t *)ret.contents.object;

                char *builtin_name;
                asprintf(&builtin_name, "builtin__%s", s1->string);
                void (*builtin)(vm_t *);
                builtin = (void (*)(vm_t *))dlsym(RTLD_SELF, builtin_name);
                free(builtin_name);

                // TODO: Maintain a symbol map for future calls?

                // TODO: Proper error handling-- we couldn't find the supplied
                // runtime symbol
                assert(builtin != NULL);

                (*builtin)(vm);

                break;

            case OP_RETURN:
                ret = vm_cstack_pop(vm);
                vm_stack_push(vm, vm->registers[instruction.fields.pair.arg1]);
                vm->pc = ret.contents.number;
                break;

        }
    }
}

void vm_dump(vm_t *vm)
{
    printf("[memory contents]\n");
    for (int i = 0; i < vm->memory->capacity; i++)
    {
        if (vm->memory->contents[i].type == VAL_ABSENT)
            break;

        printf("   %04d ", i);
        value_print(vm->memory->contents[i]);
    }

    printf("\n[stack contents]\n");
    for (int i = 0; i < vm->sp; i++)
    {
        printf("   %04d ", i);
        value_print(vm->stack->contents[i]);
    }

    printf("\n[register contents]\n");
    for (int i = 1; i < VM_NUM_REGISTERS; i++)
    {
        if (vm->registers[i].type == VAL_ABSENT)
            break;

        printf("   %04d ", i);
        value_print(vm->registers[i]);
    }

    printf("\nstack pointer: %d\n", vm->sp);
}
