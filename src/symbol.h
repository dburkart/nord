/*
 * Copyright (c) 2021, Dana Burkart <dana.burkart@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef SYMBOL_H
#define SYMBOL_H

typedef struct
{
    enum {
        LOC_REGISTER, LOC_MEMORY
    } type;

    uint32_t address;
} location_t;

typedef struct
{
    char *name;
    location_t location;
} symbol_t;

typedef struct
{
    uint32_t size;
    uint32_t capacity;
    symbol_t *items;
} symbol_map_t;

symbol_map_t *symbol_map_create(void);
void symbol_map_destroy(symbol_map_t *);

// Adding / getting items
void symbol_map_set(symbol_map_t *, char *, location_t);
location_t symbol_map_get(symbol_map_t *, char *);

#endif
