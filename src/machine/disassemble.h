/*
 * Copyright (c) 2021, Dana Burkart <dana.burkart@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef DISASSEMBLE_H
#define DISASSEMBLE_H

#include "bytecode.h"

// Given a block of code, disassemble it into a human-readable string
char *disassemble(code_block_t *);

#endif
