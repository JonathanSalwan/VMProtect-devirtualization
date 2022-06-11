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

static int foo_rep1()
{
    printf("foo rep1 called\n");

    return ((foo_t)foo_ptr1)();
}

static int foo_rep2(foo_t orig_foo, ADDRINT returnIp)
{
    printf("foo rep2 called\n");

    int res = orig_foo();

    // May not be executed if exception occurs in previous statement
    printf("Caller IP = %s\n", hexstr(returnIp).c_str());

    return res;
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
            foo_ptr1 = RTN_ReplaceProbed(routine, (AFUNPTR)foo_rep1);

            PROTO foo_proto  = PROTO_Allocate(PIN_PARG(int), CALLINGSTD_DEFAULT, "foo", PIN_PARG_END());
            AFUNPTR foo_ptr2 = RTN_ReplaceSignatureProbed(routine, (AFUNPTR)foo_rep2, IARG_PROTOTYPE, foo_proto,
                                                          IARG_ORIG_FUNCPTR, IARG_RETURN_IP, IARG_END);
            ASSERTX(foo_ptr2 != 0);
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
