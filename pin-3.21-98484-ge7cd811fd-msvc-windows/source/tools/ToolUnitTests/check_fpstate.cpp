/*
 * Copyright (C) 2004-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <iostream>
#include <fstream>
#include <cstring>
#include "pin.H"
#include "../Utils/regvalue_utils.h"
using std::cerr;
using std::endl;
using std::hex;
using std::ios;

/* ===================================================================== */
/* Names of malloc and free */
/* ===================================================================== */
#if defined(TARGET_MAC)
#define FN "_trap_me"
#else
#define FN "trap_me"
#endif

/* ===================================================================== */
/* Global Variables */
/* ===================================================================== */

std::ofstream out;

/* ===================================================================== */
/* Commandline Switches */
/* ===================================================================== */

KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "check_fpstate.out", "specify output file name");

/* ===================================================================== */

static CONTEXT saveCtxt;
static BOOL enable = FALSE;

/* ===================================================================== */
/* Analysis routines                                                     */
/* ===================================================================== */

VOID CompreContext(CONTEXT* c1, CONTEXT* c2)
{
    struct
    {
        REG reg;
        const char* name;
    } fields[] = {{REG_FPIP_OFF, "ipoff"},
                  {REG_FPIP_SEL, "ipsel"},
                  {REG_FPOPCODE, "opcod"},
                  {REG_FPDP_OFF, "dpoff"},
                  {REG_FPDP_SEL, "dpsel"}};

    for (unsigned int i = 0; i < sizeof(fields) / sizeof(fields[0]); i++)
    {
        REG reg   = fields[i].reg;
        UINT size = REG_Size(reg);
        UINT8* v1 = new UINT8[size];
        UINT8* v2 = new UINT8[size];

        PIN_GetContextRegval(c1, reg, v1);
        PIN_GetContextRegval(c2, reg, v2);

        if (memcmp(v1, v2, size) == 0)
            out << "Same ";
        else
            out << "Mismatch ";
        out << fields[i].name << " " << Val2Str(v1, size) << " " << Val2Str(v2, size) << endl;

        delete[] v1;
        delete[] v2;
    }
}

VOID Enable() { enable = TRUE; }

VOID SaveContext(CONTEXT* ctxt)
{
    if (!enable) return;

    PIN_SaveContext(ctxt, &saveCtxt);
}

VOID SyscallEntry(THREADID tid, CONTEXT* ctxt, SYSCALL_STANDARD std, VOID* v)
{
    if (!enable) return;

    CompreContext(ctxt, &saveCtxt);

    enable = FALSE;
}

/* ===================================================================== */
/* Instrumentation routines                                              */
/* ===================================================================== */

VOID Image(IMG img, VOID* v)
{
    // Instrument the trapme function to activate the analysis.
    RTN rtn = RTN_FindByName(img, FN);
    if (RTN_Valid(rtn))
    {
        RTN_Open(rtn);

        RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)Enable, IARG_END);

        RTN_Close(rtn);
    }
}

VOID Trace(TRACE trace, VOID* v)
{
    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
    {
        for (INS ins = BBL_InsHead(bbl); INS_Valid(ins); ins = INS_Next(ins))
        {
            if (INS_IsSyscall(ins))
            {
                INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)SaveContext, IARG_CONTEXT, IARG_END);
            }
        }
    }
}

/* ===================================================================== */

VOID Fini(INT32 code, VOID* v) { out.close(); }

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage()
{
    cerr << "This tool produces a trace of calls to malloc." << endl;
    cerr << endl << KNOB_BASE::StringKnobSummary() << endl;
    return -1;
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

int main(int argc, char* argv[])
{
    // Initialize pin & symbol manager
    PIN_InitSymbols();
    if (PIN_Init(argc, argv))
    {
        return Usage();
    }

    // Write to a file since cout and cerr maybe closed by the application
    out.open(KnobOutputFile.Value().c_str());
    out << hex;
    out.setf(ios::showbase);

    IMG_AddInstrumentFunction(Image, 0);
    TRACE_AddInstrumentFunction(Trace, 0);
    PIN_AddSyscallEntryFunction(SyscallEntry, 0);
    PIN_AddFiniFunction(Fini, 0);

    // Never returns
    PIN_StartProgram();

    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
