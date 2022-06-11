/*
 * Copyright (C) 2018-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/* @file
 * Mantis 4231
 * Check the APIs PIN_CheckWriteAccess  and PIN_CheckReadAccess produce the right result when
 * the argument of the API is the last byte of the page.
 *
 * App allocate 2 continuous pages with the next protections: first- read/write. second - none.
 *
 */
#include "pin.H"

/* ===================================================================== */

//ret is the last byte of the first page that the app allocated.

VOID MmapAfter(ADDRINT ret)
{
    ASSERTX(PIN_CheckReadAccess((void*)ret));
    ASSERTX(PIN_CheckWriteAccess((void*)ret));
    ASSERTX(!PIN_CheckReadAccess((void*)(ret + 1)));
    ASSERTX(!PIN_CheckWriteAccess((void*)(ret + 1)));
}

/* ===================================================================== */
/* Instrumentation routines                                              */
/* ===================================================================== */

VOID Image(IMG img, VOID* v)
{
    if (IMG_IsMainExecutable(img))
    {
        RTN rtn = RTN_FindByName(img, "_NotifyPinAfterMmap");
        if (!RTN_Valid(rtn))
        {
            rtn = RTN_FindByName(img, "NotifyPinAfterMmap");
            if (!RTN_Valid(rtn))
            {
                PIN_ExitApplication(3);
            }
        }
        RTN_Open(rtn);
        RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)MmapAfter, IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_END);
        RTN_Close(rtn);
    }
}
/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

int main(int argc, char* argv[])
{
    // Initialize pin & symbol manager
    if (PIN_Init(argc, argv))
    {
        return 2;
    }
    PIN_InitSymbols();

    // Register Image to be called to instrument functions.
    IMG_AddInstrumentFunction(Image, 0);
    //RTN_AddInstrumentFunction(InstrumentRoutine, 0);
    // Never returns
    PIN_StartProgram();
    return 1;
}
