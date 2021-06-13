/*
 * Copyright (c) 2021, Dana Burkart <dana.burkart@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "hash.h"

// PJW hash, also known as the Elf Hash. See wikipedia for more details.
// Note: This is _not_ a cryptographic hash, and is meant only for use in
//       our compiler hash maps.
uint64_t pjw_hash(const char *string)
{
    uint64_t hash = 0;
    uint64_t high_byte;
    while (*string)
    {
        // Shift the current hash over by one byte to make room for a new byte
        hash = (hash << 4) + *string;
        high_byte = hash & 0xF0000000;
        // If the high byte isn't 0, then move it 3/4 of the way down and XOR it
        if (high_byte)
        {
            hash ^= high_byte >> 48;
            hash &= ~high_byte;
        }

        // Advance string
        string += 1;
    }
    return hash;
}
