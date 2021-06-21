/*
 * Copyright (c) 2021, Dana Burkart <dana.burkart@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef MEMORY_H
#define MEMORY_H

#include <stdbool.h>
#include <stdlib.h>

typedef struct v_t
{
    enum
    {
        VAL_NONE, VAL_INT, VAL_STRING,
        VAL_FLOAT, VAL_BOOLEAN, VAL_TUPLE
    } type;

    union
    {
        int number;
        float real;
        char *string;
        bool boolean;
        struct {
            struct v_t *first;
            struct v_t *second;
        } tuple;
    } contents;
} value_t;

typedef struct
{
    size_t capacity;
    value_t *contents;
} memory_t;

memory_t *memory_create(void);
void memory_free(memory_t *);
void memory_set(memory_t *, int address, value_t val);
value_t memory_get(memory_t *, int address);

#endif
