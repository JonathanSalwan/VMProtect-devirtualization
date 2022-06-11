/*
 * Copyright (C) 2006-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/* ===================================================================== */
/*! @file
  Replace an original function with a custom function defined in the tool. The
  new function can have either the same or different signature from that of its
  original function.
*/

/* ===================================================================== */
#include "pin.H"
#include <stdlib.h>
#include <iostream>
#include "tool_macros.h"
using std::cout;
using std::endl;

/* ===================================================================== */
static void (*pf_bar)(int);

/* ===================================================================== */
VOID Boo()
{
    // This replacement routine does nothing
}

/* ===================================================================== */
VOID ImageLoad(IMG img, VOID* v)
{
    cout << IMG_Name(img) << endl;

    RTN rtn = RTN_FindByName(img, C_MANGLE("Bar"));
    if (RTN_Valid(rtn))
    {
        if (!RTN_IsSafeForProbedReplacement(rtn))
        {
            cout << "Cannot replace " << RTN_Name(rtn) << " in " << IMG_Name(img) << endl;
            exit(1);
        }

        cout << "Replacing " << RTN_Name(rtn) << " in " << IMG_Name(img) << endl;

        pf_bar = (void (*)(int))RTN_ReplaceProbed(rtn, AFUNPTR(Boo));
    }
}

/* ===================================================================== */
int main(INT32 argc, CHAR* argv[])
{
    PIN_InitSymbols();

    PIN_Init(argc, argv);

    IMG_AddInstrumentFunction(ImageLoad, 0);

    PIN_StartProgramProbed();

    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
