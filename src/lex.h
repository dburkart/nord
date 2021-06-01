/*
 * Copyright (c) 2021, Dana Burkart <dana.burkart@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef LEXER_H
#define LEXER_H

#include "token.h"

// Scan an input string, and return a list of tokens.
TokenList scan(char *);

#endif
