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

    code_block_write(block, OP_LOAD);
    code_block_write(block, 0);
    code_block_write(block, 1);
    code_block_write(block, OP_LOAD);
    code_block_write(block, 1);
    code_block_write(block, 2);
    code_block_write(block, OP_ADD);
    code_block_write(block, 1);
    code_block_write(block, 2);
    code_block_write(block, 3);

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
