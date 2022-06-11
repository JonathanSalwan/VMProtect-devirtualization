/*
 * Copyright (C) 2004-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 *  This file tests pin tool failure behavior
 */

#include <stdio.h>
#include "pin.H"

/* ===================================================================== */

VOID Trace(TRACE trace, void* v)
{
    char* p  = 0;
    char foo = *p; // SEGV
    printf("%d\n", foo);
}

/* ===================================================================== */

int main(int argc, char* argv[])
{
    PIN_Init(argc, argv);

    TRACE_AddInstrumentFunction(Trace, 0);

    // Never returns
    PIN_StartProgram();

    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
