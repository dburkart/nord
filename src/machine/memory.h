/*
 * Copyright (c) 2021, Dana Burkart <dana.burkart@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef MEMORY_H
#define MEMORY_H

#include <stdbool.h>
#include <stdlib.h>

#include "value.h"

typedef struct
{
    size_t capacity;
    value_t *contents;
} memory_t;

memory_t *memory_create(size_t size);
void memory_free(memory_t *);
void memory_set(memory_t *, int address, value_t val);
value_t memory_get(memory_t *, int address);

#endif
