/*
 * Copyright (c) 2021, Dana Burkart <dana.burkart@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef VALUE_H
#define VALUE_H

#include <stdbool.h>

struct obj_t;

// Enumeration of value types supported by nord
typedef enum
{
    VAL_ABSENT,
    VAL_INT,
    VAL_STRING,
    VAL_FLOAT,
    VAL_BOOLEAN,
    VAL_TUPLE,
    VAL_ITERATOR
} value_type_e;

// The value_t struct encapsulates all possible values in the nord language.
// For simple primitives, like integers, floats, and booleans the value is
// held in the structure. For more complex types, we use struct punning to
// approximate "inheritance".
typedef struct
{
    value_type_e type;
    union
    {
        int number;
        float real;
        bool boolean;
        struct obj_t *object;
    } contents;
} value_t;

// All complex types inherit from object_t.
typedef struct obj_t
{
    value_type_e type;
} object_t;

// Iterator
typedef struct
{
    object_t object;
    int index;
    value_t iterable;
} iterator_t;

value_t iterator_create(value_t collection);

// Returns whether or not the specified value is a collection
bool is_collection(value_t value);

// String object
typedef struct
{
    object_t object;
    int length;
    char *string;
} string_t;

value_t string_create(char *string);

typedef struct
{
    object_t object;
    int length;
    value_t *values;
} tuple_t;

value_t tuple_create(int length);

#endif