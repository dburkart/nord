/*
 * Copyright (c) 2021, Dana Burkart <dana.burkart@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <stdlib.h>
#include <string.h>

#include "value.h"

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
