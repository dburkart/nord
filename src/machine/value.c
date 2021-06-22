/*
 * Copyright (c) 2021, Dana Burkart <dana.burkart@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <stdlib.h>

#include "value.h"

value_t make_string(char *string)
{
    value_t val;
    string_t *str = (string_t *)malloc(sizeof(string_t));

    str->object.type = VAL_STRING;
    str->string = string;

    val.type = VAL_STRING;
    val.contents.object = (object_t *)str;

    return val;
}
