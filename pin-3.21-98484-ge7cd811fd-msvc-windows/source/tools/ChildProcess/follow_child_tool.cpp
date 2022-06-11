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

using std::cout;
using std::endl;
using std::flush;

// Whether to check current process id received in KnobCurrentProcessId
KNOB< BOOL > KnobLoadSystemDlls(KNOB_MODE_WRITEONCE, "pintool", "load_system_dlls", "0", "load system dlls in main()");

BOOL FollowChild(CHILD_PROCESS cProcess, VOID* userData)
{
    cout << "At follow child callback" << endl << flush;
    cout << "Child process id = " << CHILD_PROCESS_GetId(cProcess) << endl << flush;
    return TRUE;
}

VOID ImageLoad(IMG img, VOID* v) { cout << "Loading " << IMG_Name(img) << ", Image id = " << IMG_Id(img) << endl << flush; }

// Pin calls this function every time a new img is unloaded
// You can't instrument an image that is about to be unloaded
VOID ImageUnload(IMG img, VOID* v) { cout << "Unloading " << IMG_Name(img) << ", Image id = " << IMG_Id(img) << endl << flush; }

VOID AppStart(VOID* v) { cout << "Application started" << endl << flush; }

int main(INT32 argc, CHAR** argv)
{
    PIN_InitSymbols();

    PIN_Init(argc, argv);

    cout << "In tool's main, probed = " << decstr(PIN_IsProbeMode()) << endl << flush;
    cout << "Current process id = " << WIND::GetCurrentProcessId() << endl << flush;

    PIN_AddFollowChildProcessFunction(FollowChild, 0);

    IMG_AddInstrumentFunction(ImageLoad, 0);

    IMG_AddUnloadFunction(ImageUnload, 0);

    if (KnobLoadSystemDlls)
    {
        WIND::HMODULE h1 = WIND::LoadLibrary("RPCRT4.dll");
        if (h1 == NULL)
        {
            cout << "Failed to load RPCRT4" << endl << flush;
            exit(-1);
        }
        WIND::HMODULE h2 = WIND::LoadLibrary("advapi32.dll");
        if (h1 == NULL)
        {
            cout << "Failed to load advapi32" << endl << flush;
            exit(-1);
        }
        WIND::HMODULE h3 = WIND::LoadLibrary("dbghelp.dll");
        if (h1 == NULL)
        {
            cout << "Failed to load dbghelp" << endl << flush;
            exit(-1);
        }
        WIND::HMODULE h4 = WIND::LoadLibrary("user32.dll");
        if (h1 == NULL)
        {
            cout << "Failed to load user32" << endl << flush;
            exit(-1);
        }
    }

    // Never returns
    if (PIN_IsProbeMode())
    {
        PIN_AddApplicationStartFunction(AppStart, 0);
        PIN_StartProgramProbed();
    }
    else
    {
        PIN_StartProgram();
    }

    return 0;
}
