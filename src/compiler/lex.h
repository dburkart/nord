/*
 * Copyright (c) 2021, Dana Burkart <dana.burkart@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef LEX_H
#define LEX_H

#include <stdbool.h>
#include <stdint.h>

#include "token.h"

// State for a single instance of a lexical scanner.
typedef struct {
    char *name;
    char *buffer;
    uint64_t position;
    token_t previous;
    token_t lookahead;
} scan_context_t;

// Return the next token for the given context
token_t accept(scan_context_t *);
// Return the next token, but don't consume it
token_t peek(scan_context_t *);
// Return whether or not the next token matches the specified tokens
bool match(scan_context_t *, int, ...);
// Rewind to the last seen token
void backup(scan_context_t *);

char *token_value(scan_context_t *, token_t);

// Scan an input string, and return a list of tokens.
token_list_t scan_input(char *, char *);

#endif
