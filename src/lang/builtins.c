/*
 * Copyright (c) 2021, Dana Burkart <dana.burkart@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <assert.h>
#include <stdio.h>
#include <time.h>

#include "machine/memory.h"
#include "machine/value.h"
#include "machine/vm.h"

void print_internal(value_t val)
{
    string_t *s1;
    tuple_t *t1;

    switch(val.type)
    {
        case VAL_STRING:
            s1 = (string_t *)val.contents.object;
            printf("%s", s1->string);
            break;

        case VAL_INT:
            printf("%d", val.contents.number);
            break;

        case VAL_FLOAT:
            printf("%f", val.contents.real);
            break;

        case VAL_BOOLEAN:
            printf("%s", val.contents.boolean ? "true" : "false");
            break;

        case VAL_ABSENT:
            printf("nothing");
            break;

        case VAL_TUPLE:
            t1 = (tuple_t *)val.contents.object;
            printf("(");
            for (int i = 0; i < t1->length; i++)
            {
                print_internal(t1->values[i]);
                if (i < t1->length - 1)
                    printf(", ");
            }
            printf(")");
            break;
    }
}

// Prints a value, followed by a newline.
// Returns true
void builtin__print(vm_t *vm)
{
    int num_args = vm->registers[0].contents.number;

    if (!num_args)
    {
        printf("\n");
        goto print_ret;
    }

    value_t result;
    value_t val = vm_stack_pop(vm);

    print_internal(val);
    printf("\n");

print_ret:
    result.type = VAL_BOOLEAN;
    result.contents.boolean = true;

    vm_stack_push(vm, result);
}

void builtin__time(vm_t *vm)
{
    value_t result;
    time_t tm = time(NULL);

    result.type = VAL_INT;
    result.contents.number = tm;

    vm_stack_push(vm, result);
}

void builtin__tuple(vm_t *vm)
{
    assert(vm->registers[0].type == VAL_INT);
    int num_args = vm->registers[0].contents.number;
    value_t val = tuple_create(num_args);
    tuple_t *tuple = (tuple_t *)val.contents.object;

    for (int i = 0; i < num_args; i++)
    {
        tuple->values[i] = vm_stack_pop(vm);
    }

    vm_stack_push(vm, val);
}

// -- Type handling

void builtin__type(vm_t *vm)
{
    // Error handling!
    assert(vm->registers[0].contents.number == 1);
    value_t val = vm_stack_pop(vm);
    switch (val.type)
    {
        case VAL_FLOAT:
            vm_stack_push(vm, string_create("float"));
            break;

        case VAL_INT:
            vm_stack_push(vm, string_create("integer"));
            break;

        case VAL_BOOLEAN:
            vm_stack_push(vm, string_create("boolean"));
            break;

        case VAL_STRING:
            vm_stack_push(vm, string_create("string"));
            break;

        case VAL_TUPLE:
            vm_stack_push(vm, string_create("tuple"));
            break;

        case VAL_ABSENT:
            assert(false);
            break;
    }
}

void builtin__int(vm_t *vm)
{
    value_t val = vm_stack_pop(vm);
    switch(val.type)
    {
        case VAL_FLOAT:
            val.contents.number = (int)val.contents.real;
            val.type = VAL_INT;
            vm_stack_push(vm, val);
            break;

        case VAL_BOOLEAN:
            val.contents.number = (val.contents.boolean) ? 1 : 0;
            val.type = VAL_INT;
            vm_stack_push(vm, val);
            break;

        // TODO: Handle strings
        default:
            break;
    }
}

void builtin__string(vm_t *vm)
{
    char *str;
    value_t val = vm_stack_pop(vm);
    switch(val.type)
    {
        case VAL_INT:
            asprintf(&str, "%d", val.contents.number);
            vm_stack_push(vm, string_create(str));
            break;

        case VAL_FLOAT:
            asprintf(&str, "%f", val.contents.real);
            vm_stack_push(vm, string_create(str));
            break;

        case VAL_BOOLEAN:
            asprintf(&str, "%s", (val.contents.boolean) ? "true" : "false");
            vm_stack_push(vm, string_create(str));
            break;

        // TODO: Handle strings
        default:
            break;
    }
}