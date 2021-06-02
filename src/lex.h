/*
 * Copyright (c) 2021, Dana Burkart <dana.burkart@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef LEX_H
#define LEX_H

#include <stdbool.h>

#include "token.h"

// State for a single instance of a lexical scanner.
typedef struct {
    char *buffer;
    int position;
    Token lookahead;
} ScanContext;

// Return the next token for the given context
Token accept(ScanContext *);
// Return the next token, but don't consume it
Token peek(ScanContext *);
// Return whether or not the next token matches the specified tokens
bool match(ScanContext *, int, ...);

char *token_value(ScanContext *, Token);

// Scan an input string, and return a list of tokens.
TokenList scan_input(char *);

#endif
