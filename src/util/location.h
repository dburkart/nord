/*
 * Copyright (c) 2021, Dana Burkart <dana.burkart@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef LOCATION_H
#define LOCATION_H

#include <stdint.h>

typedef struct
{
    uint64_t start;
    uint64_t end;
} location_t;

#endif
