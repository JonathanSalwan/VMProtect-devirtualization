/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * This tool is meant to be run on the "resetsegv.c" application.  It tests
 * that PIN_SafeCopy() works even if the application sets up a SEGV handler
 * with SA_RESETHAND.
 */

#include <iostream>
#include <cstdlib>
#include "pin.H"

static VOID OnImage(IMG, VOID*);
static VOID DoSafeCopy();
static void OnExit(INT32, VOID*);

int main(int argc, char* argv[])
{
    PIN_Init(argc, argv);
    PIN_InitSymbols();

    IMG_AddInstrumentFunction(OnImage, 0);
    PIN_AddFiniFunction(OnExit, 0);

    PIN_StartProgram();
    return 0;
}

static VOID OnImage(IMG img, VOID*)
{
#if defined(TARGET_MAC)
    RTN rtn = RTN_FindByName(img, "_MakeSegv");
#else
    RTN rtn = RTN_FindByName(img, "MakeSegv");
#endif
    if (RTN_Valid(rtn))
    {
        RTN_Open(rtn);
        RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(DoSafeCopy), IARG_END);
        RTN_Close(rtn);
    }
}

static VOID DoSafeCopy()
{
    // Before the bug was fixed in Pin, the first SafeCopy() would work, but would
    // reset the internal SEGV handler (because of SA_RESETHAND).  This caused the
    // second SafeCopy() to fail.  That's why we call SafeCopy() twice.
    //
    int word;
    size_t sz = PIN_SafeCopy(&word, 0, sizeof(word));
    if (sz != 0)
    {
        std::cerr << "SafeCopy should fail (1)\n";
        std::exit(1);
    }
    sz = PIN_SafeCopy(&word, 0, sizeof(word));
    if (sz != 0)
    {
        std::cerr << "SafeCopy should fail (2)\n";
        std::exit(1);
    }
    std::exit(0);
}

static VOID OnExit(INT32, VOID*)
{
    std::cerr << "Did not attempt SafeCopy\n";
    std::exit(1);
}
