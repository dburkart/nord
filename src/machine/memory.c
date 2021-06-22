#include "memory.h"

memory_t *memory_create(size_t size)
{
    memory_t *mem = calloc(size, sizeof(memory_t));

    return mem;
}

void memory_free(memory_t *mem)
{
    // TODO: Free values
    free(mem);
}

void memory_set(memory_t *mem, int address, value_t val)
{
    // We've not stored anything in this memory block yet, so set us up
    // for a capacity of address + 1
    if (mem->capacity == 0)
    {
        mem->capacity = address + 1;
        mem->contents = calloc(mem->capacity, sizeof(value_t));
    }

    if (mem->capacity <= address)
    {
        int old = mem->capacity;
        int difference = address - mem->capacity;
        mem->capacity = mem->capacity * 2;
        mem->contents = realloc(mem->contents, mem->capacity * sizeof(value_t));

        // Ensure that we zero out our new memory
        for (int i = old; i < mem->capacity; i++)
        {
            mem->contents[i].type = VAL_ABSENT;
        }
    }

    mem->contents[address] = val;
}

value_t memory_get(memory_t *mem, int address)
{
    value_t none = {VAL_ABSENT};
    if (address > mem->capacity)
        return none;

    // TODO: Error checking
    return mem->contents[address];
}
