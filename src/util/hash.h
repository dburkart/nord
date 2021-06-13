/*
 * Copyright (c) 2021, Dana Burkart <dana.burkart@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef HASH_H
#define HASH_H

#include <stdint.h>

// PJW hash function -- this is a non-cryptographic hash used in our symbol table
uint64_t pjw_hash(const char *);

#endif
