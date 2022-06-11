/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

//
// This tool replaces my_yield, and calls the original.
// The function my_yield is written such that there is a syscall within
// the probe
//

#include "pin.H"
#include <iostream>
#include <stdlib.h>
using std::cerr;
using std::endl;

/* ===================================================================== */
/* Global Variables */
/* ===================================================================== */

static int (*pf_yield)();
static void (*pf_marker)();
static int my_counter = 0;

/* ===================================================================== */
/* Replacement Functions */
/* ===================================================================== */

int YieldProbe()
{
    my_counter++;
    return (pf_yield)();
}

void MarkerProbe()
{
    if (my_counter != 100)
    {
        cerr << "Unexpected number of calls to yield" << endl;
        exit(1);
    }

    int res = (pf_yield)();
    if (res != 0)
    {
        cerr << "Unexpected number of calls to yield" << endl;
        exit(1);
    }
    cerr << "Test ended succesfully" << endl;
    exit(0);
}

/* ===================================================================== */

// Called every time a new image is loaded
// Look for routines that we want to probe

VOID ImageLoad(IMG img, VOID* v)
{
    RTN yieldRtn = RTN_FindByName(img, "my_yield");
    if (RTN_Valid(yieldRtn))
    {
        if (RTN_IsSafeForProbedReplacement(yieldRtn))
        {
            pf_yield = (int (*)())RTN_ReplaceProbed(yieldRtn, AFUNPTR(YieldProbe));

            cerr << "Inserted probe for my_yield:" << IMG_Name(img) << endl;
        }
    }

    RTN markerRtn = RTN_FindByName(img, "my_marker");
    if (RTN_Valid(markerRtn))
    {
        if (RTN_IsSafeForProbedReplacement(markerRtn))
        {
            pf_marker = (void (*)())RTN_ReplaceProbed(markerRtn, AFUNPTR(MarkerProbe));

            cerr << "Inserted probe for my_marker:" << IMG_Name(img) << endl;
        }
    }
}

/* ===================================================================== */

int main(int argc, CHAR* argv[])
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
