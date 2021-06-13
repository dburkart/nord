/*
 * Copyright (c) 2021, Dana Burkart <dana.burkart@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef BINARY_H
#define BINARY_H

#include <stdint.h>

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
        uint32_t text_offset;
        uint32_t code_offset;
        uint32_t reserved_1;
        uint32_t reserved_2;
    } sections;
    // The following fields are filled in on deserialization
    memory_t *text;
    code_block_t *code;
} binary_t;

binary_t *binary_create(void);

#endif