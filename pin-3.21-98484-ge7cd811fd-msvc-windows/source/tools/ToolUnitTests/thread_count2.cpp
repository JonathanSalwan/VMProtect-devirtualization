/*
 * Copyright (C) 2006-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include "pin.H"

using std::string;

KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "thread_count2.out", "Output file");

/* ===================================================================== */
/* Global Variables */
/* ===================================================================== */

FILE* out;
PIN_LOCK pinLock;
INT32 threadCreated = 0;
INT32 threadEnded   = 0;

/* ===================================================================== */

VOID ThreadStart(THREADID threadid, CONTEXT* ctxt, INT32 flags, VOID* v)
{
    PIN_GetLock(&pinLock, PIN_GetTid());
    threadCreated++;
    PIN_ReleaseLock(&pinLock);
}

/* ===================================================================== */

VOID ThreadFini(THREADID threadid, const CONTEXT* ctxt, INT32 code, VOID* v)
{
    PIN_GetLock(&pinLock, PIN_GetTid());
    threadEnded++;
    PIN_ReleaseLock(&pinLock);
}

/* ===================================================================== */

VOID Fini(INT32 code, VOID* v)
{
    fprintf(out, "Number of threads created  - %d\n", (int)threadCreated);
    fprintf(out, "Number of threads terminated  - %d\n", (int)threadEnded);
    fclose(out);
}

/* ===================================================================== */

VOID UseToolStack()
{
    UINT32 local = 0;
    local++;
}

/* ===================================================================== */

VOID Instruction(INS ins, VOID* v) { INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)UseToolStack, IARG_END); }

/* ===================================================================== */

int main(INT32 argc, CHAR** argv)
{
    PIN_Init(argc, argv);
    PIN_InitLock(&pinLock);

    out = fopen(KnobOutputFile.Value().c_str(), "w");

    INS_AddInstrumentFunction(Instruction, 0);

    PIN_AddThreadStartFunction(ThreadStart, 0);
    PIN_AddThreadFiniFunction(ThreadFini, 0);
    PIN_AddFiniFunction(Fini, 0);

    // Never returns
    PIN_StartProgram();

    return 0;
}
