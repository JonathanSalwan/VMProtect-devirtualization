/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include "pin.H"
#include <iostream>

namespace WIND
{
#include <windows.h>
}
using std::cerr;
using std::cout;
using std::endl;
using std::flush;
using std::string;
/* ===================================================================== */
/* Commandline Switches */
/* ===================================================================== */

/* ===================================================================== */
/* Global variables and declarations */
/* ===================================================================== */

// Number of application threads to validate.
// This number corresponds to NTHREADS in w_app3 application.
#define NTHREADS 5
struct
{
    THREADID id;   // Thread id
    UINT32 ncalls; // Number of DoLoop() calls in this thread
} tlist[NTHREADS]; // Global object. Zero-initialized.
// Min positive number of DoLoop() calls in specific thread to qualify for pass.
// Arbitrary number, enough for decisive validation.
#define PASS_COUNT 10

typedef int(__cdecl* DO_LOOP_TYPE)();

/* ===================================================================== */

// This replacement function starts to return 0 once all validated threads
// called it at least PASS_COUNT times.
// Return value 0 causes calling threads to finish gracefully.
int rep_DoLoop()
{
    static UINT32 passedThreads = 0;

    THREADID tid = PIN_ThreadId();
    UINT32 i;
    PIN_LockClient();
    for (i = 0; i < NTHREADS; ++i)
    {
        if (tlist[i].ncalls == 0)
        {
            // This thread calls DoLoop() first time. Register it.
            tlist[i].id = tid;
        }
        else if (tlist[i].id != tid)
        {
            continue;
        }
        if (++tlist[i].ncalls == PASS_COUNT)
        {
            // This thread achieved pass criteria.
            ++passedThreads;
        }
        break;
    }
    PIN_UnlockClient();
    ASSERTX(i < NTHREADS); // Number of threads that ever called DoLoop doesn't exceed NTHREADS.

    return passedThreads < NTHREADS;
}

/* ===================================================================== */
VOID ImageLoad(IMG img, VOID* v)
{
    cout << endl << IMG_Name(img);

    if (!IMG_IsMainExecutable(img))
    {
        return;
    }
    const string sFuncName("DoLoop");

    for (SYM sym = IMG_RegsymHead(img); SYM_Valid(sym); sym = SYM_Next(sym))
    {
        string undFuncName = PIN_UndecorateSymbolName(SYM_Name(sym), UNDECORATION_NAME_ONLY);
        if (undFuncName == sFuncName)
        {
            RTN rtn = RTN_FindByAddress(IMG_LowAddress(img) + SYM_Value(sym));
            if (RTN_Valid(rtn))
            {
                // Even though this is not an error - print to cerr (in order to see it on the screen)
                cerr << "Replacing DoLoop() in " << IMG_Name(img) << endl;

                RTN_Replace(rtn, AFUNPTR(rep_DoLoop));
            }
        }
    }
}

int main(INT32 argc, CHAR** argv)
{
    PIN_InitSymbols();

    PIN_Init(argc, argv);

    IMG_AddInstrumentFunction(ImageLoad, 0);

    std::cerr << "Application is starting" << endl << flush;

    // Never returns
    PIN_StartProgram();

    return 0;
}
