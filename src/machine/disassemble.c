/*
 * Copyright (c) 2021, Dana Burkart <dana.burkart@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "disassemble.h"
#include "memory.h"

#define FORMAT_SINGLE           "%-10s $%d\n"
#define FORMAT_SINGLE_CONST     "%-10s %d\n"
#define FORMAT_SINGLE_ADDR      "%-10s @%d\n"
#define FORMAT_PAIR             "%-10s $%d $%d\n"
#define FORMAT_PAIR_ADDR        "%-10s $%d @%d\n"
#define FORMAT_PAIR_ADDR2       "%-10s @%d $%d\n"
#define FORMAT_PAIR_CONST_NUM   "%-10s $%d %d\n"
#define FORMAT_PAIR_CONST_STR   "%-10s $%d %s\n"
#define FORMAT_PAIR_CONST_REG   "%-10s %d $%d\n"
#define FORMAT_TRIPLET          "%-10s $%d $%d $%d\n"
#define FORMAT_TRIPLET_CMP      "%-10s %d $%d $%d\n"
#define FORMAT_TRIPLET_VAL      "%-10s $%d $%d %d\n"

char *disassemble_instruction(memory_t *mem, instruction_t);

char *disassemble(binary_t *binary)
{
    char *assembly;
    size_t size = 32;
    size_t len = 0;

    assembly = calloc(size, sizeof(char));

    for (int i = 0; i < binary->code->size; i++)
    {
        char *region_marker;

        asprintf(&region_marker, "\nCode Region: %d\n\n", i);
        size_t region_marker_len = strlen(region_marker);
        if (region_marker_len >= size - len)
        {
            size = size * 2 - len > size - len ? size * 2 : size - len + 1;
            assembly = realloc(assembly, size);
        }

        strcpy(assembly + len, region_marker);
        len += region_marker_len;
        free(region_marker);

        for (int j = 0; j < binary->code->blocks[i]->size; j++)
        {
            char *new_instruction = disassemble_instruction(binary->data, binary->code->blocks[i]->code[j]);

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
    }

    return assembly;
}

char *disassemble_instruction(memory_t *mem, instruction_t instruction)
{
    char *assembly;
    char *str;
    value_t value;
    string_t *s1;

    switch (instruction.opcode)
    {
        case OP_NIL:
            asprintf(&assembly, FORMAT_SINGLE,
                     "nil",
                     instruction.fields.pair.arg1
                    );
            break;

        case OP_DEREF:
            asprintf(&assembly, FORMAT_TRIPLET_VAL,
                     "deref",
                     instruction.fields.triplet.arg1,
                     instruction.fields.triplet.arg2,
                     instruction.fields.triplet.arg3
                    );
            break;

        // load <register> <address>
        case OP_LOAD:
            value = memory_get(mem, instruction.fields.pair.arg2);

            if (value.type == VAL_ABSENT)
            {
                asprintf(&assembly, FORMAT_PAIR_ADDR,
                        "load",
                        instruction.fields.pair.arg1,
                        instruction.fields.pair.arg2
                    );
            }
            // When disassembling a load instruction, we instead output a
            // pseudo-instruction, "set", which sets values directly to
            // registers. We do this only for "constant" values though.
            else
            {
                if (value.type == VAL_INT)
                    asprintf(&str, "%d", value.contents.number);
                else if (value.type == VAL_STRING)
                {
                    s1 = (string_t *)value.contents.object;
                    asprintf(&str, "\"%s\"", s1->string);
                }
                else if (value.type == VAL_FLOAT)
                    asprintf(&str, "%f", value.contents.real);
                else if (value.type == VAL_BOOLEAN)
                    asprintf(&str, "%s", (value.contents.boolean == true) ? "true" : "false");
                else if (value.type == VAL_FUNCTION)
                    asprintf(&str, "@%d ; Function", instruction.fields.pair.arg2);
                asprintf(&assembly, FORMAT_PAIR_CONST_STR,
                        "set",
                        instruction.fields.pair.arg1,
                        str
                        );
                free(str);
            }

            break;

        // load <register> <value>
        case OP_LOADV:
            asprintf(&assembly, FORMAT_PAIR_CONST_NUM,
                     "loadv",
                     instruction.fields.pair.arg1,
                     instruction.fields.pair_signed.arg2
                    );
            break;

        // store <register> <address>
        case OP_STORE:
            asprintf(&assembly, FORMAT_PAIR_ADDR2,
                     "store",
                     instruction.fields.pair.arg1,
                     instruction.fields.pair.arg2
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

        // push <register>
        case OP_PUSH:
            asprintf(&assembly, FORMAT_SINGLE,
                     "push",
                     instruction.fields.pair.arg2
                    );
            break;

        // pop <register>
        case OP_POP:
            asprintf(&assembly, FORMAT_SINGLE,
                     "pop",
                     instruction.fields.pair.arg2
                    );
            break;

        case OP_RESTORE:
            asprintf(&assembly, FORMAT_SINGLE_CONST,
                     "restore",
                     instruction.fields.pair.arg2
                    );
            break;

        // jump <instruction #>
        case OP_JMP:
            asprintf(&assembly, FORMAT_SINGLE,
                     "jump",
                     instruction.fields.pair.arg1
                    );
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
        case OP_EQUAL:
            asprintf(&assembly, FORMAT_TRIPLET_CMP,
                     "eq",
                     instruction.fields.triplet.arg1,
                     instruction.fields.triplet.arg2,
                     instruction.fields.triplet.arg3
                    );
            break;
        case OP_LESSTHAN:
            asprintf(&assembly, FORMAT_TRIPLET_CMP,
                     "lt",
                     instruction.fields.triplet.arg1,
                     instruction.fields.triplet.arg2,
                     instruction.fields.triplet.arg3
                    );
            break;

        case OP_AND:
            asprintf(&assembly, FORMAT_TRIPLET,
                     "and",
                     instruction.fields.triplet.arg1,
                     instruction.fields.triplet.arg2,
                     instruction.fields.triplet.arg3
                    );
            break;

        case OP_OR:
            asprintf(&assembly, FORMAT_TRIPLET,
                     "or",
                     instruction.fields.triplet.arg1,
                     instruction.fields.triplet.arg2,
                     instruction.fields.triplet.arg3
                    );
            break;

        // negate <register-out> <register-in>
        case OP_NEGATE:
            asprintf(&assembly, FORMAT_PAIR,
                     "negate",
                     instruction.fields.pair.arg1,
                     instruction.fields.pair.arg2
                    );
            break;

        case OP_NOT:
            asprintf(&assembly, FORMAT_PAIR,
                     "not",
                     instruction.fields.pair.arg1,
                     instruction.fields.pair.arg2
                    );
            break;

        // -- Functions
        case OP_CALL:
            asprintf(&assembly, FORMAT_SINGLE_ADDR,
                     "call",
                     instruction.fields.pair.arg2
                    );
            break;

        case OP_CALL_DYNAMIC:
            asprintf(&assembly, FORMAT_SINGLE_ADDR,
                     "calld",
                     instruction.fields.pair.arg2
                    );
            break;

        case OP_RETURN:
            asprintf(&assembly, FORMAT_SINGLE,
                     "return",
                     instruction.fields.pair.arg2
                    );
            break;

        case OP_IMPORT:
            asprintf(&assembly, FORMAT_SINGLE_ADDR,
                     "import",
                     instruction.fields.pair.arg1
                     );
            break;
    }

    if (assembly == NULL)
        return NULL;

    return assembly;
}
