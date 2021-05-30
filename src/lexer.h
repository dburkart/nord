/*
 * Copyright (c) 2021, Dana Burkart <dana.burkart@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

 #include <stdlib.h>

// Lexeme Enumeration
enum TokenType {
	EQUAL,

	IDENTIFIER, STRING, NUMERAL,

	VAR, FUNCTION,

	EOF_CHAR
};

typedef struct {
	enum TokenType type;
} Token;

typedef struct {
	size_t size;
	size_t capacity;
	Token *tokens;
} TokenList;

TokenList token_list_create(size_t size);
void token_list_free(TokenList *);
void token_list_add(TokenList *, Token);
void token_list_print(TokenList);

TokenList scan(char *);
