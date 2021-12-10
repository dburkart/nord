/*
 * Copyright (c) 2021, Dana Burkart <dana.burkart@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef MATCH_H
#define MATCH_H

#include <stdbool.h>

bool is_reserved(char);
bool is_whitespace(char);
bool is_boundary(char);

int match_keyword(const char *to_match, const char *c, int len);

#endif
