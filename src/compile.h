/*
 * Copyright (c) 2021, Dana Burkart <dana.burkart@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef COMPILE_H
#define COMPILE_H

#include "bytecode.h"
#include "parse.h"

code_block_t *compile(ast_t *);

#endif
