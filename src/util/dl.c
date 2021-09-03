/*
 * Copyright (c) 2021, Dana Burkart <dana.burkart@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>

#include "dl.h"

void *dynamic_load_self(const char *symbol)
{
    void *sym;

    // Clear dlerror() before potentially calling dlopen()
    dlerror();

#if PLATFORM == LINUX
    void *handle = dlopen(NULL, RTLD_LAZY | RTLD_GLOBAL);

    if (!handle)
    {
        fprintf(stderr, "dlopen failed: %s\n", dlerror());
        exit(1);
    }

    dlerror();
    sym = dlsym(handle, symbol);


    if (sym == NULL)
    {
        fprintf(stderr, "dlsym %s failed: %s\n", symbol, dlerror());
        exit(1);
    }

    dlclose(handle);
#else
    sym = dlsym(RTLD_SELF, symbol);

    if (sym == NULL)
    {
        fprintf(stderr, "dlsym %s failed: %s\n", symbol, dlerror());
        exit(1);
    }
#endif

    return sym;
}