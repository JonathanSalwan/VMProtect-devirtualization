/*
 * Copyright (C) 2005-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 *  This file contains a Pintool for sampling the IPs of instruction executed.
 *  It serves as an example of a more efficient way to write analysis routines
 *  that include conditional tests.
 *  Currently, it works on IA-32 and Intel(R) 64 architectures.
 */

#include <stdio.h>
#include <stdlib.h>
#include "pin.H"

FILE* trace;

const INT32 N = 100000;
const INT32 M = 50000;

INT32 icount = N;

/*
 *  IP-sampling could be done in a single analysis routine like:
 *
 *        VOID IpSample(VOID *ip)
 *        {
 *            --icount;
 *            if (icount == 0)
 *            {
 *               fprintf(trace, "%p\n", ip);
 *               icount = N + rand() % M;
 *            }
 *        }
 *
 *  However, we break IpSample() into two analysis routines,
 *  CountDown() and PrintIp(), to facilitate Pin inlining CountDown()
 *  (which is the much more frequently executed one than PrintIp()).
 */

ADDRINT CountDown()
{
    --icount;
    return (icount == 0);
}

// The IP of the current instruction will be printed and
// the icount will be reset to a random number between N and N+M.
VOID PrintIp(VOID* ip)
{
    fprintf(trace, "%p\n", ip);

    // Prepare for next period
    icount = N + rand() % M; // random number from N to N+M
}

// Pin calls this function every time a new instruction is encountered
VOID Instruction(INS ins, VOID* v)
{
    // CountDown() is called for every instruction executed
    INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)CountDown, IARG_END);

    // PrintIp() is called only when the last CountDown() returns a non-zero value.
    INS_InsertThenCall(ins, IPOINT_BEFORE, (AFUNPTR)PrintIp, IARG_INST_PTR, IARG_END);
}

// This function is called when the application exits
VOID Fini(INT32 code, VOID* v)
{
    fprintf(trace, "#eof\n");
    fclose(trace);
}

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage()
{
    PIN_ERROR("This Pintool samples the IPs of instruction executed\n" + KNOB_BASE::StringKnobSummary() + "\n");
    return -1;
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

int main(int argc, char* argv[])
{
    trace = fopen("isampling.out", "w");

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
