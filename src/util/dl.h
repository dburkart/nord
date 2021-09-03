/*
 * Copyright (c) 2021, Dana Burkart <dana.burkart@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef DL_H
#define DL_H

#include "util/platform.h"

/*
 * Platform-agnostic way to load in symbols from the current
 * executable.
 */
void *dynamic_load_self(const char *symbol);

#endif
