/*
 * Copyright (c) 2021, Dana Burkart <dana.burkart@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lex.h"

/*
 * Return whether or not the specified character is reserved and shouldn't be
 * allowed in an identifier.
 */
bool is_reserved(char c)
{
    if (c == '(' || c == ')' || c == ':' || c == ',' || c == '*' || c == '+' ||
        c == '/' || c == '-')
        return true;
    return false;
}

/*
 * Return whether or not the specified character is whitespace
 */
bool is_whitespace(char c)
{
    if (c == ' ' || c == '\t' || c == '\n')
        return true;
    return false;
}

///---- Matching functions

/*
 * Match 'var' keyword
 */
int match_var(const char *c)
{
    if (*c != 'v') return 0;
    if (*(c+1) != 'a') return 0;
    if (*(c+2) != 'r') return 0;

    return 3;
}

/*
 * Match 'true' keyword
 */
int match_true(const char *c)
{
    if (*c != 't') return 0;
    if (*(c+1) != 'r') return 0;
    if (*(c+2) != 'u') return 0;
    if (*(c+3) != 'e') return 0;

    return 4;
}

/*
 * Match 'false' keyword
 */
int match_false(const char *c)
{
    if (*c != 'f') return 0;
    if (*(c+1) != 'a') return 0;
    if (*(c+2) != 'l') return 0;
    if (*(c+3) != 's') return 0;
    if (*(c+4) != 'e') return 0;

    return 5;
}

/*
 * Match 'nil' keyword
 */
int match_nil(const char *c)
{
    if (*c != 'n') return 0;
    if (*(c+1) != 'i') return 0;
    if (*(c+2) != 'l') return 0;

    return 3;
}

/*
 * Match 'fn' keyword
 */
int match_fn(const char *c)
{
    if (*c != 'f') return 0;
    if (*(c+1) != 'n') return 0;

    return 2;
}

/*
 * Match an identifier. Identifiers must begin with an alphabetic character,
 * and can contain any non-reserved character.
 */
int match_identifier(const char *c)
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
        if (is_reserved(*c))
            break;

        len = len + 1;
        c = c + 1;
    }

    return len;
}

/*
 * Match a number.
 */
int match_number(const char *c)
{
    int len = 0;

    while (!is_whitespace(*c) && *c != 0 && !is_reserved(*c))
    {
        if (*c < '0' || *c > '9')
            return 0;

        len = len + 1;
        c = c + 1;
    }

    return len;
}

/*
 * Match a string.
 */
int match_string(const char *c)
{
    int len = 1;

    if (*c != '"')
        return 0;

    do {
        len = len + 1;
        c = c + 1;
    } while (*c != '"');

    return len;
}

/*
 * Match the -> keyword.
 */
int match_right_arrow(const char *c)
{
    if (*c != '-') return 0;
    if (*(c+1) != '>') return 0;

    return 2;
}

/*
 * Return the next token on the input stream, without advancing the scan position
 */
Token peek(ScanContext *context)
{
    int start;
    int position = context->position;
    int advance = 0;
    Token t;
    bool token_found = true;

    // If our lookahead token has already been calculated, return it
    if (context->lookahead.start >= position && position > 0)
        return context->lookahead;

    do {
        char *c = context->buffer + position;

        start = position;
        // We don't set to true in the switch statement below
        token_found = true;
        switch (*c)
        {
            case ' ':
            case '\t':
                advance = 1;
                token_found = false;
                break;
            case '\0':
                t.type = EOF_CHAR;
                advance = 1;
                break;
            case '\n':
                t.type = EOL;
                advance = 1;
                break;
            case '=':
                if (*(c+1) == '=')
                {
                    t.type = EQUAL_EQUAL;
                    advance = 2;
                    break;
                }

                t.type = EQUAL;
                advance = 1;
                break;
            case '!':
                if (*(c+1) == '=')
                {
                    t.type = BANG_EQUAL;
                    advance = 2;
                    break;
                }

                t.type = BANG;
                advance = 1;
                break;
            case '(':
                t.type = L_PAREN;
                advance = 1;
                break;
            case ')':
                t.type = R_PAREN;
                advance = 1;
                break;
            case ':':
                t.type = COLON;
                advance = 1;
                break;
            case ',':
                t.type = COMMA;
                advance = 1;
                break;
            case '-':
                advance = match_right_arrow(c);
                if (advance)
                {
                    t.type = R_ARROW;
                    break;
                }

                t.type = MINUS;
                advance = 1;
                break;
            case '+':
                t.type = PLUS;
                advance = 1;
                break;
            case '*':
                t.type = ASTERISK;
                advance = 1;
                break;
            case '/':
                t.type = SLASH;
                advance = 1;
                break;
            case '"':
                advance = match_string(c);
                if (advance)
                {
                    t.type = STRING;
                    break;
                }
            case 'f':
                advance = match_fn(c);
                if (advance)
                {
                    t.type = FUNCTION;
                    break;
                }

                advance = match_false(c);
                if (advance)
                {
                    t.type = FALSE;
                    break;
                }
            case 'n':
                advance = match_nil(c);
                if (advance)
                {
                    t.type = NIL;
                    break;
                }
            case 't':
                advance = match_true(c);
                if (advance)
                {
                    t.type = TRUE;
                    break;
                }
            case 'v':
                advance = match_var(c);
                if (advance)
                {
                    t.type = VAR;
                    break;
                }

            default:
                advance = match_identifier(c);
                if (advance)
                {
                    t.type = IDENTIFIER;
                    break;
                }

                advance = match_number(c);
                if (advance)
                {
                    t.type = NUMBER;
                    break;
                }

                // We didn't match anything, so consume until we hit whitespace
                advance = 1;
                while (!is_whitespace(*(c+advance)))
                {
                    advance = advance + 1;
                }
                t.type = INVALID;
        }

        // Advance our position
        position = start + advance;
    } while (!token_found);

    t.start = start;
    t.end = position;

    context->lookahead = t;

    return t;
}

/*
 * Accept the next token on the input stream, consuming it.
 */
Token accept(ScanContext *context)
{
    if (context->lookahead.start >= context->position && context->position > 0)
    {
        context->position = context->lookahead.end;
        return context->lookahead;
    }

    Token t = peek(context);
    context->position = t.end;
    return t;
}

bool match(ScanContext *context, int num, ...)
{
    bool found = false;
    Token next = peek(context);

    va_list args;
    va_start(args, num);
    for (int i = 0; i < num; i++)
    {
        if (next.type == va_arg(args, int))
        {
            found = true;
            break;
        }
    }
    va_end(args);

    return found;
}

char *token_value(ScanContext *context, Token t)
{
    int len = t.end - t.start;
    char *value = calloc(len + 1, sizeof(char));
    memcpy(value, context->buffer + t.start, len);

    return value;
}

/*
 * Scan a string, and return a list of corresponding tokens. This is primarily
 * for testing purposes.
 */
TokenList scan_input(char *input)
{
    // We just create an arbitrarily-sized token list to begin with
    TokenList tokens = token_list_create(2);
    Token t;
    ScanContext context = {input, 0};

    do {
        t = accept(&context);
        token_list_add(&tokens, t);
    } while (t.type != EOF_CHAR);

    return tokens;
}

