/*
 * Copyright (c) 2021, Dana Burkart <dana.burkart@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef ERROR_H
#define ERROR_H

#include "location.h"

char *format_error(const char *listing_name, const char *listing, const char *str, location_t loc);

#endif
