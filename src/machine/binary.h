/*
 * Copyright (c) 2021, Dana Burkart <dana.burkart@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef BINARY_H
#define BINARY_H

#include <stdint.h>

#include "compiler/symbol.h"
#include "bytecode.h"
#include "memory.h"

#define VERSION            1

typedef struct
{
    // 0xBABABEEF
    uint32_t magic;
    uint16_t version;
    uint16_t reserved;
    // The 'sections' structure is used for serialization / deserialization of
    // binaries. When creating a new binary, this field can be ignored.
    struct {
        uint32_t data_offset;
        uint32_t code_offset;
        uint32_t reserved_1;
        uint32_t reserved_2;
    } sections;
    // The following fields are filled in on deserialization
    memory_t *data;
    // Packaged code is relocatable "chunks". Currently these are functions, but
    // in theory these could be any chunk of distinct code.
    code_collection_t *packaged_code;
    code_collection_t *main_code;
    symbol_map_t *symbols;
} binary_t;

binary_t *binary_create(void);
binary_t *binary_load(const char *);
void binary_write(binary_t *binary, const char *);

#endif
