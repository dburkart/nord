/*
 * Copyright (c) 2021, Dana Burkart <dana.burkart@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

 #ifndef TOKEN_H
 #define TOKEN_H

#include <stdlib.h>
#include <stdint.h>

// Lexeme Enumeration
enum TokenType {
    // Delimiters
    L_PAREN, R_PAREN, COLON, COMMA, EOL,

    EQUAL, BANG,

    // Math
    PLUS, MINUS, ASTERISK, SLASH,

    // Comparators
    GREATER, GREATER_OR_EQUAL, LESS, LESS_OR_EQUAL, EQUAL_EQUAL, BANG_EQUAL,

    // Arrows
    R_ARROW,

    // Literals
    IDENTIFIER, STRING, NUMBER,

    // Keywords
    VAR, FUNCTION, TRUE, FALSE, NIL,

    // Invalid token
    INVALID,

    EOF_CHAR // 'EOF' is reserved on some platforms
};

// Token specific information
typedef struct {
    enum TokenType type;
    // Positional data, relative to the original buffer
    uint64_t start;
    uint64_t end;
} Token;

// List of tokens to be fed to the parser
typedef struct {
    size_t size;
    size_t capacity;
    Token *tokens;
} TokenList;

TokenList token_list_create(size_t);
void token_list_destroy(TokenList);
void token_list_add(TokenList *, Token);
void token_list_print(TokenList);
const char *token_name(Token);

#endif
