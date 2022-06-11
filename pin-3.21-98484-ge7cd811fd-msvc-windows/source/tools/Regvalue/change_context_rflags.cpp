/*
 * Copyright (C) 2016-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <iostream>
#include <fstream>
#include <cstring>
#include <cassert>
#include "pin.H"
using std::cerr;
using std::endl;
using std::flush;
using std::string;

typedef enum
{
    ERROR_SHOULD_NOT_RETURN    = 1,
    ERROR_CONTEXT_WAS_NOT_SET  = 2,
    ERROR_FLAG_WAS_NOT_CHANGED = 3,
    ERROR_ILLEGAL_CONTEXT      = 4
} ERROR_CODES;

/////////////////////
// GLOBAL VARIABLES
/////////////////////

// A knob for defining which context to test. One of:
// 1. default   - regular CONTEXT passed to the analysis routine using IARG_CONTEXT.
// 2. partial   - partial CONTEXT passed to the analysis routine using IARG_PARTIAL_CONTEXT.
KNOB< string > KnobTestContext(KNOB_MODE_WRITEONCE, "pintool", "testcontext", "default",
                               "specify which context to test. One of default|partial.");

// A knob for defining the output file name
KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "", "specify output file name");

// A knob to specify the bit position of the flag to manipulate
KNOB< INT32 > KnobChangeFlags(KNOB_MODE_WRITEONCE, "pintool", "cf", "-1", "specify which flag to set. Default no change.");

#define D_FLAG 0x400 // bit 10 set

// ofstream object for handling the output
std::ostream* OutFile = &cerr;

ADDRINT rflagsBefore;
ADDRINT rflagsAfter;
REGSET regsIn;
REGSET regsOut;
BOOL firstBefore = TRUE;
BOOL doExecuteAt = FALSE;
ADDRINT changedFlag;
THREADID myThread = INVALID_THREADID; // we care only about main thread

/////////////////////
// ANALYSIS FUNCTIONS
/////////////////////

static void GetAndSetRflagsBefore(CONTEXT* ctxt, ADDRINT addr)
{
    BOOL emulateFlagsChange = FALSE;
    // we care only about one/main thread
    if (firstBefore && PIN_ThreadId() == myThread) // to ensure that next run with changed context run "as is"
    {
        rflagsBefore = PIN_GetContextReg(ctxt, REG_GFLAGS);
        firstBefore  = FALSE;
        if (doExecuteAt && (rflagsBefore & D_FLAG)) // in such case we enter with set DFlag so we  dont want to do anything
        {
            rflagsAfter        = rflagsBefore;
            emulateFlagsChange = TRUE;
        }
        else if (rflagsBefore & changedFlag)
        {
            rflagsAfter = rflagsBefore & ~changedFlag;
        }
        else
        {
            rflagsAfter = rflagsBefore | changedFlag;
        }
        if (changedFlag)
        {
            PIN_SetContextReg(ctxt, REG_GFLAGS, rflagsAfter);
            ADDRINT fVal = PIN_GetContextReg(ctxt, REG_GFLAGS);
            if (fVal != rflagsAfter)
            {
                *OutFile << "FLAGS values " << hexstr(fVal) << " did not change to " << hexstr(rflagsAfter) << endl << flush;
                PIN_ExitApplication(ERROR_CONTEXT_WAS_NOT_SET);
            }
        }
        if (doExecuteAt && FALSE == emulateFlagsChange)
        {
            // rerun with changed context
            PIN_ExecuteAt(ctxt);
        }
    }
}

static void GetAndRestoreRflagsAfter(CONTEXT* ctxt, ADDRINT addr)
{
    if (PIN_ThreadId() == myThread)
    {
        ADDRINT val = PIN_GetContextReg(ctxt, REG_GFLAGS);
        firstBefore = TRUE;

        if (val != rflagsAfter)
        {
            *OutFile << "Expecting " << hexstr(rflagsAfter) << " saw " << hexstr(val) << endl << flush;
            PIN_ExitApplication(ERROR_FLAG_WAS_NOT_CHANGED);
        }
        if (changedFlag)
        {
            PIN_SetContextReg(ctxt, REG_GFLAGS, rflagsBefore);
            if (doExecuteAt)
            {
                // need to run ExecutAt to make the restore of flags in affect
                // note that this may result instrumenting this instruction as first instruction
                // if will pass instrumentation checks
                PIN_ExecuteAt(ctxt);
            }
        }
    }
}

/////////////////////
// CALLBACKS
/////////////////////

static BOOL InstructionUsesFlags(INS ins)
{
    if (INS_RegRContain(ins, REG_GFLAGS) || INS_RegRContain(ins, REG_DF_FLAG) || INS_RegRContain(ins, REG_STATUS_FLAGS) ||
        INS_RegWContain(ins, REG_GFLAGS) || INS_RegWContain(ins, REG_DF_FLAG) || INS_RegWContain(ins, REG_STATUS_FLAGS))
    {
        return TRUE;
    }
    return FALSE;
}

inline BOOL IsInstructionsInstrumentable(INS ins, INS nextIns)
{
    if (INS_HasFallThrough(ins) && INS_Invalid() != nextIns && !InstructionUsesFlags(ins) && !INS_IsControlFlow(ins) &&
        !INS_IsSyscall(ins))
    {
        return TRUE;
    }

    return FALSE;
}

static VOID InstructionForCanonicalContext(INS ins, VOID* v)
{
    INS nextIns = INS_Next(ins);
    if (IsInstructionsInstrumentable(ins, nextIns))
    {
        INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(GetAndSetRflagsBefore), IARG_CONTEXT, IARG_INST_PTR, IARG_END);
        INS_InsertCall(nextIns, IPOINT_BEFORE, AFUNPTR(GetAndRestoreRflagsAfter), IARG_CONTEXT, IARG_INST_PTR, IARG_END);
    }
}

static VOID InstructionForPartialContext(INS ins, VOID* v)
{
    INS nextIns = INS_Next(ins);
    if (IsInstructionsInstrumentable(ins, nextIns))
    {
        INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(GetAndSetRflagsBefore), IARG_PARTIAL_CONTEXT, &regsIn, &regsOut, IARG_INST_PTR,
                       IARG_END);
        INS_InsertCall(nextIns, IPOINT_BEFORE, AFUNPTR(GetAndRestoreRflagsAfter), IARG_PARTIAL_CONTEXT, &regsIn, &regsOut,
                       IARG_INST_PTR, IARG_END);
    }
}

static VOID Fini(INT32 code, VOID* v)
{
    if (!KnobOutputFile.Value().empty())
    {
        delete OutFile;
    }
}

/////////////////////
// MAIN FUNCTION
/////////////////////

int main(int argc, char* argv[])
{
    // Initialize Pin
    PIN_InitSymbols();
    PIN_Init(argc, argv);
    REGSET_Clear(regsIn);
    REGSET_Insert(regsIn, REG_GFLAGS);
    REGSET_Clear(regsOut);
    REGSET_Insert(regsOut, REG_GFLAGS);

    changedFlag = (KnobChangeFlags.Value() == -1 ? 0 : 1 << KnobChangeFlags.Value());
    myThread    = PIN_ThreadId();

    // Open the output file
    if (!KnobOutputFile.Value().empty())
    {
        OutFile = new std::ofstream(KnobOutputFile.Value().c_str());
    }
    *OutFile << "Thread " << decstr(myThread) << " flags to change " << hexstr(changedFlag);
    *OutFile << " Knob value " << decstr(KnobChangeFlags.Value()) << endl << flush;

    // Add instrumentation
    if (KnobTestContext.Value() == "default")
    {
        doExecuteAt = TRUE;
        INS_AddInstrumentFunction(InstructionForCanonicalContext, 0);
    }
    else if (KnobTestContext.Value() == "partial")
    {
        INS_AddInstrumentFunction(InstructionForPartialContext, 0);
    }
    else
    {
        *OutFile << "ERROR: Unknown context requested for testing: " << KnobTestContext.Value() << endl;
        PIN_ExitApplication(ERROR_ILLEGAL_CONTEXT); // never returns
    }

    PIN_AddFiniFunction(Fini, 0);

    // Start running the application
    PIN_StartProgram(); // never returns
    return ERROR_SHOULD_NOT_RETURN;
}
