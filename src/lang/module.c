/*
 * Copyright (c) 2021, Dana Burkart <dana.burkart@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <string.h>

#include "module.h"

const char *symbol_name_for_module_path(char *module_path)
{
    const char *last_component = strrchr(module_path, '/');

    if (last_component == NULL)
        return module_path;

    last_component += 1;

    // TODO: Handle suffix, if there is one
    return last_component;
}
