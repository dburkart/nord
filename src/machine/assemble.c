/*
 * Copyright (c) 2021, Dana Burkart <dana.burkart@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <assert.h>
#include <stdbool.h>

#include "assemble.h"
#include "bytecode.h"
#include "util/match.h"

enum asm_token_type_e
{
    ASM_TOK_INSTRUCTION,
    ASM_TOK_REGISTER,
    ASM_TOK_ADDRESS,
    ASM_TOK_NUMBER,
    ASM_TOK_EOF,
};

typedef struct
{
    enum asm_token_type_e type;
    opcode_t opcode;
    unsigned long start;
    unsigned long end;
} asm_token_t;

typedef struct
{
    const char *buffer;
    uint64_t position;
    asm_token_t lookahead;
    uint64_t lookahead_position;
} asm_scan_context_t;

int asm_value(asm_scan_context_t *context, asm_token_t token)
{
    return atoi(context->buffer + token.start + 1);
}

int asm_match_register_or_address(const char *c)
{
    int len = 0;

    if (*c != '$' && *c != '@')
        return len;

    len += 1;
    c += 1;

    while (!is_boundary(*c))
    {
        if (*c < '0' || *c > '9')
        {
            return 0;
        }

        len = len + 1;
        c = c + 1;
    }

    return len;
}

int asm_match_number(const char *c)
{
    int len = 0;

    while (!is_boundary(*c))
    {
        if (*c < '0' || *c > '9')
        {
            return 0;
        }

        len = len + 1;
        c = c + 1;
    }

    return len;
}

asm_token_t asm_peek(asm_scan_context_t *context)
{
    uint64_t position = context->position;
    uint64_t start;
    int advance;
    bool instruction_found = true;
    asm_token_t token;

    if (context->lookahead_position >= position && position > 0)
        return context->lookahead;

    do
    {
        const char *c = context->buffer + position;

        start = position;
        switch (*c)
        {
            case '\0':
                token.type = ASM_TOK_EOF;
                advance = 1;
                instruction_found = true;
                break;
            case ' ':
            case '\t':
            case '\n':
                advance = 1;
                instruction_found = false;

                break;
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                advance = asm_match_number(c);
                if (advance)
                {
                    token.type = ASM_TOK_NUMBER;
                }

                break;

            case '$':
                advance = asm_match_register_or_address(c);
                if (advance)
                {
                    token.type = ASM_TOK_REGISTER;
                }

                break;

            case '@':
                advance = asm_match_register_or_address(c);
                if (advance)
                {
                    token.type = ASM_TOK_ADDRESS;
                }

                break;
            case 'l':
                advance = match_keyword("loadv", c, 5);
                if (advance)
                {
                    token.type = ASM_TOK_INSTRUCTION;
                    token.opcode = OP_LOADV;
                    break;
                }

                advance = match_keyword("load", c, 4);
                if (advance)
                {
                    token.type = ASM_TOK_INSTRUCTION;
                    token.opcode = OP_LOAD;
                    break;
                }

                break;
        }

        position = start + advance;
    } while (!instruction_found);

    token.start = start;
    token.end = position;

    context->position = position;
    context->lookahead = token;
    context->lookahead_position = token.start;

    return token;
}

asm_token_t asm_accept(asm_scan_context_t *context)
{
    if (context->lookahead_position >= context->position && context->position > 0)
    {
        context->position = context->lookahead.end;
        return context->lookahead;
    }

    asm_token_t t = asm_peek(context);
    context->position = t.end;
    return t;
}

instruction_t assemble_instruction(asm_scan_context_t *context)
{
    instruction_t instruction;
    asm_token_t op, arg1, arg2, arg3;

    op = asm_accept(context);

    if (op.type == ASM_TOK_EOF)
    {
        instruction.opcode = OP_NONE;
        return instruction;
    }

    // TODO: Error handling
    assert(op.type == ASM_TOK_INSTRUCTION);

    instruction.opcode = op.opcode;

    switch (instruction.opcode)
    {
        case OP_LOAD:
            arg1 = asm_accept(context);
            arg2 = asm_accept(context);

            // TODO: Error handling
            assert(arg1.type == ASM_TOK_REGISTER);
            assert(arg2.type == ASM_TOK_ADDRESS);

            instruction.fields.pair.arg1 = asm_value(context, arg1);
            instruction.fields.pair.arg2 = asm_value(context, arg2);

            break;

        case OP_LOADV:
            arg1 = asm_accept(context);
            arg2 = asm_accept(context);

            // TODO: Error handling
            assert(arg1.type == ASM_TOK_REGISTER);
            assert(arg2.type == ASM_TOK_NUMBER);

            instruction.fields.pair.arg1 = asm_value(context, arg1);
            instruction.fields.pair.arg2 = atoi(context->buffer + arg1.start);

            break;

        default:
            ;
    }
}

code_block_t *assemble(const char *input)
{
    asm_scan_context_t context = { .buffer = input, .position = 0, .lookahead_position = 0 };

    code_block_t *block = code_block_create();

    while (asm_peek(&context).type != ASM_TOK_EOF)
    {
        instruction_t instruction = assemble_instruction(&context);
        code_block_write(block, instruction);
    }

    return block;
}
