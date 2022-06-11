/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 *  pin tool combined from multi-DLLs (main_dll, dynamic_secondary_dll, static_secondary_dll). 
 *  This is the "main DLL", use PIN API only in this DLL
 *  usage of PIN API in dynamic_secondary_dll and static_secondary_dll is not allowed
 *  (see README for more inforamtion)
 *
 *  NOTE: New Pin image loader supports dynamic loading of Pin DLLs.
 *        Look at Mantis 3280 for implementation details.
 *        The test also validates the dynamic loading feature.
 */

#include <iostream>
#include <fstream>

#include <link.h>
#include <dlfcn.h>

#include "pin.H"

using std::cerr;
using std::endl;
using std::hex;
using std::string;

KNOB< BOOL > KnobEnumerate(KNOB_MODE_WRITEONCE, "pintool", "enumerate", "0", "Enumerate modules loaded by Pin");

KNOB< string > KnobOutputFile1(KNOB_MODE_WRITEONCE, "pintool", "o1", "static_secondary_dll.out", "Output file 1");

KNOB< string > KnobOutputFile2(KNOB_MODE_WRITEONCE, "pintool", "o2", "dynamic_secondary_dll.out", "Output file 2");

/* ===================================================================== */
/* Global Variables and Declerations */
/* ===================================================================== */

PIN_LOCK pinLock;

typedef VOID (*BEFORE_BBL)(ADDRINT ip);
typedef int (*INIT_F)(bool enumerate, const char* out_filename);
typedef VOID (*FINI_F)();

// Functions pointers for dynamic_secondary_dll
BEFORE_BBL pBeforeBBL2;
INIT_F pInit2;
FINI_F pFini2;

// Dll imports for static_secondary_dll
extern "C" __declspec(dllimport) VOID BeforeBBL1(ADDRINT ip);
extern "C" __declspec(dllimport) VOID Init1(const char*);
extern "C" __declspec(dllimport) VOID Fini1();

/* ===================================================================== */

// This function is called before every basic block
VOID PIN_FAST_ANALYSIS_CALL BeforeBBL(ADDRINT ip)
{
    PIN_GetLock(&pinLock, PIN_GetTid());
    BeforeBBL1(ip);
    pBeforeBBL2(ip);
    PIN_ReleaseLock(&pinLock);
}

/* ===================================================================== */

// Pin calls this function every time a new trace is encountered
VOID Trace(TRACE trace, VOID* v)
{
    // Visit every basic block  in the trace
    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
    {
        // Insert a call to BeforeBBL before every bbl, passing the ip address.
        BBL_InsertCall(bbl, IPOINT_BEFORE, (AFUNPTR)BeforeBBL, IARG_FAST_ANALYSIS_CALL, IARG_INST_PTR, IARG_END);
    }
}

VOID ThreadStart(THREADID threadid, CONTEXT* ctxt, INT32 flags, VOID* v)
{
    PIN_GetLock(&pinLock, PIN_GetTid());
    BeforeBBL1(0);
    pBeforeBBL2(0);
    PIN_ReleaseLock(&pinLock);
}

VOID ThreadFini(THREADID threadid, const CONTEXT* ctxt, INT32 code, VOID* v)
{
    PIN_GetLock(&pinLock, PIN_GetTid());
    BeforeBBL1(0);
    pBeforeBBL2(0);
    PIN_ReleaseLock(&pinLock);
}

/* ===================================================================== */

// This function is called when the application exits
VOID Fini(INT32 code, VOID* v)
{
    Fini1();
    pFini2();
}

// This function gets info of an image loaded by Pin loader.
// Invoked by dl_iterate_phdr()
int dl_iterate_callback(struct dl_phdr_info* info, size_t size, VOID* data)
{
    cerr << info->dlpi_name << " " << hex << info->dlpi_addr << " " << info->dlpi_phdr->p_memsz << endl;
    // Increment module counter.
    ++(*reinterpret_cast< int* >(data));
    return 0;
}

/* ===================================================================== */

int main(int argc, char* argv[])
{
    // Initialize pin
    PIN_Init(argc, argv);

    PIN_InitLock(&pinLock);

    // Register Trace() to be called to instrument traces
    TRACE_AddInstrumentFunction(Trace, 0);

    // Register Fini() to be called when the application exits
    PIN_AddFiniFunction(Fini, 0);

    // Call Static secondary dll Init1()
    Init1(KnobOutputFile1.Value().c_str());

    int nModules;

    // Dynamic secondary dll - load library, initialize function pointers
    // and call Init2()
    VOID* module = dlopen("dynamic_secondary_dll.dll", RTLD_NOW);
    if (module == NULL)
    {
        cerr << "Failed to load dynamic_secondary_dll.dll" << endl;
        exit(1);
    }
    pInit2      = reinterpret_cast< INIT_F >(dlsym(module, "Init2"));
    pBeforeBBL2 = reinterpret_cast< BEFORE_BBL >(dlsym(module, "BeforeBBL2"));
    pFini2      = reinterpret_cast< FINI_F >(dlsym(module, "Fini2"));
    if (pInit2 == NULL || pBeforeBBL2 == NULL || pFini2 == NULL)
    {
        cerr << "Failed to find proc addresses in dynamic_secondary_dll.dll" << endl;
        exit(1);
    }

    nModules = pInit2(KnobEnumerate, KnobOutputFile2.Value().c_str());

    int nModulesMain = 0;
    // Enumerate DLLs currently loaded by Pin loader.
    dl_iterate_phdr(dl_iterate_callback, &nModulesMain);

    if (KnobEnumerate && ((nModulesMain <= 0) || (nModulesMain != nModules)))
    {
        // Failure. Module enumeration results in main and dynamic Dlls don't match.
        PIN_ExitApplication(1);
    }

    // Start the program, never returns
    PIN_StartProgram();

    return 0;
}
