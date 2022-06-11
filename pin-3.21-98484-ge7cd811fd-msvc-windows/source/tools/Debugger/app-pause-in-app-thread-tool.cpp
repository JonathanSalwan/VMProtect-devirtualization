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
  The tool validates stopping threads API called in application thread.
*/

// Interval in instructions between PIN_StopApplicationThreads() calls.
#define PAUSE_INTERVAL 10000000

// Instruction data counters per thread
struct tdata
{
    UINT64 icount;
    UINT64 stopPoint;
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

VOID doPause(THREADID threadid)
{
    tdata* data = get_tls(threadid);

    ++data->icount;

    if (data->icount == data->stopPoint)
    {
        printf("Threads to be stopped by application thread %u\n", threadid);
        fflush(stdout);
        // Pause threads in each LOOPS/2 instructions
        if (PIN_StopApplicationThreads(threadid))
        {
            printf("Threads stopped by application thread %u\n", threadid);
            fflush(stdout);

            UINT32 nThreads = PIN_GetStoppedThreadCount();
            for (UINT32 i = 0; i < nThreads; i++)
            {
                THREADID tid        = PIN_GetStoppedThreadId(i);
                const CONTEXT* ctxt = PIN_GetStoppedThreadContext(tid);
                printf("  Thread %u, IP = %llx\n", tid, (long long unsigned int)PIN_GetContextReg(ctxt, REG_INST_PTR));
            }
            PIN_ResumeApplicationThreads(threadid);
            printf("Threads resumed by application thread %u\n", threadid);
            fflush(stdout);

            // Reset stop point, do not try to stop in this thread any more.
            data->stopPoint = 0;
        }
        else
        {
            // Probably collision with other thread calling PIN_StopApplicationThreads()
            // Update stop point.
            data->stopPoint += PAUSE_INTERVAL;
        }
    }
    return;
}

VOID insCallback(INS ins, void* v) { INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(doPause), IARG_THREAD_ID, IARG_END); }

VOID ThreadStart(THREADID threadid, CONTEXT* ctxt, INT32 flags, VOID* v)
{
    tdata* data = new tdata;
    // Thread ID 0 reserved for main thread.
    // This thread will not call PIN_StopApplicationThreads()
    data->icount    = 0;
    data->stopPoint = PAUSE_INTERVAL * threadid;
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

    tls_key = PIN_CreateThreadDataKey(NULL);
    if (tls_key == INVALID_TLS_KEY)
    {
        cerr << "number of already allocated keys reached the MAX_CLIENT_TLS_KEYS limit" << endl;
        PIN_ExitProcess(1);
    }

    INS_AddInstrumentFunction(insCallback, NULL);
    PIN_AddThreadStartFunction(ThreadStart, NULL);
    PIN_AddThreadFiniFunction(ThreadFini, NULL);

    PIN_StartProgram();
    return 1;
}
