/*
 * Copyright (c) 2021, Dana Burkart <dana.burkart@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "binary.h"

binary_t *binary_create(void)
{
    binary_t *binary = (binary_t *)calloc(1, sizeof(binary_t));

    binary->magic = 0xBABABEEF;
    binary->version = VERSION;

    return binary;
}
