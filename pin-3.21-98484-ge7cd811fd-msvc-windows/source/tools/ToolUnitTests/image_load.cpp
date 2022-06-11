/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

//
// This tool prints a trace of image load and unload events
//

#include <stdio.h>
#include "pin.H"

namespace WIND
{
#include <windows.h>
}

// Whether to check current process id received in KnobCurrentProcessId
KNOB< BOOL > KnobLoadSystemDlls(KNOB_MODE_WRITEONCE, "pintool", "load_system_dlls", "0", "load system dlls in main()");

// Pin calls this function every time a new img is loaded
// It can instrument the image, but this example does not
// Note that imgs (including shared libraries) are loaded lazily

VOID ImageLoad(IMG img, VOID* v)
{
    printf("Loading %s, Image id = %d\n", IMG_Name(img).c_str(), IMG_Id(img));
    fflush(stdout);
}

// Pin calls this function every time a new img is unloaded
// You can't instrument an image that is about to be unloaded
VOID ImageUnload(IMG img, VOID* v)
{
    printf("Unloading %s\n", IMG_Name(img).c_str());
    fflush(stdout);
}

VOID ThreadStart(THREADID threadid, CONTEXT* ctxt, INT32 flags, VOID* v)
{
    static THREADID myThread = INVALID_THREADID;
    if (INVALID_THREADID == myThread)
    {
        myThread = threadid;
        printf("Got thread start notification\n");
    }
    fflush(stdout);
}

void FiniCore()
{
    printf("Application finished\n");
    fflush(stdout);
}

VOID Fini(INT32 code, VOID* v) { FiniCore(); }

class PROBE_FINI_OBJECT
{
  public:
    ~PROBE_FINI_OBJECT() { FiniCore(); }
};

VOID AppStart(VOID* v)
{
    printf("Application started\n");
    fflush(stdout);
}

VOID AppThreadStart()
{
    printf("Got thread start notification\n");
    fflush(stdout);
}

//Instrument the app thread rtn
VOID InstrumentRtn(RTN rtn, VOID*)
{
    if (PIN_UndecorateSymbolName(RTN_Name(rtn), UNDECORATION_NAME_ONLY) == "ThreadProc")
    {
        RTN_Open(rtn);
        RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(AppThreadStart), IARG_END);
        RTN_Close(rtn);
    }
}

// argc, argv are the entire command line, including pin -t <toolname> -- ...
int main(int argc, char* argv[])
{
    // Initialize symbol processing
    PIN_InitSymbols();

    // Initialize pin
    PIN_Init(argc, argv);

    RTN_AddInstrumentFunction(InstrumentRtn, NULL);

    // Register ImageLoad to be called when an image is loaded
    IMG_AddInstrumentFunction(ImageLoad, NULL);

    // Register ImageUnload to be called when an image is unloaded
    IMG_AddUnloadFunction(ImageUnload, NULL);

    printf("In tool's main, probed = %d\n", PIN_IsProbeMode());

    if (KnobLoadSystemDlls)
    {
        WIND::HMODULE h1 = WIND::LoadLibrary("RPCRT4.dll");
        if (h1 == NULL)
        {
            printf("Failed to load RPCRT4\n");
            fflush(stdout);
            exit(-1);
        }
        WIND::HMODULE h2 = WIND::LoadLibrary("advapi32.dll");
        if (h1 == NULL)
        {
            printf("Failed to load advapi32\n");
            fflush(stdout);
            exit(-1);
        }
        WIND::HMODULE h3 = WIND::LoadLibrary("dbghelp.dll");
        if (h1 == NULL)
        {
            printf("Failed to load dbghelp\n");
            fflush(stdout);
            exit(-1);
        }
        WIND::HMODULE h4 = WIND::LoadLibrary("user32.dll");
        if (h1 == NULL)
        {
            printf("Failed to load user32\n");
            fflush(stdout);
            exit(-1);
        }
    }

    if (PIN_IsProbeMode())
    {
        static PROBE_FINI_OBJECT finiObject;
        PIN_AddApplicationStartFunction(AppStart, NULL);
        PIN_StartProgramProbed();
    }
    else
    {
        PIN_AddThreadStartFunction(ThreadStart, NULL);
        PIN_AddFiniFunction(Fini, NULL);
        PIN_StartProgram();
    }
    // Never returns

    return 1;
}
