/*
 * Copyright (c) 2021, Dana Burkart <dana.burkart@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

 #ifndef TOKEN_H
 #define TOKEN_H

#include <stdlib.h>

// Lexeme Enumeration
enum TokenType {
	// Single-character tokens
	EQUAL, L_PAREN, R_PAREN, COLON, COMMA, EOL,

	// Arrows
	R_ARROW,

	// Literals
	IDENTIFIER, STRING, NUMERAL,

	// Keywords
	VAR, FUNCTION,

	EOF_CHAR // 'EOF' is reserved on some platforms
};

// Token specific information
typedef struct {
	enum TokenType type;
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

#endif
