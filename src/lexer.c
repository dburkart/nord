/*
 * Copyright (c) 2021, Dana Burkart <dana.burkart@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <assert.h>

#include "lexer.h"

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
	TokenList tokens = token_list_create(2);

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

