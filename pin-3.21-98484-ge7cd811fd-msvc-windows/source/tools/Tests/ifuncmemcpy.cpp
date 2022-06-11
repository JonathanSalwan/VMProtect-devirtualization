/*
 * Copyright (C) 2011-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include "pin.H"
#include <iostream>
#include <fstream>
#include <string.h>
using std::cerr;
using std::cout;
using std::endl;
using std::hex;
using std::ios;
using std::string;

/* ===================================================================== */
/* Global Variables */
/* ===================================================================== */

std::ofstream TraceFile;

/* ===================================================================== */
/* Commandline Switches */
/* ===================================================================== */

KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "", "specify trace file name");

/* ===================================================================== */

/* ===================================================================== */
/* Analysis routines                                                     */
/* ===================================================================== */

VOID MemcpyBefore(CHAR* name, ADDRINT dst, ADDRINT src, ADDRINT size)
{
    TraceFile << name << "(" << dst << "," << src << "," << size << ")" << endl;
}

// Capture the return address of the ifunc which is the address of the actual memcpy
VOID* IfuncMemcpyWrapper(CONTEXT* context, AFUNPTR orgFuncptr)
{
    VOID* ret;

    PIN_CallApplicationFunction(context, PIN_ThreadId(), CALLINGSTD_DEFAULT, orgFuncptr, NULL, PIN_PARG(void*), &ret,
                                PIN_PARG_END());
    TraceFile << "memcpy (ifunc)" << endl;
    cout << "ifunc_memcpy("
         << ") returns " << hex << ret << endl;
    return ret;
}

/* ===================================================================== */
/* Instrumentation routines                                              */
/* ===================================================================== */

VOID Routine(IMG img, RTN rtn, void* v)
{
    // In some libc implementations, memcpy, memmove  symbols have the same address.
    // In this case, since Pin only creates one RTN per start address, the RTN name
    // will be either memcpy, memmove.
    bool isMemmove = strcmp(RTN_Name(rtn).c_str(), "memmove") == 0;
    bool isMemcpy  = strcmp(RTN_Name(rtn).c_str(), "memcpy") == 0;

    if (isMemmove || isMemcpy)
    {
        if (SYM_IFuncResolver(RTN_Sym(rtn)))
        {
            TraceFile << "Ifunc memcpy" << endl;

            PROTO proto_ifunc_memcpy = PROTO_Allocate(PIN_PARG(void*), CALLINGSTD_DEFAULT, "memcpy", PIN_PARG_END());

            RTN_ReplaceSignature(rtn, AFUNPTR(IfuncMemcpyWrapper), IARG_PROTOTYPE, proto_ifunc_memcpy, IARG_CONTEXT,
                                 IARG_ORIG_FUNCPTR, IARG_END);
        }
        else
        {
            RTN_Open(rtn);

            TraceFile << "Normal memcpy" << endl;

            // Instrument memcpy() to print the input argument value and the return value.
            RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)MemcpyBefore, IARG_ADDRINT, "memcpy (normal)",
                           IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_FUNCARG_ENTRYPOINT_VALUE, 1, IARG_FUNCARG_ENTRYPOINT_VALUE, 2,
                           IARG_END);

            RTN_Close(rtn);
        }
    }
}

VOID Image(IMG img, VOID* v)
{
    for (SEC sec = IMG_SecHead(img); SEC_Valid(sec); sec = SEC_Next(sec))
    {
        if (SEC_IsExecutable(sec))
        {
            for (RTN rtn = SEC_RtnHead(sec); RTN_Valid(rtn); rtn = RTN_Next(rtn))
                Routine(img, rtn, v);
        }
    }
}
/* ===================================================================== */

VOID Fini(INT32 code, VOID* v) { TraceFile.close(); }

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage()
{
    cerr << "This tool produces a trace of calls to normal and ifunc memcpy." << endl;
    cerr << endl << KNOB_BASE::StringKnobSummary() << endl;
    return -1;
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

int main(int argc, char* argv[])
{
    // Initialize pin & symbol manager
    PIN_InitSymbolsAlt(SYMBOL_INFO_MODE(UINT32(IFUNC_SYMBOLS) | UINT32(DEBUG_OR_EXPORT_SYMBOLS)));
    if (PIN_Init(argc, argv))
    {
        return Usage();
    }

    if ((KnobOutputFile.Value()).empty())
    {
        cerr << "No output file name has been provided to the tool" << endl;
    }

    // Write to a file since cout and cerr maybe closed by the application
    TraceFile.open(KnobOutputFile.Value().c_str());
    TraceFile << hex;
    TraceFile.setf(ios::showbase);

    // Register Image to be called to instrument functions.
    IMG_AddInstrumentFunction(Image, 0);

    PIN_AddFiniFunction(Fini, 0);

    // Never returns
    PIN_StartProgram();

    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
