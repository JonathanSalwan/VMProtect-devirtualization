/*
 * Copyright (C) 2017-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include "pin.H"

VOID Trace(TRACE trace, VOID* v) { PIN_ExitApplication(123); }

int main(int argc, char* argv[])
{
    PIN_Init(argc, argv);
    TRACE_AddInstrumentFunction(Trace, 0);
    PIN_StartProgram();
    return 1;
}
