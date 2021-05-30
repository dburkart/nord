/*
 * Copyright (c) 2021, Dana Burkart <dana.burkart@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <assert.h>
#include <stdio.h>

#include "lexer.h"

TokenList token_list_create(size_t capacity)
{
	TokenList list;

	list.size = 0;
	list.capacity = capacity;
	if (capacity)
	{
		list.tokens = calloc(capacity, sizeof(Token));
	}
	else
	{
		list.tokens = NULL;
	}

	return list;
}

void token_list_free(TokenList *list)
{
	if (list->tokens)
	{
		free(list->tokens);
	}
}

void token_list_add(TokenList *list, Token t)
{
	// TODO: Grow our token list if necessary
	list->tokens[list->size] = t;
	list->size = list->size + 1;
}

void token_list_print(TokenList list)
{
	for (int i = 0; i < list.size; i++)
	{
		Token t = list.tokens[i];
		switch(t.type)
		{
			case EQUAL:
				printf("[EQUAL] ");
				break;
			case IDENTIFIER:
				printf("[IDENTIFIER] ");
				break;
			case STRING:
				printf("[STRING] ");
				break;
			case NUMERAL:
				printf("[NUMERAL] ");
				break;
			case VAR:
				printf("[VAR] ");
				break;
			case FUNCTION:
				printf("[FUNCTION] ");
				break;
			case EOF_CHAR:
				printf("[EOF] ");
				break;
		}
	}

	printf("\n");
}

int match_var(const char *c, TokenList *list)
{
	if (*c != 'v') return 0;
	if (*(c+1) != 'a') return 0;
	if (*(c+2) != 'r') return 0;

	Token t = { VAR };
	token_list_add(list, t);

	return 3;
}

int match_identifier(const char *c, TokenList *list)
{
	int len = 0;

	// Identifiers must start with a letter, so we do some bounds checking
	if (*c < 'A')
		return 0;

	if (*c > 'Z' && *c < 'a')
		return 0;

	if (*c > 'z')
		return 0;

	while (*c != ' ' && *c != '\t' && *c != '\n' && *c != '\0')
	{
		len = len + 1;
		c = c + 1;
	}

	Token t = { IDENTIFIER };
	token_list_add(list, t);

	return len;
}

int match_numeral(const char *c, TokenList *list)
{
	int len = 0;

	while (*c != ' ' && *c != '\t' && *c != '\n' && *c != '\0')
	{
		if (*c < '0' || *c > '9')
			return 0;

		len = len + 1;
		c = c + 1;
	}

	Token t = { NUMERAL };
	token_list_add(list, t);

	return len;
}

TokenList scan(char *input)
{
	// We just create an arbitrarily-sized token list to begin with
	TokenList tokens = token_list_create(10);

	char *c = input;
	while (*c != '\0')
	{
		int advance = 0;
		Token t;
		switch (*c)
		{
			case ' ':
			case '\t':
			case '\n':
				advance = 1;
				break;
			case '=':
				t.type = EQUAL;
				token_list_add(&tokens, t);
				advance = 1;
				break;
			case 'v':
				advance = match_var(c, &tokens);
				if (advance)
					break;
			default:
				advance = match_identifier(c, &tokens);
				if (advance)
					break;

				advance = match_numeral(c, &tokens);
				if (advance)
					break;
		}

		// TODO: Better Error handling here
		assert(advance != 0);

		c += advance;
	}

	return tokens;
}

