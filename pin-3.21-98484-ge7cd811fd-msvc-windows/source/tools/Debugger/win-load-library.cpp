/*
 * Copyright (C) 2011-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <windows.h>
#include <iostream>

typedef void (*FOOPTR)();

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        std::cerr << "Must specify pathname of library to load.\n";
        return 1;
    }

    HMODULE mod = LoadLibraryA(argv[1]);
    if (!mod)
    {
        std::cerr << "Unable to load library '" << argv[1] << "\n";
        return 1;
    }

    FOOPTR foo = reinterpret_cast< FOOPTR >(GetProcAddress(mod, "Foo"));
    if (!foo)
    {
        std::cerr << "Unable to find function Foo\n";
        return 1;
    }
    foo();

    FreeLibrary(mod);
    return 0;
}
