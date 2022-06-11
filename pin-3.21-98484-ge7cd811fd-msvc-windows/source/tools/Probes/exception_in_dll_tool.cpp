/*
 * Copyright (C) 2011-2021 Intel Corporation.
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

typedef void (*foo_t)();

static void foo1_rep(foo_t orig_foo1)
{
    printf("foo1 rep called\n");

    orig_foo1();
}

static void ExcInDll_rep(foo_t orig_func)
{
    printf("ExcInDll rep called\n");

    orig_func();
}

static VOID foo2_before() { printf("foo2 - before called\n"); }

static VOID foo2_after() { printf("foo2 after called\n"); }

static VOID on_module_loading(IMG img, VOID* data)
{
    if (IMG_IsMainExecutable(img))
    {
        RTN routine = RTN_FindByName(img, C_MANGLE("foo1"));

        if (RTN_Valid(routine) && RTN_IsSafeForProbedReplacement(routine))
        {
            PROTO foo1_proto = PROTO_Allocate(PIN_PARG(void), CALLINGSTD_DEFAULT, "foo1", PIN_PARG_END());
            AFUNPTR foo1_ptr =
                RTN_ReplaceSignatureProbed(routine, (AFUNPTR)foo1_rep, IARG_PROTOTYPE, foo1_proto, IARG_ORIG_FUNCPTR, IARG_END);
            ASSERTX(foo1_ptr != 0);
        }

        routine = RTN_FindByName(img, C_MANGLE("foo2"));
        if (RTN_Valid(routine) && RTN_IsSafeForProbedInsertion(routine))
        {
            PROTO foo2_proto = PROTO_Allocate(PIN_PARG(void), CALLINGSTD_DEFAULT, "foo2", PIN_PARG_END());

            RTN_InsertCallProbed(routine, IPOINT_BEFORE, AFUNPTR(foo2_before), IARG_PROTOTYPE, foo2_proto, IARG_END);

            RTN_InsertCallProbed(routine, IPOINT_AFTER, AFUNPTR(foo2_after), IARG_PROTOTYPE, foo2_proto, IARG_END);
        }
    }

    else
    {
        RTN routine = RTN_FindByName(img, C_MANGLE("ExcInDll"));

        if (RTN_Valid(routine) && RTN_IsSafeForProbedReplacement(routine))
        {
            PROTO ExcInDll_proto = PROTO_Allocate(PIN_PARG(void), CALLINGSTD_DEFAULT, "ExcInDll", PIN_PARG_END());
            AFUNPTR ExcInDll_ptr = RTN_ReplaceSignatureProbed(routine, (AFUNPTR)ExcInDll_rep, IARG_PROTOTYPE, ExcInDll_proto,
                                                              IARG_ORIG_FUNCPTR, IARG_END);
            ASSERTX(ExcInDll_ptr != 0);
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
