/*
 * Copyright (c) 2021, Dana Burkart <dana.burkart@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef SYMBOL_H
#define SYMBOL_H

typedef enum {
    SYM_VAR,
    SYM_CONSTANT,
    SYM_FN,
    SYM_MODULE,
} sym_type_e;

// Location struct for a symbol. Details whether the symbol is in a register
// or in memory, and where.
typedef struct
{
    enum {
        LOC_UNDEF,
        LOC_REGISTER,
        LOC_MEMORY,
        LOC_CODE,
        LOC_BUILTIN,
        // Used to indicate a symbol has been declared but not assigned
        LOC_NONE
    } type;

    uint32_t address;
} sym_pointer_t;

// Symbol struct, contains the symbol name and location
typedef struct
{
    char *name;
    sym_type_e type;
    sym_pointer_t location;
} symbol_t;

// Symbol hash map, containing an array of symbols
typedef struct sym_map_t
{
    // We keep track of the number of elements in our map to know when to grow
    uint32_t size;
    uint32_t capacity;
    symbol_t *items;
    struct sym_map_t *parent;
} symbol_map_t;

symbol_map_t *symbol_map_create(void);
void symbol_map_destroy(symbol_map_t *);

// Adding / getting items
void symbol_map_set(symbol_map_t *, symbol_t);
symbol_t symbol_map_get_local(symbol_map_t *symbol_map, char *name);
symbol_t symbol_map_get(symbol_map_t *, char *);
symbol_map_t *symbol_map_context(symbol_map_t *symbol_map, char *name);

#endif
