/*
 * Copyright (c) 2021, Dana Burkart <dana.burkart@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <stdio.h>

#include "bytecode.h"
#include "vm.h"

int main(int argc, char *argv[])
{
    int status = 0;

    VM *vm;
    CodeBlock *block = code_block_create();
    Instruction i;

    i = (Instruction){OP_LOAD, 0, 1};
    code_block_write(block, i);

    i = (Instruction){OP_LOAD, 1, 2};
    code_block_write(block, i);

    i.opcode = OP_ADD;
    i.fields.triplet.arg1 = 1;
    i.fields.triplet.arg2 = 2;
    i.fields.triplet.arg3 = 3;
    code_block_write(block, i);

    code_block_print(block);

    vm = vm_create(block);

    Value a = {0, 15};
    Value b = {0, 25};
    memory_set(vm->memory, 0, a);
    memory_set(vm->memory, 1, b);

    vm_execute(vm);

    vm_dump(vm);

    code_block_free(block);

done:
    fflush(stdout);
    fflush(stderr);
    return status;
}
