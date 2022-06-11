/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <string>
#include <stdlib.h>

#include "pin.H"

#ifdef TARGET_WINDOWS
namespace WND
{
#include <windows.h>
}
#endif
#include "tool_macros.h"

typedef int (*foo_t)();

static AFUNPTR foo_ptr1;
static AFUNPTR foo_ptr2;

static int foo_rep1()
{
    printf("foo rep1 called\n");

    return ((foo_t)foo_ptr1)();
}

static int foo_rep2()
{
    printf("foo rep2 called\n");

    return ((foo_t)foo_ptr2)();
}

static VOID on_module_loading(IMG img, VOID* data)
{
    if (IMG_IsMainExecutable(img))
    {
        RTN routine = RTN_FindByName(img, C_MANGLE("foo"));
        if (!RTN_Valid(routine))
        {
            routine = RTN_FindByName(img, C_MANGLE("_foo"));
        }

        if (RTN_Valid(routine))
        {
            foo_ptr1 = RTN_ReplaceProbed(routine, (AFUNPTR)(foo_rep1));
            foo_ptr2 = RTN_ReplaceProbed(routine, (AFUNPTR)(foo_rep2));
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
