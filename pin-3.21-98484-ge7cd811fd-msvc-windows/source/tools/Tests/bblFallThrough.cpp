/*
 * Copyright (C) 2010-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 *  Check consistency of fall through in BBLs and INSes.
 */

#include "pin_tests_util.H"

VOID Trace(TRACE trace, VOID* v)
{
    for (BBL bbl = TRACE_BblTail(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
    {
        if (BBL_HasFallThrough(bbl))
        {
            INS ins = BBL_InsTail(bbl);
            TEST(INS_HasFallThrough(ins), "BBL_HasFallThrough or INS_HasFallThrough failed");
        }
    }
}

int main(INT32 argc, CHAR** argv)
{
    PIN_Init(argc, argv);

    TRACE_AddInstrumentFunction(Trace, 0);

    // Never returns
    PIN_StartProgram();

    return 0;
}
