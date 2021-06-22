/*
 * Copyright (c) 2021, Dana Burkart <dana.burkart@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <assert.h>
#include <stdio.h>
#include <time.h>

#include "machine/memory.h"
#include "machine/vm.h"

// Prints a value, followed by a newline.
// Returns true
void builtin__print(vm_t *vm)
{
    value_t result;
    value_t val = vm_stack_pop(vm);
    string_t *s1;

    switch(val.type)
    {
        case VAL_STRING:
            s1 = (string_t *)val.contents.object;
            printf("%s\n", s1->string);
            break;

        case VAL_INT:
            printf("%d\n", val.contents.number);
            break;

        case VAL_FLOAT:
            printf("%f\n", val.contents.real);
            break;

        case VAL_BOOLEAN:
            printf("%s\n", val.contents.boolean ? "true" : "false");
            break;

        case VAL_ABSENT:
            printf("nothing\n");
            break;

        case VAL_TUPLE:
            assert(false);
            break;
    }

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