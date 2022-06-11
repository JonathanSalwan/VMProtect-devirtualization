/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
  This tool validates proper propagation of exception from function being instrumented
  with RTN_InsertCallProbed(routine, IPOINT_AFTER, ...)
  and proper delivery of IARG_RETURN_IP value to IPOINT_AFTER analysis routine.
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

static VOID foo_before(int a0, int a1, int a2, int a3, int a4, int a5, int a6)
{
    printf("result=%d\n", a0 + a1 + a2 + a3 + a4 + a5 + a6);
    fflush(stdout);
}

static VOID foo_after(ADDRINT returnIp)
{
    printf("foo after called\n");
    fflush(stdout);

    // May not be executed if exception occurs in previous statement
    printf("Caller IP = %s\n", hexstr(returnIp).c_str());
    fflush(stdout);
}

#if defined(TARGET_LINUX) || defined(TARGET_MAC)
#define CALL_TYPE CALLINGSTD_DEFAULT
#else
#if defined(TARGET_IA32E)
#define CALL_TYPE CALLINGSTD_DEFAULT
#else
#define CALL_TYPE CALLINGSTD_REGPARMS
#endif
#endif

static VOID on_module_loading(IMG img, VOID* data)
{
    if (IMG_IsMainExecutable(img))
    {
        RTN routine = RTN_FindByName(img, C_MANGLE("foo"));
        if (!RTN_Valid(routine))
        {
            routine = RTN_FindByName(img, C_MANGLE("@foo@28"));
        }

        if (RTN_Valid(routine))
        {
            PROTO foo_proto = PROTO_Allocate(PIN_PARG(int), CALL_TYPE, "foo", PIN_PARG(int), PIN_PARG(int), PIN_PARG(int),
                                             PIN_PARG(int), PIN_PARG(int), PIN_PARG(int), PIN_PARG(int), PIN_PARG_END());

            RTN_InsertCallProbed(
                routine, IPOINT_BEFORE, AFUNPTR(foo_before), IARG_PROTOTYPE, foo_proto, IARG_FUNCARG_ENTRYPOINT_VALUE, 6,
                IARG_FUNCARG_ENTRYPOINT_VALUE, 5, IARG_FUNCARG_ENTRYPOINT_VALUE, 4, IARG_FUNCARG_ENTRYPOINT_VALUE, 3,
                IARG_FUNCARG_ENTRYPOINT_VALUE, 2, IARG_FUNCARG_ENTRYPOINT_VALUE, 1, IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_END);

            RTN_InsertCallProbed(routine, IPOINT_AFTER, AFUNPTR(foo_after), IARG_PROTOTYPE, foo_proto, IARG_RETURN_IP, IARG_END);
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
