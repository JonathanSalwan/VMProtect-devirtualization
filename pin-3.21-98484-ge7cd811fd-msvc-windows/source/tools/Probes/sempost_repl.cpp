/*
 * Copyright (C) 2007-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/* ===================================================================== */
/*! @file
  Replaces sem_post(). Linux only, of course.
  On some OSes this routine can be probed only if Pin
  moves the whole routine to another place
 */

#include "pin.H"
#include <iostream>
#include <fstream>
using std::cerr;
using std::cout;
using std::endl;

/* ===================================================================== */
/* Global Variables */
/* ===================================================================== */

typedef int (*FUNCPTR)(void*);
static int (*pf_sempost)(void* arg);

/* ===================================================================== */

INT32 Usage()
{
    cerr << "This pin tool replaces sem_post()\n"
            "\n";
    cerr << KNOB_BASE::StringKnobSummary();
    cerr << endl;
    return -1;
}

/* ===================================================================== */

int SemPost(void* arg)
{
    cout << "SemPost: calling original sem_post() from pthread" << endl;

    return (pf_sempost)(arg);
}

/* ===================================================================== */

// Called every time a new image is loaded.
// Look for routines that we want to replace.
VOID ImageLoad(IMG img, VOID* v)
{
    RTN rtn = RTN_FindByName(img, "sem_post");

    if (RTN_Valid(rtn))
    {
        //fprintf(stderr, "Attach to prs %d\n", PIN_GetPid());
        //getchar();
        if (RTN_IsSafeForProbedReplacementEx(rtn, PROBE_MODE_ALLOW_RELOCATION))
        {
            // Save the function pointer that points to the new location of
            // the entry point of the original exit in this image.
            //
            pf_sempost = (FUNCPTR)RTN_ReplaceProbedEx(rtn, PROBE_MODE_ALLOW_RELOCATION, AFUNPTR(SemPost));

            cout << "ImageLoad: Replaced sem_post() in:" << IMG_Name(img) << endl;
        }
    }
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
