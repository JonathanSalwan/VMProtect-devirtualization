/*
 * Copyright (C) 2007-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/* ===================================================================== */
/*! @file
  Replaces pthread_spin_lock(). Linux only, of course.
 */

#include <stdlib.h>

#include "pin.H"
#include <iostream>
#include <fstream>
using std::cerr;
using std::cout;
using std::endl;

/* ===================================================================== */
/* Global Variables */
/* ===================================================================== */

// Don't use pthread_spinlock_t* in pthread_spin_lock signature since
// our pthread_spin_lock type may be different than the application's libc
// pthread_spin_lock.
typedef int (*FUNCPTR)(void*);
static int (*pf_spinlock)(void* lock);

/* ===================================================================== */

INT32 Usage()
{
    cerr << "This pin tool replaces pthread_spin_lock()\n"
            "\n";
    cerr << KNOB_BASE::StringKnobSummary();
    cerr << endl;
    return -1;
}

/* ===================================================================== */

int SpinLock(void* lock)
{
    cout << "SpinLock: calling pthread_spin_lock()" << endl;

    return (pf_spinlock)(lock);
}

/* ===================================================================== */

// Called every time a new image is loaded.
// Look for routines that we want to replace.
VOID ImageLoad(IMG img, VOID* v)
{
    RTN rtn = RTN_FindByName(img, "pthread_spin_lock");

    if (RTN_Valid(rtn))
    {
        if (RTN_IsSafeForProbedReplacementEx(rtn, PROBE_MODE_ALLOW_RELOCATION))
        {
            // Save the function pointer that points to the new location of
            // the entry point of the original exit in this image.
            //
            pf_spinlock = (FUNCPTR)RTN_ReplaceProbedEx(rtn, PROBE_MODE_ALLOW_RELOCATION, AFUNPTR(SpinLock));

            cout << "ImageLoad: Replaced pthread_spin_lock() in:" << IMG_Name(img) << endl;
        }
        else
        {
            cout << "ImageLocad: Can't replace pthread_spin_lock() in:" << IMG_Name(img) << endl;
            exit(-1);
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
