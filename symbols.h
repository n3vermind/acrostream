#pragma once

#include <stdio.h>
#include <dlfcn.h>

#include "log.h"

inline static void *symbol_load(void *handle, const char *symbol) {
    void *ptr = dlsym(handle, symbol);
    if (!ptr) {
        error("Failed to acquire symbol %s!\n", symbol);
    }
    return ptr;
}

