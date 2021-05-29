/*
 * Copyright (c) 2021, Dana Burkart <dana.burkart@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "lexer.h"

TokenList token_list_create(size_t size)
{
	TokenList list;
	
	list.size = size;
	list.capacity = size;
	if (size)
	{
		list.tokens = calloc(size, sizeof(Token));
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