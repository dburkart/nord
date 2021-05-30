/*
 * Copyright (c) 2021, Dana Burkart <dana.burkart@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "lexer.h"

int main(int argc, char *argv[])
{
	token_list_print(scan("var foo = 12\0"));

	return 0;
}
