/*
 * Copyright (C) 2011-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include "pin.H"
#include <stdio.h>

/*
 * Instrumentation routines
 */
int numRets = 0;
BOOL isSafeForProbeReplacement;
VOID ImageLoad(IMG img, VOID* v)

{
    for (SEC sec = IMG_SecHead(img); SEC_Valid(sec); sec = SEC_Next(sec))
    {
        for (RTN rtn = SEC_RtnHead(sec); RTN_Valid(rtn); rtn = RTN_Next(rtn))
        {
            // Open the RTN.
            RTN_Open(rtn);
            // Examine each instruction in the routine.
            for (INS ins = RTN_InsHead(rtn); INS_Valid(ins); ins = INS_Next(ins))
            {
                if (INS_IsRet(ins))
                {
                    numRets++;
                }
            }
            // Close the RTN.
            RTN_Close(rtn);

            isSafeForProbeReplacement = RTN_IsSafeForProbedReplacement(rtn);
        }
    }
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

int main(int argc, char* argv[])
{
    // Initialize pin & symbol manager
    PIN_Init(argc, argv);
    PIN_InitSymbols();

    // Register ImageLoad
    IMG_AddInstrumentFunction(ImageLoad, 0);

    // Start the program, never returns
    PIN_StartProgramProbed();

    return 0;
}
/* ===================================================================== */
