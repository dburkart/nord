/*
 * Copyright (c) 2021, Dana Burkart <dana.burkart@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */
 
 #include <stdlib.h>

// Lexeme Enumeration
enum TokenType {
	EQUAL, SEMICOLON,
	
	IDENTIFIER, STRING, NUMERAL,
	
	VAR, DEF,
	
	EOF
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


// token_list_t scan_string(char *);