/*
 * Copyright (c) 2021, Dana Burkart <dana.burkart@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <stdlib.h>
#include <string.h>

#include "hash.h"
#include "symbol.h"

symbol_map_t *symbol_map_create(void)
{
    symbol_map_t *symbol_map = malloc(sizeof(symbol_map_t));
    // The capacity of a symbol map must always be a power of 2
    symbol_map->capacity = 8;
    symbol_map->size = 0;
    // We want items to be zero'd out so that we can test for existence
    symbol_map->items = calloc(symbol_map->capacity, sizeof(symbol_t));
    return symbol_map;
}

void symbol_map_destroy(symbol_map_t *symbol_map)
{
    // First, free our items
    free(symbol_map->items);
    free(symbol_map);
}

void symbol_map_set(symbol_map_t *symbol_map, char *name, location_t loc)
{
    // When we are 50% full or more, we grow the map. Why 50%? We never want
    // our hash map to get too full since that would result in more collisions
    // which in turn would slow down performance. 50% is nice and round, and
    // will give us good average-case performance, at the cost of using more
    // memory. Maybe we'll tune this if we need to squeeze out better memory
    // performance.
    if ((symbol_map->size + 1) / (float)symbol_map->capacity >= .5)
    {
        symbol_t *new_items;
        uint32_t new_capacity = symbol_map->capacity * 2;
        new_items = calloc(new_capacity, sizeof(symbol_t));

        for (int i = 0; i < symbol_map->capacity; i++)
        {
            symbol_t symbol = symbol_map->items[i];
            if (symbol.name != NULL)
            {
                new_items[pjw_hash(symbol.name) & (new_capacity - 1)] = symbol;
            }
        }

        free(symbol_map->items);
        symbol_map->capacity = new_capacity;
        symbol_map->items = new_items;
    }

    symbol_t symbol = {name, loc};
    // Because our capacity will always be a power of 2, we can use a bitwise
    // AND to compute modulo.
    uint32_t index = pjw_hash(name) & (symbol_map->capacity - 1);

    // Collision handling, simply look for the next free spot
    while (symbol_map->items[index].name != NULL)
    {
        index += 1;
    }

    symbol_map->items[index] = symbol;
    symbol_map->size += 1;
}

location_t symbol_map_get(symbol_map_t *symbol_map, char *name)
{
    uint32_t index = pjw_hash(name) & (symbol_map->capacity - 1);
    symbol_t symbol = symbol_map->items[index];
    // If we have a collision, advance until we find the correct symbol
    while (strcmp(symbol.name, name))
    {
        index += 1;
        // TODO: Should we really be assigning here?
        symbol = symbol_map->items[index];
    }

    return symbol.location;
}
