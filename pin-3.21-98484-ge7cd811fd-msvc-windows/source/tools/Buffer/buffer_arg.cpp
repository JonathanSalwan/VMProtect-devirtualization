/*
 * Copyright (C) 2020-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <iostream>
#include <fstream>
#include <cstdlib>
#include "pin.H"
using std::cerr;
using std::endl;
using std::string;

/*
 * This test validates Pin calculates a correct value of IARG_EXECUTING when
 * using buffer. This test will fail when there's a mismatch between the predicted
 * execution and the actual execution.
 */

BUFFER_ID bufId = BUFFER_ID_INVALID;

/*
 * Number of OS pages for the buffer
 */
#define NUM_BUF_PAGES 1

/*
 * Number of loop iterations in the application.
 */
#define ITERATIONS 1000

bool predicationResults[ITERATIONS];
int resultsIndex = 0;

struct PREDICATED
{
    bool executing;
};

/**************************************************************************
 *
 *  Callback Routines
 *
 **************************************************************************/

/*
 * Check if IARG_EXECUTING was correct.
 * BufferFull is called only one time, when application ends because it's size is bigger
 * than number of iterations * size of buffer element.
 */
VOID* BufferFull(BUFFER_ID id, THREADID tid, const CONTEXT* ctxt, VOID* buf, UINT64 numElements, VOID* v)
{
    struct PREDICATED* bufferRecord = (struct PREDICATED*)buf;
    for (UINT64 i = 0; i < numElements; i++, bufferRecord++)
    {
        if (bufferRecord->executing != predicationResults[i])
        {
            cerr << "Error: Prediction didn't match." << endl;
            exit(-1);
        }
    }
    return buf;
}

/*
 * Store bool whether predicated command was executed or not
 */
VOID RecordFunctionReturnValue(ADDRINT value) { predicationResults[resultsIndex++] = value ? false : true; }
/**************************************************************************
 *
 *  Instrumentation routines
 *
 **************************************************************************/
VOID Routine(RTN rtn, VOID* v)
{
    if ((RTN_Name(rtn) == "SimpleCmovTest") || (RTN_Name(rtn) == "_SimpleCmovTest"))
    {
        RTN_Open(rtn);
        RTN_InsertCall(rtn, IPOINT_AFTER, (AFUNPTR)RecordFunctionReturnValue, IARG_FUNCRET_EXITPOINT_VALUE, IARG_END);
        RTN_Close(rtn);
    }
}

VOID Trace(TRACE trace, VOID* v)
{
    RTN rtn = TRACE_Rtn(trace);
    if (RTN_Valid(rtn))
    {
        if ((RTN_Name(rtn) == "SimpleCmovTest") || (RTN_Name(rtn) == "_SimpleCmovTest"))
        {
            for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
            {
                for (INS ins = BBL_InsHead(bbl); INS_Valid(ins); ins = INS_Next(ins))
                {
                    if (INS_IsPredicated(ins))
                    {
                        INS_InsertFillBuffer(ins, IPOINT_BEFORE, bufId, IARG_EXECUTING, offsetof(struct PREDICATED, executing),
                                             IARG_END);
                    }
                }
            }
        }
    }
}

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage()
{
    cerr << "This tool demonstrates the basic use of the buffering API." << endl;
    cerr << endl << KNOB_BASE::StringKnobSummary() << endl;
    return -1;
}

int main(int argc, char* argv[])
{
    PIN_InitSymbols();
    if (PIN_Init(argc, argv))
    {
        return Usage();
    }

    // Initialize the memory reference buffer;
    // set up the callback to process the buffer.
    bufId = PIN_DefineTraceBuffer(sizeof(struct PREDICATED), NUM_BUF_PAGES, BufferFull, 0);

    if (bufId == BUFFER_ID_INVALID)
    {
        cerr << "Error: could not allocate initial buffer" << endl;
        return 1;
    }

    RTN_AddInstrumentFunction(Routine, 0);
    TRACE_AddInstrumentFunction(Trace, 0);

    // Start the program, never returns
    PIN_StartProgram();

    return 0;
}
