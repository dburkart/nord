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
enum token_type_e {
    // Delimiters
    TOK_L_PAREN, TOK_R_PAREN, TOK_L_BRACE, TOK_R_BRACE, TOK_COLON, TOK_COMMA,
    TOK_EOL,

    TOK_EQUAL, TOK_BANG,

    // Math
    TOK_PLUS, TOK_MINUS, TOK_ASTERISK, TOK_SLASH,

    // Comparators
    TOK_GREATER, TOK_GREATER_EQUAL, TOK_LESS, TOK_LESS_EQUAL, TOK_EQUAL_EQUAL,
    TOK_BANG_EQUAL,

    // Arrows
    TOK_R_ARROW,

    // Literals
    TOK_IDENTIFIER, TOK_STRING, TOK_NUMBER, TOK_FLOAT,

    // Keywords
    TOK_VAR, TOK_FN, TOK_RETURN, TOK_TRUE, TOK_FALSE, TOK_NIL,

    // Branching
    TOK_IF,

    // Invalid token
    TOK_INVALID,

    TOK_EOF // 'EOF' is reserved on some platforms
};

// Token specific information
typedef struct {
    enum token_type_e type;
    // Positional data, relative to the original buffer
    uint64_t start;
    uint64_t end;
} token_t;

// List of tokens to be fed to the parser
typedef struct {
    size_t size;
    size_t capacity;
    token_t *tokens;
} token_list_t;

token_list_t token_list_create(size_t);
void token_list_destroy(token_list_t);
void token_list_add(token_list_t *, token_t);
void token_list_print(token_list_t);
const char *token_name(token_t);

#endif
