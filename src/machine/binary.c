/*
 * Copyright (c) 2021, Dana Burkart <dana.burkart@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "binary.h"
#include "memory.h"
#include "value.h"

typedef struct __attribute__((__packed__))
{
    uint8_t type;
    uint32_t v_size;
} packed_memory_value_t;

binary_t *binary_create(void)
{
    binary_t *binary = (binary_t *)calloc(1, sizeof(binary_t));

    binary->magic = 0xBABABEEF;
    binary->version = VERSION;

    return binary;
}

binary_t *binary_load(const char *path)
{
    binary_t *binary = (binary_t *)malloc(sizeof(binary_t));
    int fd = open(path, O_RDONLY);

    read(fd, &binary->magic, 4);
    read(fd, &binary->version, 2);

    assert(binary->magic == 0xBABABEEF);

    read(fd, &binary->reserved, 2);

    read(fd, &binary->sections, sizeof(binary->sections));

    // Load data section
    lseek(fd, binary->sections.data_offset, SEEK_SET);
    memory_t *mem = memory_create(1);
    size_t bytes_read = 0;
    int i = 0;
    while (bytes_read < binary->sections.code_offset - binary->sections.data_offset)
    {
        // First, read off our packed memory value descriptor and create a value_t
        value_t val;
        char *str;
        packed_memory_value_t packed;
        read(fd, &packed, sizeof(packed_memory_value_t));
        bytes_read += sizeof(packed_memory_value_t);
        val.type = packed.type;
        switch (val.type)
        {
            case VAL_INT:
                read(fd, &val.contents.number, packed.v_size);
                break;
            case VAL_FLOAT:
                read(fd, &val.contents.real, packed.v_size);
                break;
            case VAL_BOOLEAN:
                read(fd, &val.contents.boolean, packed.v_size);
                break;
            case VAL_STRING:
                str = (char *)malloc(packed.v_size);
                read(fd, str, packed.v_size);
                val = string_create(str);
                break;
            default:
                ;
        }
        bytes_read += packed.v_size;
        memory_set(mem, i++, val);
    }
    binary->data = mem;

    // Load code section
    lseek(fd, binary->sections.code_offset, SEEK_SET);
    code_block_t *code = code_block_create();
    size_t size;
    read(fd, &size, sizeof(size_t));
    for (int j = 0; j < size; j++)
    {
        instruction_t instruction;
        read(fd, &instruction, sizeof(instruction_t));
        code_block_write(code, instruction);
    }
    binary->code = code;

    close(fd);

    return binary;
}

void binary_write(binary_t *binary, const char *path)
{
    int fd = open(path, O_WRONLY|O_CREAT|O_TRUNC, 0666);

    // Measure our data section
    uint8_t *data_section;
    size_t total_size = 0;
    string_t *s1;
    packed_memory_value_t *packed_values = (packed_memory_value_t *)malloc(sizeof(packed_memory_value_t) * binary->data->capacity);
    for (int i = 0; i < binary->data->capacity; i++)
    {
        uint32_t size = 0;
        packed_values[i].type = binary->data->contents[i].type;
        switch(binary->data->contents[i].type)
        {
            case VAL_INT:
                size = sizeof(int);
                break;
            case VAL_FLOAT:
                size = sizeof(float);
                break;
            case VAL_BOOLEAN:
                size = sizeof(bool);
                break;
            case VAL_STRING:
                s1 = (string_t *)binary->data->contents[i].contents.object;
                size = strlen(s1->string) + 1;
                break;
            default:
                ;
        }

        total_size += sizeof(packed_memory_value_t) + size;
        packed_values[i].v_size = size;
    }

    binary->sections.data_offset = 24;
    binary->sections.code_offset = 24 + total_size;

    // Write out the header
    write(fd, binary, 24);

    // Now, serialize our data section
    data_section = (uint8_t *)malloc(total_size);
    for (int i = 0; i < binary->data->capacity; i++)
    {
        write(fd, &packed_values[i], sizeof(packed_memory_value_t));
        switch(binary->data->contents[i].type)
        {
            case VAL_INT:
                write(fd, &binary->data->contents[i].contents.number, packed_values[i].v_size);
                break;
            case VAL_FLOAT:
                write(fd, &binary->data->contents[i].contents.real, packed_values[i].v_size);
                break;
            case VAL_BOOLEAN:
                write(fd, &binary->data->contents[i].contents.boolean, packed_values[i].v_size);
                break;
            case VAL_STRING:
                s1 = (string_t *)binary->data->contents[i].contents.object;
                write(fd, s1->string, packed_values[i].v_size);
                break;
            case VAL_ABSENT:
                ;
            case VAL_TUPLE:
                // TODO: Handle
                ;
        }
    }

    // Write out code section
    write(fd, &binary->code->size, sizeof(size_t));
    write(fd, binary->code->code, binary->code->size * sizeof(instruction_t));

done:
    close(fd);
    free(data_section);
    free(packed_values);
}
