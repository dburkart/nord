/*
 * Copyright (c) 2021, Dana Burkart <dana.burkart@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef VALUE_H
#define VALUE_H

#include <stdbool.h>
#include <stdint.h>

#define value(x) _Generic((x),               \
                    int: value_from_int,     \
                    float: value_from_float, \
                    double: value_from_float,\
                    char *: string_create    \
                    )(x)

struct obj_t;
struct vm_t;

// Enumeration of value types supported by nord
typedef enum
{
    VAL_ABSENT,
    VAL_NIL,
    VAL_INT,
    VAL_STRING,
    VAL_FLOAT,
    VAL_BOOLEAN,
    VAL_TUPLE,
    VAL_ITERATOR,
    VAL_FUNCTION,
    VAL_MODULE,
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
    int length;
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

typedef struct
{
    object_t object;
    char *name;
    uint32_t addr;           // Where this function lives in code
    uint32_t return_addr;    // Return address
    uint8_t nargs;           // number of args
    uint8_t *locals;         // Which registers are used by this function, 0 terminated
    uint8_t low_reg;
    value_t *save;           // Registers to restore upon return
} function_t;

value_t function_def_create(char *name, uint32_t address, uint8_t nargs, uint8_t *locals, uint8_t low);

typedef struct
{
    object_t object;
    char *name;
    struct vm_t *vm;
} module_t;

value_t module_create(char *name, struct vm_t *vm);

static inline value_t value_from_int(int x)
{
    return (value_t){ .type=VAL_INT, .contents={ .number=x } };
}

static inline value_t value_from_float(double x)
{
    return (value_t){ .type=VAL_INT, .contents={ .real=x } };
}

#endif
