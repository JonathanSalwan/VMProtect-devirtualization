/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/* ===================================================================== */
/*! @file
  Checks probe mode with functions relocation
 */

#include <stdlib.h>
#include "pin.H"
#include <iostream>
#include <fstream>
#include "tool_macros.h"
using std::cerr;
using std::cout;
using std::endl;

/* ===================================================================== */
/* Global Variables */
/* ===================================================================== */

typedef void (*FUNCPTR)();
typedef int (*FUNCPTR1a)();
static void (*pf_r1)();
static int (*pf_r1a)();
static void (*pf_r2)();

/* ===================================================================== */

INT32 Usage()
{
    cerr << "This pin tool replaces relocatable_1() and relocatable_2()\n"
            "\n";
    cerr << KNOB_BASE::StringKnobSummary();
    cerr << endl;
    return -1;
}

/* ===================================================================== */

void R1()
{
    cout << "R1: calling original relocatable_1()" << endl;

    return (pf_r1)();
}

int R1a()
{
    cout << "R1a: calling original relocatable_1a()" << endl;

    return (pf_r1a)();
}

void R2()
{
    cout << "R2: calling original relocatable_2()" << endl;

    return (pf_r2)();
}
void R3(FUNCPTR fptr)
{
    cout << "R3: calling original relocatable_3()" << endl;

    return (fptr)();
}

/* ===================================================================== */

VOID CheckRelocatable(const char* fname, IMG img, FUNCPTR* origFptr, AFUNPTR replacementFunc)
{
    RTN rtn = RTN_FindByName(img, fname);

    if (RTN_Valid(rtn))
    {
        //fprintf(stderr, "Attach to prs %d and debug %s\n", PIN_GetPid(), fname);
        //getchar();
        if (RTN_IsSafeForProbedReplacement(rtn))
        {
            cout << fname << " routine should not be probed w/o relocation" << endl;
            exit(-1);
        }
        if (RTN_IsSafeForProbedReplacementEx(rtn, PROBE_MODE_ALLOW_RELOCATION))
        {
            // Save the function pointer that points to the new location of
            // the entry point of the original exit in this image.
            //
            *origFptr = (FUNCPTR)RTN_ReplaceProbedEx(rtn, PROBE_MODE_ALLOW_RELOCATION, replacementFunc);

            cout << "ImageLoad: Replaced " << fname << " in:" << IMG_Name(img) << endl;
        }
        else
        {
            cout << "ImageLoad: Pin does not want to replace " << fname << " in:" << IMG_Name(img) << endl;
            exit(-1);
        }
    }
}

VOID CheckRelocatable1a(const char* fname, IMG img, FUNCPTR1a* origFptr, AFUNPTR replacementFunc)
{
    RTN rtn = RTN_FindByName(img, fname);

    if (RTN_Valid(rtn))
    {
        //fprintf(stderr, "Attach to prs %d and debug %s\n", PIN_GetPid(), fname);
        //getchar();
        if (RTN_IsSafeForProbedReplacement(rtn))
        {
            cout << fname << " routine should not be probed w/o relocation" << endl;
            exit(-1);
        }
        if (RTN_IsSafeForProbedReplacementEx(rtn, PROBE_MODE_ALLOW_RELOCATION))
        {
            // Save the function pointer that points to the new location of
            // the entry point of the original exit in this image.
            //
            *origFptr = (FUNCPTR1a)RTN_ReplaceProbedEx(rtn, PROBE_MODE_ALLOW_RELOCATION, replacementFunc);

            cout << "ImageLoad: Replaced " << fname << " in:" << IMG_Name(img) << endl;
        }
        else
        {
            cout << "ImageLoad: Pin does not want to replace " << fname << " in:" << IMG_Name(img) << endl;
            exit(-1);
        }
    }
}

VOID CheckRelocatableSig(const char* fname, IMG img, AFUNPTR replacementFunc)
{
    RTN rtn = RTN_FindByName(img, fname);

    if (RTN_Valid(rtn))
    {
        //fprintf(stderr, "Attach to prs %d and debug %s\n", PIN_GetPid(), fname);
        //getchar();
        if (RTN_IsSafeForProbedReplacement(rtn))
        {
            cout << fname << " routine should not be probed w/o relocation" << endl;
            exit(-1);
        }
        if (RTN_IsSafeForProbedReplacementEx(rtn, PROBE_MODE_ALLOW_RELOCATION))
        {
            PROTO proto = PROTO_Allocate(PIN_PARG(void), CALLINGSTD_DEFAULT, fname, PIN_PARG_END());

            RTN_ReplaceSignatureProbedEx(rtn, PROBE_MODE_ALLOW_RELOCATION, replacementFunc, IARG_PROTOTYPE, proto,
                                         IARG_ORIG_FUNCPTR, IARG_END);

            cout << "ImageLoad: Replaced " << fname << " in:" << IMG_Name(img) << endl;
        }
        else
        {
            cout << "ImageLoad: Pin does not want to replace " << fname << " in:" << IMG_Name(img) << endl;
            exit(-1);
        }
    }
}

VOID CheckNonRelocatable(const char* fname, IMG img)
{
    RTN rtn = RTN_FindByName(img, fname);

    if (RTN_Valid(rtn))
    {
        //fprintf(stderr, "Attach to prs %d and debug %s\n", PIN_GetPid(), fname);
        //getchar();
        if (RTN_IsSafeForProbedReplacement(rtn))
        {
            cout << fname << " routine should not be probed" << endl;
            exit(-1);
        }
        if (RTN_IsSafeForProbedReplacementEx(rtn, PROBE_MODE_ALLOW_RELOCATION))
        {
            cout << fname << " routine should not be probed" << endl;
            exit(-1);
        }
        else
        {
            cout << "ImageLoad: Pin behavior is right: reject to replace " << fname << " routine" << endl;
        }
    }
}

VOID ImageLoad(IMG img, VOID* v)
{
    CheckRelocatable(C_MANGLE("relocatable_1"), img, &pf_r1, AFUNPTR(R1));
    CheckRelocatable1a(C_MANGLE("relocatable_1a"), img, &pf_r1a, AFUNPTR(R1a));
    CheckRelocatable(C_MANGLE("relocatable_2"), img, &pf_r2, AFUNPTR(R2));
    CheckRelocatableSig(C_MANGLE("relocatable_3"), img, AFUNPTR(R3));
    CheckNonRelocatable(C_MANGLE("non_relocatable_1"), img);
    CheckNonRelocatable(C_MANGLE("non_relocatable_2"), img);
}

/* ===================================================================== */

int main(int argc, CHAR* argv[])
{
    PIN_InitSymbols();

    if (PIN_Init(argc, argv)) return Usage();

    IMG_AddInstrumentFunction(ImageLoad, 0);

    PIN_StartProgramProbed();

    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
