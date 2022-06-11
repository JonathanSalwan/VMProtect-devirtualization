/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 * Replace main's executable entry point and DLL entry point
 */

#include "pin.H"
#include <iostream>
#include <fstream>
namespace WIND
{
#include <windows.h>
}

using std::cerr;
using std::endl;
using std::hex;
using std::ios;
using std::ofstream;
using std::string;
/* ===================================================================== */
/* Commandline Switches */
/* ===================================================================== */

KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "out_file", "image_entry.out", "specify image entry file name");

KNOB< BOOL > KnobVerbose(KNOB_MODE_WRITEONCE, "pintool", "verbose", "0", "verbosity level - 0 / 1");

KNOB< BOOL > KnobPseudoFini(KNOB_MODE_WRITEONCE, "pintool", "pseudofini", "0",
                            "Enable pseudo-Fini functionality in probe mode - 0 / 1");

KNOB< string > KnobDllName(KNOB_MODE_WRITEONCE, "pintool", "dll_name", "win_tls_dll.dll",
                           "specify DLL name whose entry point to probe");

/* ===================================================================== */

INT32 Usage()
{
    cerr << "This pin tool replaces main's executable entry point \
             and DLL entry point."
         << endl;
    cerr << KNOB_BASE::StringKnobSummary();
    cerr << endl;
    cerr.flush();
    return -1;
}

/* ===================================================================== */
/* Global definitions*/
/* ===================================================================== */

//Signature of main executable entry point
typedef int (*EXE_ENTRY_POINT)();

//Signature of dll entry point
typedef WIND::BOOLEAN(WINAPI* DLL_ENTRY_POINT)(WIND::HINSTANCE hDllHandle, WIND::DWORD nReason, WIND::LPVOID Reserved);

/* ===================================================================== */
/* Global Variables */
/* ===================================================================== */
//output file
ofstream traceFile;
//lock
PIN_LOCK pinLock;
//counter of number of exe entry point entrances
UINT32 exeEntryCounter = 0;
//counter of number of DLL entry point entrances, reason = THREAD_ATTACH
UINT32 dllEntryCounterThreadAttach = 0;
//counter of number of DLL entry point entrances, reason = THREAD_DETACH
UINT32 dllEntryCounterThreadDetach = 0;
//this variable is set to 1 when the application start callback is called
static volatile int isAppStarted = 0;

/* ===================================================================== */
// Fini code
/* ===================================================================== */
VOID CoreFini()
{
    traceFile << "exeEntryCounter = " << exeEntryCounter << endl;
    traceFile << "dllEntryCounterThreadAttach = " << dllEntryCounterThreadAttach << endl;
    traceFile << "dllEntryCounterThreadDetach = " << dllEntryCounterThreadDetach << endl;
    traceFile.close();
}

// Pseudo-Fini flag
bool pseudoFini;

// Pseudo-Fini functionality in probe mode could be achieved
// by using destructors of static objects.
// Pin guarantees invocation of the destructors just before Win32 subsystem uninitialization
// when CRT is still functional.
// Declare class with destructor that performs Fini actions.
class PROBE_FINI_OBJECT
{
  public:
    ~PROBE_FINI_OBJECT()
    {
        if (pseudoFini)
        {
            if (isAppStarted == 0)
            {
                traceFile << "AppStart() was not called" << endl;
            }
            CoreFini();
        }
    }
};

VOID Fini(INT32 code, VOID* v) { CoreFini(); }

VOID AppStart(VOID* v) { isAppStarted = 1; }

/* ===================================================================== */
/* Main executable entry point (before - JIT / replacement - PROBE) */
/* ===================================================================== */
//Used in JIT mode
void BeforeExeEntry()
{
    PIN_GetLock(&pinLock, PIN_GetTid());
    exeEntryCounter++;
    if (KnobVerbose)
    {
        traceFile << "In exe entry point, threadid = " << PIN_GetTid() << endl;
    }
    PIN_ReleaseLock(&pinLock);
}

//Used in PROBE mode
int MyExeEntry(CONTEXT* context, EXE_ENTRY_POINT orig_exeEntry)
{
    PIN_GetLock(&pinLock, PIN_GetTid());
    exeEntryCounter++;
    if (KnobVerbose)
    {
        traceFile << "In exe entry point, threadid = " << PIN_GetTid() << endl;
    }
    PIN_ReleaseLock(&pinLock);

    return orig_exeEntry();
}

/* ===================================================================== */
/* DLL entry point replacement */
/* ===================================================================== */
WIND::BOOLEAN WINAPI MyDllEntry(CONTEXT* context, DLL_ENTRY_POINT orig_dllEntry, WIND::HINSTANCE hDllHandle, WIND::DWORD nReason,
                                WIND::LPVOID Reserved)
{
    PIN_GetLock(&pinLock, PIN_GetTid());
    if (nReason == DLL_THREAD_ATTACH)
    {
        dllEntryCounterThreadAttach++;
    }
    else if (nReason == DLL_THREAD_DETACH)
    {
        dllEntryCounterThreadDetach++;
    }
    if (KnobVerbose)
    {
        traceFile << "In DLL entry point,  threadid = " << PIN_GetTid() << ", dll handle = " << hDllHandle
                  << ", reason  = " << nReason << endl;
    }
    PIN_ReleaseLock(&pinLock);

    WIND::BOOLEAN ret;
    if (PIN_IsProbeMode())
    {
        ret = orig_dllEntry(hDllHandle, nReason, Reserved);
        if (!pseudoFini && (nReason == DLL_PROCESS_DETACH))
        {
            // It is called at late exit stage when all threads but current are finished.
            CoreFini();
        }
    }
    else
    {
        //FIXME : Use PIN_CallApplicationFunction when it possible
        //        (Fix for mantis 1122/1123)
#if 1
        ret = orig_dllEntry(hDllHandle, nReason, Reserved);
#else
        PIN_CallApplicationFunction(context, PIN_ThreadId(), CALLINGSTD_STDCALL, AFUNPTR(orig_dllEntry), NULL,
                                    PIN_PARG(WIND::BOOLEAN), &ret, PIN_PARG(WIND::HINSTANCE), hDllHandle, PIN_PARG(WIND::DWORD),
                                    nReason, PIN_PARG(WIND::LPVOID), Reserved, PIN_PARG_END());
#endif
    }
    ASSERTX(ret);
    return ret;
}

/* ===================================================================== */
/* Replace (PROBE) / Before (JIT) exe entry point (in exe image load event) */
/* ===================================================================== */
VOID ReplaceExeEntryPoint(IMG img)
{
    if (!IMG_IsMainExecutable(img))
    {
        return;
    }

    // This is main executable, find it's entry point and replace it
    RTN rtn = RTN_FindByAddress(IMG_EntryAddress(img));
    ASSERTX(RTN_Valid(rtn));
    if (KnobVerbose)
    {
        traceFile << "Replacing Exe entry point, Address = " << RTN_Address(rtn) << ", Name = \"" << RTN_Name(rtn).c_str()
                  << "\", Size = " << RTN_Size(rtn) << endl;
    }

    if (PIN_IsProbeMode())
    {
        PROTO proto_exeEntry = PROTO_Allocate(PIN_PARG(int), CALLINGSTD_DEFAULT, "", PIN_PARG_END());
        ASSERTX(RTN_IsSafeForProbedReplacement(rtn));
        RTN_ReplaceSignatureProbed(rtn, AFUNPTR(MyExeEntry), IARG_PROTOTYPE, proto_exeEntry, IARG_CONTEXT, IARG_ORIG_FUNCPTR,
                                   IARG_END);
        PROTO_Free(proto_exeEntry);
    }
    else
    {
        RTN_Open(rtn);
        RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(BeforeExeEntry), IARG_END);
        RTN_Close(rtn);
    }
}

/* ===================================================================== */
/* Replace DLL entry point */
/* ===================================================================== */
VOID ReplaceDllEntryPoint(IMG img, const string& dllName, AFUNPTR replacementFunction)
{
    string imagePath = IMG_Name(img);
    if (!dllName.empty())
    {
        string::size_type index = imagePath.find(dllName);
        if (index == string::npos)
        {
            return;
        }
    }

    // This is the dll we are looking for, find it's entry point and replace it
    RTN rtn = RTN_FindByAddress(IMG_EntryAddress(img));
    ASSERTX(RTN_Valid(rtn));
    if (KnobVerbose)
    {
        traceFile << "Replacing " << imagePath << " entry point, Address = " << RTN_Address(rtn) << ", Name = \""
                  << RTN_Name(rtn).c_str() << "\", Size = " << RTN_Size(rtn) << endl;
    }
    PROTO proto_dllEntry = PROTO_Allocate(PIN_PARG(WIND::BOOLEAN), CALLINGSTD_STDCALL, "", PIN_PARG(WIND::HINSTANCE),
                                          PIN_PARG(WIND::DWORD), PIN_PARG(WIND::LPVOID), PIN_PARG_END());
    if (PIN_IsProbeMode())
    {
        ASSERTX(RTN_IsSafeForProbedReplacement(rtn));
        RTN_ReplaceSignatureProbed(rtn, replacementFunction, IARG_PROTOTYPE, proto_dllEntry, IARG_CONTEXT, IARG_ORIG_FUNCPTR,
                                   IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_FUNCARG_ENTRYPOINT_VALUE, 1,
                                   IARG_FUNCARG_ENTRYPOINT_VALUE, 2, IARG_END);
    }
    else
    {
        RTN_ReplaceSignature(rtn, replacementFunction, IARG_PROTOTYPE, proto_dllEntry, IARG_CONTEXT, IARG_ORIG_FUNCPTR,
                             IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_FUNCARG_ENTRYPOINT_VALUE, 1, IARG_FUNCARG_ENTRYPOINT_VALUE, 2,
                             IARG_END);
    }
    PROTO_Free(proto_dllEntry);
}

/* ===================================================================== */
// Called every time a new image is loaded
// Look for routines that we want to replace
/* ===================================================================== */
VOID ImageLoad(IMG img, VOID* v)
{
    ReplaceExeEntryPoint(img);
    ReplaceDllEntryPoint(img, KnobDllName.Value(), AFUNPTR(MyDllEntry));
}

/* ===================================================================== */
// main function. Initialize and start probe/jit
/* ===================================================================== */
int main(int argc, CHAR* argv[])
{
    PIN_InitSymbols();

    if (PIN_Init(argc, argv))
    {
        return Usage();
    }

    pseudoFini = KnobPseudoFini;

    traceFile.open(KnobOutputFile.Value().c_str());
    traceFile << hex;
    traceFile.setf(ios::showbase);

    PIN_InitLock(&pinLock);

    IMG_AddInstrumentFunction(ImageLoad, 0);

    if (PIN_IsProbeMode())
    {
        PIN_AddApplicationStartFunction(AppStart, 0);

        // Define static object whose destructor would provide Fini functionality
        static PROBE_FINI_OBJECT finiObject;

        PIN_StartProgramProbed();
    }
    else
    {
        PIN_AddFiniFunction(Fini, 0);
        PIN_StartProgram();
    }

    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
