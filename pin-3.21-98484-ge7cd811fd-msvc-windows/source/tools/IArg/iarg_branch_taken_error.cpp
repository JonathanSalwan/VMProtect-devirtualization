/*
 * Copyright (C) 2018-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include "pin.H"
#include <iostream>
using std::cerr;
using std::cout;
using std::endl;

/*
 *************************************************************************
 **                   ###    IMPORTANT NOTE     ###                     **
 **  In the future, if support of instrumenting RTM conditional branch  **
 **  instructions (e.g. XBEGIN, XEND) with IARG_BRANCH_TAKEN will be    **
 **  added to Pin, this test should be removed.                         **
 **  -> View mantis 4504 for more info.                                 **
 *************************************************************************
 */

/*
 *                        #  TOOL DESCRIPTION  #
 * This Pin tool presents wrong usage of instrumenting with IARG_BRANCH_TAKEN:
 * When it is applied on applications that have RTM instructions, it will cause an error.
 */

/* Global Variables */
/* ===================================================================== */
int taken_count = 0;

/* Analysis routines                                                     */
/* ===================================================================== */
VOID analysis(bool is_taken)
{
    if (is_taken)
    {
        taken_count++;
    }
}

/* Instrumentation routines                                              */
/* ===================================================================== */
VOID Instruction(INS ins, VOID* v)
{
    if ((INS_Opcode(ins) == XED_ICLASS_XBEGIN) || (INS_Opcode(ins) == XED_ICLASS_XEND))
    {
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)analysis, IARG_BRANCH_TAKEN, IARG_END);
    }
}

VOID Fini(INT32 code, VOID* v) { cout << "end of run: taken = " << taken_count << endl; }

/* =====================================================================
 * Called upon bad command line argument
 * ===================================================================== */
INT32 Usage()
{
    cerr << "This Pin tool presents wrong usage of instrumenting with IARG_BRANCH_TAKEN" << endl;
    cerr << "When it is applied on applications that have RTM instructions, it will cause an error" << endl;
    cerr << KNOB_BASE::StringKnobSummary();
    cerr << endl;
    return -1;
}

/* Main                                                                  */
/* ===================================================================== */
int main(int argc, char* argv[])
{
    if (PIN_Init(argc, argv))
    {
        return Usage();
    };
    PIN_AddFiniFunction(Fini, 0);
    INS_AddInstrumentFunction(Instruction, 0);
    PIN_StartProgram(); // Never returns
    return 0;
}
