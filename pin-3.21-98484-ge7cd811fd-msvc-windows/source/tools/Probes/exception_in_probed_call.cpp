/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <string>

#include "pin.H"

namespace WND
{
#include <windows.h>
}

typedef int (*foo_t)();

static AFUNPTR foo_ptr;

static int foo_rep()
{
    printf("foo called\n");

    return ((foo_t)foo_ptr)();
}

static VOID on_module_loading(IMG img, VOID* data)
{
    if (IMG_IsMainExecutable(img))
    {
        RTN routine = RTN_FindByName(img, "foo");
        if (!RTN_Valid(routine))
        {
            routine = RTN_FindByName(img, "_foo");
        }

        if (RTN_Valid(routine))
        {
            foo_ptr = RTN_ReplaceProbed(routine, (AFUNPTR)(foo_rep));
        }
    }
}

int main(int argc, char** argv)
{
    PIN_InitSymbols();

    if (!PIN_Init(argc, argv))
    {
        IMG_AddInstrumentFunction(on_module_loading, 0);

        PIN_StartProgramProbed();
    }

    exit(1);
}
