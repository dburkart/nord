#include "memory.h"

memory_t *memory_create(size_t size)
{
    memory_t *mem = malloc(sizeof(memory_t));

    mem->capacity = size;
    mem->contents = calloc(mem->capacity, sizeof(value_t));

    return mem;
}

void memory_free(memory_t *mem)
{
    // TODO: Free values
    free(mem);
}

void memory_set(memory_t *mem, int address, value_t val)
{
    if (mem->capacity - 1 < address)
    {
        int old = mem->capacity;
        mem->capacity = (address > old * 2) ? address * 2 : old * 2;
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
