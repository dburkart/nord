/*
 * Copyright (c) 2021, Dana Burkart <dana.burkart@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <stdio.h>

#include "token.h"

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
	if (list->size == list->capacity)
	{
		// Grow our capacity by 2x
		list->capacity = list->capacity * 2;
		list->tokens = realloc(list->tokens, sizeof(Token) * list->capacity);
	}

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
