/*
 * Copyright (c) 2021, Dana Burkart <dana.burkart@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "disassemble.h"

#define FORMAT_PAIR         "%-10s $%d $%d\n"
#define FORMAT_PAIR_ADDR    "%-10s $%d @%d\n"
#define FORMAT_PAIR_CONST   "%-10s $%d %d\n"
#define FORMAT_TRIPLET      "%-10s $%d $%d $%d\n"

char *disassemble_instruction(instruction_t);

char *disassemble(code_block_t *block)
{
    char *assembly;
    size_t size = 32;
    size_t len = 0;

    assembly = calloc(size, sizeof(char));

    for (int i = 0; i < block->size; i++)
    {
        char *new_instruction = disassemble_instruction(block->code[i]);

        if (new_instruction == NULL)
            continue;

        size_t instruction_len = strlen(new_instruction);

        if (instruction_len >= size - len)
        {
            size = size * 2 - len > size - len ? size * 2 : size - len + 1;
            assembly = realloc(assembly, size);
        }

        strcpy(assembly + len, new_instruction);
        len += instruction_len;
        free(new_instruction);
    }

    return assembly;
}

char *disassemble_instruction(instruction_t instruction)
{
    char *assembly;

    switch (instruction.opcode)
    {
        // load <register> <address>
        case OP_LOAD:
            asprintf(&assembly, FORMAT_PAIR_ADDR,
                     "load",
                     instruction.fields.pair.arg1,
                     instruction.fields.pair.arg2
                    );
            break;

        // load <register> <value>
        case OP_LOADV:
            asprintf(&assembly, FORMAT_PAIR_CONST,
                     "loadv",
                     instruction.fields.triplet.arg1,
                     instruction.fields.triplet.arg3
                    );
            break;

        // store <register> <address>
        case OP_STORE:
            asprintf(&assembly, FORMAT_PAIR_ADDR,
                     "store",
                     instruction.fields.triplet.arg1,
                     instruction.fields.triplet.arg3
                    );
            break;

        // move <register-out> <register-in>
        case OP_MOVE:
            asprintf(&assembly, FORMAT_PAIR,
                     "move",
                     instruction.fields.triplet.arg1,
                     instruction.fields.triplet.arg3
                    );
            break;

        // jump <instruction #>
        case OP_JMP:

            break;

        // add <register-out> <register-in> <register-in>
        case OP_ADD:
            asprintf(&assembly, FORMAT_TRIPLET,
                     "add",
                     instruction.fields.triplet.arg1,
                     instruction.fields.triplet.arg2,
                     instruction.fields.triplet.arg3
                    );
            break;

        // subtract <register-out> <register-in> <register-in>
        case OP_SUBTRACT:
            asprintf(&assembly, FORMAT_TRIPLET,
                     "subtract",
                     instruction.fields.triplet.arg1,
                     instruction.fields.triplet.arg2,
                     instruction.fields.triplet.arg3
                    );
            break;

        // multiply <register-out> <register-in> <register-in>
        case OP_MULTIPLY:
            asprintf(&assembly, FORMAT_TRIPLET,
                     "multiply",
                     instruction.fields.triplet.arg1,
                     instruction.fields.triplet.arg2,
                     instruction.fields.triplet.arg3
                    );
            break;

        // divide <register-out> <register-in> <register-in>
        case OP_DIVIDE:
            asprintf(&assembly, FORMAT_TRIPLET,
                     "divide",
                     instruction.fields.triplet.arg1,
                     instruction.fields.triplet.arg2,
                     instruction.fields.triplet.arg3
                    );
            break;
        // negate <register-out> <register-in>
        case OP_NEGATE:
            asprintf(&assembly, FORMAT_PAIR,
                     "negate",
                     instruction.fields.triplet.arg1,
                     instruction.fields.triplet.arg2
                    );
            break;

        // -- Functions
        case OP_RETURN:

            break;
    }

    if (assembly == NULL)
        return NULL;

    return assembly;
}