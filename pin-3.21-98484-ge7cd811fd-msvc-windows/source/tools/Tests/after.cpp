/*
 * Copyright (C) 2004-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pin.H"

/*
 * Tests:
 *    before, after, and fall through instrumentation
 *    multiple instrumentation for the same instruction
 *    inlined and non-inlined
 *    references to global variables (RIP relative addressing in inlined function)
 *
 */

int beforeVal = 0;
int afterVal  = 0;
int takenVal  = 0;

void beforeCall() { beforeVal = 1; }

void beforeCheck()
{
    if (beforeVal != 1)
    {
        fprintf(stderr, "Before failed\n");
        exit(1);
    }
}

void afterCall() { afterVal = 2; }

void afterCheck()
{
    if (afterVal != 2)
    {
        fprintf(stderr, "After failed\n");
        exit(1);
    }
}

void takenCall() { takenVal = 3; }

void takenCheck()
{
    if (takenVal != 3)
    {
        fprintf(stderr, "Taken failed\n");
        exit(1);
    }
}

const INT32 phigh    = 100;
const INT32 pdefault = 200;
const INT32 plow     = 300;

UINT32 pri[10];
UINT32 pri_count = 0;

VOID RecordPriority(UINT32 val)
{
    pri[pri_count] = val;
    pri_count++;
}

UINT32 pri_val[] = {phigh, phigh + 1, pdefault, pdefault + 1, pdefault + 2, pdefault + 3, plow, plow + 1};

VOID printAll(INT32 length)
{
    for (INT32 i = 0; i < length; i++)
    {
        fprintf(stderr, "%d\n", pri[i]);
    }
}

VOID CheckPriority()
{
    UINT32 length = sizeof(pri_val) / sizeof(pri_val[0]);

    if (pri_count != length)
    {
        fprintf(stderr, "Bad count expected %d, got %d\n", length, pri_count);
        printAll(length);
        exit(1);
    }

    if (memcmp(pri_val, pri, sizeof(pri_val)) != 0)
    {
        fprintf(stderr, "Bad pri\n");
        printAll(length);
        exit(1);
    }

    pri_count = 0;
}

VOID Priority(IPOINT ipoint, INS ins)
{
    INS_InsertCall(ins, ipoint, AFUNPTR(RecordPriority), IARG_CALL_ORDER, CALL_ORDER_LAST, IARG_UINT32, plow, IARG_END);
    INS_InsertCall(ins, ipoint, AFUNPTR(RecordPriority), IARG_CALL_ORDER, CALL_ORDER_LAST, IARG_UINT32, plow + 1, IARG_END);

    INS_InsertCall(ins, ipoint, AFUNPTR(RecordPriority), IARG_CALL_ORDER, CALL_ORDER_DEFAULT, IARG_UINT32, pdefault, IARG_END);
    INS_InsertCall(ins, ipoint, AFUNPTR(RecordPriority), IARG_CALL_ORDER, CALL_ORDER_DEFAULT, IARG_UINT32, pdefault + 1,
                   IARG_END);

    INS_InsertCall(ins, ipoint, AFUNPTR(RecordPriority), IARG_UINT32, pdefault + 2, IARG_END);
    INS_InsertCall(ins, ipoint, AFUNPTR(RecordPriority), IARG_UINT32, pdefault + 3, IARG_END);

    INS_InsertCall(ins, ipoint, AFUNPTR(RecordPriority), IARG_CALL_ORDER, CALL_ORDER_FIRST, IARG_UINT32, phigh, IARG_END);
    INS_InsertCall(ins, ipoint, AFUNPTR(RecordPriority), IARG_CALL_ORDER, CALL_ORDER_FIRST, IARG_UINT32, phigh + 1, IARG_END);

    INS_InsertCall(ins, ipoint, AFUNPTR(CheckPriority), IARG_CALL_ORDER, CALL_ORDER_LAST, IARG_END);
}

VOID Ins(INS ins, VOID* v)
{
    static bool before = false, after = false, taken = false;

    if (!before)
    {
        INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(beforeCall), IARG_END);
        INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(beforeCheck), IARG_END);

        Priority(IPOINT_BEFORE, ins);

        before = true;
    }

    if (!after && INS_IsValidForIpointAfter(ins))
    {
        INS_InsertCall(ins, IPOINT_AFTER, AFUNPTR(afterCall), IARG_END);
        INS_InsertCall(ins, IPOINT_AFTER, AFUNPTR(afterCheck), IARG_END);

        Priority(IPOINT_AFTER, ins);

        after = true;
    }

    if (!taken && INS_IsValidForIpointTakenBranch(ins))
    {
        INS_InsertCall(ins, IPOINT_TAKEN_BRANCH, AFUNPTR(takenCall), IARG_END);
        INS_InsertCall(ins, IPOINT_TAKEN_BRANCH, AFUNPTR(takenCheck), IARG_END);

        Priority(IPOINT_TAKEN_BRANCH, ins);

        taken = true;
    }
}

int main(INT32 argc, CHAR** argv)
{
    PIN_Init(argc, argv);

    INS_AddInstrumentFunction(Ins, 0);

    // Never returns
    PIN_StartProgram();

    return 0;
}
