/*
 * Copyright (C) 2013-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include "pin.H"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <iostream>
using std::cerr;
using std::endl;

/*
  The tool validates stopping threads API called in internal thread.
*/

// Number of times to stop application threads
#define TIMES 3
// Interval in instructions to repeat PIN_StopApplicationThreads() calls.
#define PAUSE_INTERVAL 10000000

volatile bool stopFlag = false;

THREADID intTid;

// Instruction counter per thread
struct tdata
{
    long long unsigned int icount;
} THREAD_DATA;

static TLS_KEY tls_key = INVALID_TLS_KEY;

tdata* get_tls(THREADID threadid)
{
    tdata* data = static_cast< tdata* >(PIN_GetThreadData(tls_key, threadid));
    if (!data)
    {
        cerr << "specified key is invalid or the given thread is not yet registered in the pin thread database" << endl;
        PIN_ExitProcess(1);
    }
    return data;
}

VOID doPause(VOID* arg)
{
    for (int i = 0; i < TIMES; i++)
    {
        while (stopFlag == false)
        {
            PIN_Sleep(10);
        }
        stopFlag = false;

        printf("Threads to be stopped by internal thread %u\n", intTid);
        fflush(stdout);
        if (PIN_StopApplicationThreads(intTid))
        {
            UINT32 nThreads = PIN_GetStoppedThreadCount();

            printf("Threads stopped by internal thread %u : %u\n", intTid, nThreads);
            fflush(stdout);

            for (UINT32 index = 0; index < nThreads; index++)
            {
                THREADID tid        = PIN_GetStoppedThreadId(index);
                tdata* data         = get_tls(tid);
                const CONTEXT* ctxt = PIN_GetStoppedThreadContext(tid);
                printf("  Thread %u, IP = %llx, icount = %llu\n", tid,
                       (long long unsigned int)PIN_GetContextReg(ctxt, REG_INST_PTR), data->icount);
            }
            PIN_ResumeApplicationThreads(intTid);
            printf("Threads resumed by internal thread %u\n", intTid);
            fflush(stdout);
        }
    }
    return;
}

VOID iCount(THREADID threadid)
{
    tdata* data = get_tls(threadid);
    if ((++data->icount % PAUSE_INTERVAL) == 0)
    {
        stopFlag = true;
    }
}

VOID insCallback(INS ins, void* v) { INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(iCount), IARG_THREAD_ID, IARG_END); }

VOID ThreadStart(THREADID threadid, CONTEXT* ctxt, INT32 flags, VOID* v)
{
    tdata* data  = new tdata;
    data->icount = 0;
    if (PIN_SetThreadData(tls_key, data, threadid) == FALSE)
    {
        cerr << "PIN_SetThreadData failed" << endl;
        PIN_ExitProcess(1);
    }
}

VOID ThreadFini(THREADID threadid, const CONTEXT* ctxt, INT32 code, VOID* v)
{
    tdata* data = get_tls(threadid);
    delete data;
}

int main(int argc, char** argv)
{
    PIN_Init(argc, argv);

    INS_AddInstrumentFunction(insCallback, NULL);

    intTid = PIN_SpawnInternalThread(doPause, NULL, 0, NULL);
    ASSERT(intTid != INVALID_THREADID, "Fail to spawn internal thread");

    tls_key = PIN_CreateThreadDataKey(NULL);
    if (tls_key == INVALID_TLS_KEY)
    {
        cerr << "number of already allocated keys reached the MAX_CLIENT_TLS_KEYS limit" << endl;
        PIN_ExitProcess(1);
    }

    PIN_AddThreadStartFunction(ThreadStart, NULL);
    PIN_AddThreadFiniFunction(ThreadFini, NULL);

    PIN_StartProgram();
    return 1;
}
