/*
 * Copyright (c) 2021, Dana Burkart <dana.burkart@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef LEXER_H
#define LEXER_H

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

char *token_value(ScanContext *, Token);

// Scan an input string, and return a list of tokens.
TokenList scan_input(char *);

#endif
