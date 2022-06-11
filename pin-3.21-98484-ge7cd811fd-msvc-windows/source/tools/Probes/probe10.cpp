/*
 * Copyright (C) 2007-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 */

#include "pin.H"
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include "tool_macros.h"
using std::cerr;
using std::cout;
using std::endl;

/* ===================================================================== */
/* Global Variables */
/* ===================================================================== */

static void (*pf_dn)();

/* ===================================================================== */

INT32 Usage()
{
    cerr << "This pin tool tests probe replacement.\n"
            "\n";
    cerr << KNOB_BASE::StringKnobSummary();
    cerr << endl;
    return -1;
}

void Foo_Function()
{
    if (pf_dn)
    {
        (*pf_dn)();
    }
    cout << "Inside replacement." << endl;
}

/* ===================================================================== */
// Called every time a new image is loaded
// Look for routines that we want to probe
VOID ImageLoad(IMG img, VOID* v)
{
    RTN rtn = RTN_FindByName(img, C_MANGLE("good_jump"));

    if (RTN_Valid(rtn))
    {
        if (RTN_IsSafeForProbedReplacement(rtn))
            cout << "good_jump is safe for probe replacement." << endl;
        else
            cout << "good_jump cannot be replaced." << endl;

        pf_dn = (void (*)())RTN_ReplaceProbed(rtn, AFUNPTR(Foo_Function));
    }
}

/* ===================================================================== */

int main(int argc, CHAR* argv[])
{
    PIN_InitSymbols();

    if (PIN_Init(argc, argv))
    {
        return Usage();
    }

    IMG_AddInstrumentFunction(ImageLoad, 0);

    PIN_StartProgramProbed();

    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
