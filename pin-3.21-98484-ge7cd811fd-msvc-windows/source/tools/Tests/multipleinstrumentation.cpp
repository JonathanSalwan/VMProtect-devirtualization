/*
 * Copyright (C) 2007-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 *  This is a test for correct functionality of multiple instrumentations at
 *  a single instrumentation point.
 */

#include <stdio.h>
#include <stdlib.h>
#include "pin.H"

FILE* trace;

const INT32 N = 100000;
const INT32 M = 50000;
const INT32 K = 1000;

static UINT32 syscallCount = 0;

INT32 icount = K;

ADDRINT CountDown()
{
    --icount;
    return (icount == 0);
}

// The IP of the current instruction will be printed and
// the icount will be reset to a random number between N and N+M.
VOID PrintIp(VOID* ip)
{
    // Prepare for next period
    icount = N + rand() % M; // random number from N to N+M
}

// Print the return value of the system call
VOID SysAfter(ADDRINT ip) { syscallCount++; }

// Pin calls this function every time a new instruction is encountered
VOID Instruction(INS ins, VOID* v)
{
    if (INS_IsSyscall(ins) && INS_IsValidForIpointAfter(ins))
    {
        // CountDown() is called for every instruction executed
        INS_InsertIfCall(ins, IPOINT_AFTER, (AFUNPTR)CountDown, IARG_END);

        // PrintIp() is called only when the last CountDown() returns a non-zero value.
        INS_InsertThenCall(ins, IPOINT_AFTER, (AFUNPTR)PrintIp, IARG_INST_PTR, IARG_END);
        // return value only available after
        INS_InsertCall(ins, IPOINT_AFTER, AFUNPTR(SysAfter), IARG_INST_PTR, IARG_END);
    }
}

// This function is called when the application exits
VOID Fini(INT32 code, VOID* v) { ASSERTX(syscallCount > 0); }

// argc, argv are the entire command line, including pin -t <toolname> -- ...
int main(int argc, char* argv[])
{
    // Initialize pin
    PIN_Init(argc, argv);

    // Register Instruction to be called to instrument instructions
    INS_AddInstrumentFunction(Instruction, 0);

    // Register Fini to be called when the application exits
    PIN_AddFiniFunction(Fini, 0);

    // Start the program, never returns
    PIN_StartProgram();

    return 0;
}
