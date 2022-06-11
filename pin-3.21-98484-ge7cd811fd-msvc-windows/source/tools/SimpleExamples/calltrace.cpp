/*
 * Copyright (C) 2004-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 *  This file contains an ISA-portable PIN tool for tracing instructions
 */

#include "pin.H"
#include <iostream>
#include <fstream>
using std::cerr;
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

KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "calltrace.out", "specify trace file name");
KNOB< BOOL > KnobPrintArgs(KNOB_MODE_WRITEONCE, "pintool", "a", "0", "print call arguments ");
//KNOB<BOOL>   KnobPrintArgs(KNOB_MODE_WRITEONCE, "pintool", "i", "0", "mark indirect calls ");

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage()
{
    cerr << "This tool produces a call trace." << endl << endl;
    cerr << KNOB_BASE::StringKnobSummary() << endl;
    return -1;
}

string invalid = "invalid_rtn";

/* ===================================================================== */
const string* Target2String(ADDRINT target)
{
    string name = RTN_FindNameByAddress(target);
    if (name == "")
        return &invalid;
    else
        return new string(name);
}

/* ===================================================================== */

VOID do_call_args(const string* s, ADDRINT arg0) { TraceFile << *s << "(" << arg0 << ",...)" << endl; }

/* ===================================================================== */

VOID do_call_args_indirect(ADDRINT target, BOOL taken, ADDRINT arg0)
{
    if (!taken) return;

    const string* s = Target2String(target);
    do_call_args(s, arg0);

    if (s != &invalid) delete s;
}

/* ===================================================================== */

VOID do_call(const string* s) { TraceFile << *s << endl; }

/* ===================================================================== */

VOID do_call_indirect(ADDRINT target, BOOL taken)
{
    if (!taken) return;

    const string* s = Target2String(target);
    do_call(s);

    if (s != &invalid) delete s;
}

/* ===================================================================== */

VOID Trace(TRACE trace, VOID* v)
{
    const BOOL print_args = KnobPrintArgs.Value();

    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
    {
        INS tail = BBL_InsTail(bbl);

        if (INS_IsCall(tail))
        {
            if (INS_IsDirectControlFlow(tail))
            {
                const ADDRINT target = INS_DirectControlFlowTargetAddress(tail);
                if (print_args)
                {
                    INS_InsertPredicatedCall(tail, IPOINT_BEFORE, AFUNPTR(do_call_args), IARG_PTR, Target2String(target),
                                             IARG_FUNCARG_CALLSITE_VALUE, 0, IARG_END);
                }
                else
                {
                    INS_InsertPredicatedCall(tail, IPOINT_BEFORE, AFUNPTR(do_call), IARG_PTR, Target2String(target), IARG_END);
                }
            }
            else
            {
                if (print_args)
                {
                    INS_InsertCall(tail, IPOINT_BEFORE, AFUNPTR(do_call_args_indirect), IARG_BRANCH_TARGET_ADDR,
                                   IARG_BRANCH_TAKEN, IARG_FUNCARG_CALLSITE_VALUE, 0, IARG_END);
                }
                else
                {
                    INS_InsertCall(tail, IPOINT_BEFORE, AFUNPTR(do_call_indirect), IARG_BRANCH_TARGET_ADDR, IARG_BRANCH_TAKEN,
                                   IARG_END);
                }
            }
        }
        else
        {
            // sometimes code is not in an image
            RTN rtn = TRACE_Rtn(trace);

            // also track stup jumps into share libraries
            if (RTN_Valid(rtn) && !INS_IsDirectControlFlow(tail) && ".plt" == SEC_Name(RTN_Sec(rtn)))
            {
                if (print_args)
                {
                    INS_InsertCall(tail, IPOINT_BEFORE, AFUNPTR(do_call_args_indirect), IARG_BRANCH_TARGET_ADDR,
                                   IARG_BRANCH_TAKEN, IARG_FUNCARG_CALLSITE_VALUE, 0, IARG_END);
                }
                else
                {
                    INS_InsertCall(tail, IPOINT_BEFORE, AFUNPTR(do_call_indirect), IARG_BRANCH_TARGET_ADDR, IARG_BRANCH_TAKEN,
                                   IARG_END);
                }
            }
        }
    }
}

/* ===================================================================== */

VOID Fini(INT32 code, VOID* v)
{
    TraceFile << "# eof" << endl;

    TraceFile.close();
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

int main(int argc, char* argv[])
{
    PIN_InitSymbols();

    if (PIN_Init(argc, argv))
    {
        return Usage();
    }

    TraceFile.open(KnobOutputFile.Value().c_str());

    TraceFile << hex;
    TraceFile.setf(ios::showbase);

    string trace_header = string("#\n"
                                 "# Call Trace Generated By Pin\n"
                                 "#\n");

    TraceFile.write(trace_header.c_str(), trace_header.size());

    TRACE_AddInstrumentFunction(Trace, 0);
    PIN_AddFiniFunction(Fini, 0);

    // Never returns

    PIN_StartProgram();

    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
