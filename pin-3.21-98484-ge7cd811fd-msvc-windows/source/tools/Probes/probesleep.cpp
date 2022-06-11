/*
 * Copyright (C) 2006-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

//
// This tool replaces sleep().
//

#include "pin.H"
#include <iostream>
#include "tool_macros.h"
using std::cerr;
using std::endl;
using std::string;

#if defined(TARGET_MAC) && defined(TARGET_IA32)
#define SLEEP_SUFFIX "$UNIX2003"
#else
#define SLEEP_SUFFIX
#endif

/* ===================================================================== */
/* Global Variables */
/* ===================================================================== */

static void (*pf_sleep)(int);

/* ===================================================================== */
/* Replacement Functions */
/* ===================================================================== */

void SleepProbe(int b)
{
    cerr << "SleepProbe: calling sleep" << endl;

    if (pf_sleep)
    {
        (pf_sleep)(b);
    }
}

/* ===================================================================== */

#if defined(TARGET_WINDOWS)

// Returns TRUE if baseName matches tail of imageName. Comparison is case-insensitive.
// Parameters:
//   imageName  image file name in either form with extension
//   baseName   image base name with extension (e.g. kernel32.dll)
BOOL MatchedImageName(const string& imageName, const string& baseName)
{
    if (imageName.size() >= baseName.size())
    {
        return _stricmp(imageName.c_str() + imageName.size() - baseName.size(), baseName.c_str()) == 0;
    }
    return FALSE;
}

#endif

/* ===================================================================== */

// Called every time a new image is loaded
// Look for routines that we want to probe

VOID ImageLoad(IMG img, VOID* v)
{
#if defined(TARGET_WINDOWS)
    RTN sleepRtn = RTN_Invalid();
    // Look for Sleep only in kernel32.dll
    if (MatchedImageName(IMG_Name(img), "kernel32.dll"))
    {
        sleepRtn = RTN_FindByName(img, C_MANGLE("Sleep"));
    }
#else
    RTN sleepRtn = RTN_FindByName(img, C_MANGLE("sleep") SLEEP_SUFFIX);
#endif

    if (RTN_Valid(sleepRtn))
    {
        if (RTN_IsSafeForProbedReplacement(sleepRtn))
        {
            pf_sleep = (void (*)(int))RTN_ReplaceProbed(sleepRtn, AFUNPTR(SleepProbe));

            cerr << "Inserted probe for Sleep:" << IMG_Name(img) << endl;
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
