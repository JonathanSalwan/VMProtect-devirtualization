/*
 * Copyright (C) 2021-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// This test checks the correctness of IARG_FIRST_REP_ITERATION when instrumenting REP instructions.

#include <iostream>
#include <fstream>
#include "pin.H"
using std::cerr;
using std::cout;
using std::endl;

bool first_rep_flag          = true;
bool rep_analysis_rtn_called = false;

// Pin calls this function every time a new instruction is encountered
VOID rep_analysis_rtn(BOOL is_first_iteration, ADDRINT count, THREADID threadid)
{
    rep_analysis_rtn_called = true;

    if (threadid != 0)
    {
        // The application is single-threaded, and it's thread ID is 0. The operating system
        // on Windows 32-bit creates other threads that does not executes any application
        // code but can change the test behavior, so we need to ignore them.
        return;
    }

    if (count == 0)
    {
        ASSERTX(is_first_iteration == false);
        return;
    }

    if (is_first_iteration)
    {
        ASSERTX(first_rep_flag);

        // if count == 1 this is the first and last iteration - so we leave flag untouched
        if (count > 1)
        {
            // otherwise, updating first iteration flag for next iterations
            first_rep_flag = false;
        }
    }
    else
    {
        // validate that first rep flag is turned off as well
        ASSERTX(is_first_iteration == first_rep_flag);

        if (count == 1)
        {
            // if this is the last iteration of the rep operation - reset flag
            first_rep_flag = true;
        }
    }
}

VOID Ins(INS ins, VOID* v)
{
    if (INS_HasRealRep(ins))
    {
        // we are checking only REP prefixes (not REPE/REPZ/REPNE etc.) which rely on ECX value
        std::string str = INS_Mnemonic(ins);
        if (str.find(std::string("REP_")) == 0)
        {
            INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)rep_analysis_rtn, IARG_FIRST_REP_ITERATION, IARG_REG_VALUE,
                           INS_RepCountRegister(ins), IARG_THREAD_ID, IARG_END);
        }
    }
}

VOID Fini(INT32 code, VOID* v)
{
    // verify that analysis routine has been called at least
    // once with IARG_FIRST_REP_ITERATION equal to TRUE
    ASSERT(rep_analysis_rtn_called, "No REP instruction was found in the application.\n");
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

int main(int argc, char* argv[])
{
    // Initialize pin
    PIN_Init(argc, argv);

    // Register instruction to be called to instrument instructions
    INS_AddInstrumentFunction(Ins, 0);

    // Register Fini to be called when the application exits
    PIN_AddFiniFunction(Fini, 0);

    // start the program, never returns
    PIN_StartProgram();

    return 0;
}
