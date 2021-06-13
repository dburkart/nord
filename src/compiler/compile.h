/*
 * Copyright (c) 2021, Dana Burkart <dana.burkart@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef COMPILE_H
#define COMPILE_H

#include "machine/bytecode.h"
#include "machine/binary.h"
#include "parse.h"

binary_t *compile(ast_t *);

#endif
