/*
 * Copyright (C) 2004-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <iostream>
#include <fstream>
#include "pin.H"
using std::cerr;
using std::cout;
using std::endl;
/*
 Requests an IARG_CONST_CONTEXT on each instruction
*/
// The running count of instructions is kept here
static UINT64 icount = 0;

// This function is called before every instruction is executed
VOID ConstContextAnalysisRoutine(ADDRINT ip, CONTEXT* ctxt, ADDRINT addr)
{
    icount++;
    if ((icount % 10000000) == 0)
    {
        cout << "ConstContextAnalysisRoutine called ConstContextAnalysisRoutine " << icount << " times" << endl;
    }
}

// Pin calls this function every time a new instruction is encountered
VOID Instruction(INS ins, VOID* v)
{
    // Insert a call to docount before every instruction, no arguments are passed
    INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)ConstContextAnalysisRoutine, IARG_INST_PTR, IARG_CONST_CONTEXT, IARG_ADDRINT,
                   INS_Address(ins), IARG_END);
}

// This function is called when the application exits
VOID Fini(INT32 code, VOID* v)
{
    cout << "ConstContextAnalysisRoutine called ConstContextAnalysisRoutine " << icount << " times" << endl;
}

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage()
{
    cerr << "This tool counts the number of dynamic instructions executed" << endl;
    cerr << endl << KNOB_BASE::StringKnobSummary() << endl;
    return -1;
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */
/*   argc, argv are the entire command line: pin -t <toolname> -- ...    */
/* ===================================================================== */

int main(int argc, char* argv[])
{
    // Initialize pin
    if (PIN_Init(argc, argv)) return Usage();

    // Register Instruction to be called to instrument instructions
    INS_AddInstrumentFunction(Instruction, 0);

    // Register Fini to be called when the application exits
    PIN_AddFiniFunction(Fini, 0);

    // Start the program, never returns
    PIN_StartProgram();

    return 0;
}
