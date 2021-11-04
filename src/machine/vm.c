/*
 * Copyright (c) 2021, Dana Burkart <dana.burkart@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "bytecode.h"
#include "disassemble.h"
#include "vm.h"
#include "value.h"
#include "util/dl.h"
#include "compiler/compile.h"
#include "compiler/parse.h"
#include "compiler/lex.h"

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
        case VAL_ITERATOR:
            printf("{ITERATOR}\n");
            break;
        case VAL_NIL:
            printf("{NIL}\n");
            break;
        case VAL_FUNCTION:
            printf("{FUNCTION}\n");
            break;
        case VAL_MODULE:
            printf("{MODULE}\n");
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

    vm->symbols = binary->symbols;

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
        tuple_t *t1;
        iterator_t *iter;
        char *stmp;
        memory_t *mem;
        function_t *fn;

//        printf("%d: %s", vm->pc, disassemble_instruction(vm->memory, instruction));
//        getchar();

        switch (instruction.opcode)
        {
            case OP_NIL:
                result.type = VAL_NIL;
                vm->registers[instruction.fields.pair.arg1] = result;
                break;

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
                result.contents.number = instruction.fields.pair_signed.arg2;
                vm->registers[instruction.fields.pair.arg1] = result;
                break;

            case OP_STORE:
                memory_set(vm->memory, instruction.fields.pair.arg1, vm->registers[instruction.fields.pair.arg2]);
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

            case OP_JMP:
                vm->pc += vm->registers[instruction.fields.pair.arg1].contents.number;
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
                else if (REG_TYPE3(arg2, VAL_NIL) && REG_TYPE3(arg3, VAL_NIL))
                {
                    result.contents.boolean = true;
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

            case OP_AND:
                if (IS_NUMBERISH3(arg2) && IS_NUMBERISH3(arg3))
                {
                    result.contents.boolean = NUM_OR_FLOAT_OR_BOOL3(arg2) && NUM_OR_FLOAT_OR_BOOL3(arg3);
                }
                else
                {
                    // What do we do for non-numbers?
                }
                result.type = VAL_BOOLEAN;

                vm->registers[instruction.fields.triplet.arg1] = result;
                break;

            case OP_OR:
                if (IS_NUMBERISH3(arg2) && IS_NUMBERISH3(arg3))
                {
                    result.contents.boolean = NUM_OR_FLOAT3(arg2) || NUM_OR_FLOAT3(arg3);
                }
                else
                {
                    // What do we do for non-numbers?
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

            case OP_DEREF:
                ret = vm->registers[instruction.fields.triplet.arg2];
                // TODO: Error handling!
                assert(ret.type == VAL_ITERATOR);
                iter = (iterator_t *)ret.contents.object;

                if (iter->index == iter->length)
                {
                    result.type = VAL_NIL;
                    vm->registers[instruction.fields.pair.arg1] = result;
                    break;
                }

                ret = iter->iterable;
                assert(is_collection(ret));
                // First store the value referenced by the iterator in arg1
                switch (ret.type)
                {
                    case VAL_TUPLE:
                        t1 = (tuple_t *)ret.contents.object;
                        result = t1->values[iter->index];
                        break;

                    case VAL_STRING:
                        // TODO: Handle
                        break;

                    default:
                        ;
                };

                iter->index += instruction.fields.triplet.arg3;
                vm->registers[instruction.fields.pair.arg1] = result;
                break;

            case OP_CALL:
                ret = memory_get(vm->memory, instruction.fields.pair.arg2);

                // This must be of type VAL_FUNCTION
                assert(ret.type == VAL_FUNCTION);

                // Save the current frame if it's set
                if (vm->frame.type == VAL_FUNCTION)
                {
                    vm_cstack_push(vm, vm->frame);
                }

                // Make a copy so we don't pollute the function definition later
                fn = (function_t *)ret.contents.object;
                ret = function_def_create(
                    fn->name,
                    fn->addr,
                    fn->nargs,
                    fn->locals,
                    fn->low_reg
                );

                vm->frame = ret;

                fn = (function_t *)vm->frame.contents.object;

                // First, set the return address in the current frame
                fn->return_addr = vm->pc;

                // Now set the program counter
                vm->pc = fn->addr;

                // Initialize save buffer, if null
                if (fn->save == NULL)
                {
                    uint8_t count = 0;

                    while (fn->locals[count] != 0)
                    {
                        count += 1;
                    }

                    fn->save = (value_t *)malloc(sizeof(value_t) * count);
                }

                // Finally, save out locals
                for (uint8_t i = fn->nargs, reg = fn->locals[i]; reg != 0; i++, reg = fn->locals[i])
                {
                    fn->save[i] = vm->registers[reg];
                }

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
                builtin = (void (*)(vm_t *))dynamic_load_self(builtin_name);
                free(builtin_name);

                // TODO: Maintain a symbol map for future calls?

                // TODO: Proper error handling-- we couldn't find the supplied
                // runtime symbol
                assert(builtin != NULL);

                (*builtin)(vm);

                break;

            case OP_RETURN:
                fn = (function_t *)vm->frame.contents.object;

                vm->pc = fn->return_addr;
                vm_stack_push(vm, vm->registers[instruction.fields.pair.arg1]);

                // Restore our save buffer
                for (uint8_t i = 0, reg = fn->locals[i]; reg != 0; i++, reg = fn->locals[i])
                {
                    vm->registers[reg] = fn->save[i];
                }

                // Free our save buffer
                free(fn->save);
                free(fn);
                vm->frame = (value_t){VAL_ABSENT};

                // If our call stack has something on it, pop it into the frame register
                if (vm->csp > 0)
                {
                    vm->frame = vm_cstack_pop(vm);
                }

                break;

            case OP_IMPORT:
                ret = memory_get(vm->memory, instruction.fields.pair.arg1);

                // The only valid argument to an import statement is a string
                assert(ret.type == VAL_STRING);

                s1 = (string_t *)ret.contents.object;

                char *filepath;
                asprintf(&filepath, "%s.n", s1->string);

                // TODO: This seems duplicated...
                FILE *fp = fopen(filepath, "r");

                fseek(fp, 0, SEEK_END);
                long fsize = ftell(fp);
                fseek(fp, 0, SEEK_SET);

                char *input = malloc(fsize + 1);
                fread(input, 1, fsize, fp);
                fclose(fp);

                input[fsize] = 0;

                scan_context_t context;
                context.name = filepath;
                context.buffer = input;
                context.position = 0;

                ast_t *syntax_tree = parse(&context);
                binary_t *binary = compile(filepath, input, syntax_tree);
                vm_t *module_vm = vm_create(binary);
                vm_execute(module_vm);

                result = module_create(s1->string, (struct vm_t *)module_vm);

                // TODO: This definitely leaks memory, clean this up when we
                //  implement garbage collection
                memory_set(vm->memory, instruction.fields.pair.arg1, result);

                symbol_t sym;

                sym.name = s1->string;
                sym.type = SYM_MODULE;
                sym.location.type = LOC_MEMORY;
                sym.location.address = instruction.fields.pair.arg1;

                symbol_map_set(vm->symbols, sym);

                free(input);

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
