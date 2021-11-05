/*
 * Copyright (c) 2021, Dana Burkart <dana.burkart@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "value.h"

bool is_collection(value_t value)
{
    return value.type == VAL_STRING || value.type == VAL_TUPLE;
}

value_t iterator_create(value_t collection)
{
    // TODO: Handle errors
    assert(is_collection(collection));

    value_t val;
    iterator_t *iter = (iterator_t *)malloc(sizeof(iterator_t));

    iter->object.type = VAL_ITERATOR;
    iter->iterable = collection;
    iter->index = 0;

    switch (collection.type)
    {
        case VAL_STRING:
            iter->length = ((string_t *)collection.contents.object)->length;
            break;

        case VAL_TUPLE:
            iter->length = ((tuple_t *)collection.contents.object)->length;
            break;

        default:
            ;
    }

    val.type = VAL_ITERATOR;
    val.contents.object = (object_t *)iter;

    return val;
}

value_t string_create(char *string)
{
    value_t val;
    string_t *str = (string_t *)malloc(sizeof(string_t));

    str->object.type = VAL_STRING;
    str->length = strlen(string);
    str->string = string;

    val.type = VAL_STRING;
    val.contents.object = (object_t *)str;

    return val;
}

value_t tuple_create(int length)
{
    value_t val;
    tuple_t *tuple = (tuple_t *)malloc(sizeof(tuple_t));

    tuple->object.type = VAL_TUPLE;
    tuple->length = length;
    tuple->values = (value_t *)malloc(sizeof(value_t) * length);

    val.type = VAL_TUPLE;
    val.contents.object = (object_t *)tuple;

    return val;
}

value_t function_def_create(char *name, address_t address, uint8_t nargs, uint8_t *locals, uint8_t low)
{
    value_t val;
    function_t *func = (function_t *)malloc(sizeof(function_t));

    func->object.type = VAL_FUNCTION;
    func->name = name;
    func->address = address;
    func->nargs = nargs;
    func->locals = locals;
    func->low_reg = low;
    func->save = NULL;

    val.type = VAL_FUNCTION;
    val.contents.object = (object_t *)func;

    return val;
}

value_t module_create(char *name, struct vm_t *vm)
{
    value_t val;

    module_t *module = (module_t *)malloc(sizeof(module_t));

    module->name = name;
    module->vm = vm;

    val.type = VAL_MODULE;
    val.contents.object = (object_t *)module;

    return val;
}
