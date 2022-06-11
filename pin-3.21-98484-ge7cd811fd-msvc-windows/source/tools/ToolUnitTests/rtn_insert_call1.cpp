/*
 * Copyright (C) 2011-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// This tool test RTN_InsHead RTN_InsHeadOnly and RTN_InsertCall (before)

#include <stdio.h>
#include <stdlib.h>
#include "pin.H"

UINT32 numThreads          = 0;
const UINT32 MaxNumThreads = 32;

struct THREAD_DATA
{
    ADDRINT curRtnAddr;
    int numAtRtn;
    int numBeforeInsHeadOnly;
    int numAfterInsHeadOnly;
    int numBeforeInsHead;
    int numAfterInsHead;
    THREAD_DATA()
        : curRtnAddr(0), numAtRtn(0), numBeforeInsHeadOnly(0), numAfterInsHeadOnly(0), numBeforeInsHead(0), numAfterInsHead(0)
    {}
};

// key for accessing TLS storage in the threads. initialized once in main()
static TLS_KEY tls_key;

// function to access thread-specific data
THREAD_DATA* get_tls(THREADID threadid)
{
    THREAD_DATA* tdata = static_cast< THREAD_DATA* >(PIN_GetThreadData(tls_key, threadid));
    return tdata;
}

int numRtnsFoundInImageCallback          = 0;
int numRtnsInstrumentedFromImageCallback = 0;
int numRtnsFoundInRtnCallback            = 0;

VOID ThreadStart(THREADID threadid, CONTEXT* ctxt, INT32 flags, VOID* v)
{
    ASSERT(numThreads == threadid, "Unexpected threadid\n");
    numThreads++;
    ASSERT(numThreads <= MaxNumThreads, "Maximum number of threads exceeded\n");
    THREAD_DATA* tdata = new THREAD_DATA();
    PIN_SetThreadData(tls_key, tdata, threadid);
}

VOID AtRtn(ADDRINT rtnAddr, THREADID tid)
{
    THREAD_DATA* tdata = get_tls(tid);
    if (tdata->curRtnAddr != 0)
    {
        printf("**** expected rtnAddr to be 0\n");
        PIN_ExitProcess(1);
    }
    tdata->curRtnAddr = rtnAddr;
    tdata->numAtRtn++;
}

VOID BeforeInsHeadOnly(ADDRINT insAddr, THREADID tid)
{
    THREAD_DATA* tdata = get_tls(tid);
    if (tdata->curRtnAddr == 0)
    {
        printf("**** BeforeInsHeadOnly expected curRtnAddr to be non-0\n");
        PIN_ExitProcess(1);
    }
    if (tdata->curRtnAddr != insAddr)
    {
        printf("**** BeforeInsHeadOnly got unexpected insAddr\n");
        PIN_ExitProcess(1);
    }
    tdata->numBeforeInsHeadOnly++;
}

VOID AfterInsHeadOnly(ADDRINT insAddr, THREADID tid)
{
    THREAD_DATA* tdata = get_tls(tid);
    if (tdata->curRtnAddr == 0)
    {
        printf("**** AfterInsHeadOnly expected curRtnAddr to be non-0\n");
        PIN_ExitProcess(1);
    }
    if (tdata->curRtnAddr != insAddr)
    {
        printf("**** AfterInsHeadOnly got unexpected insAddr\n");
        PIN_ExitProcess(1);
    }
    tdata->numAfterInsHeadOnly++;
}

VOID BeforeInsHead(ADDRINT insAddr, THREADID tid)
{
    THREAD_DATA* tdata = get_tls(tid);
    if (tdata->curRtnAddr == 0)
    {
        printf("**** BeforeInsHead expected curRtnAddr to be non-0\n");
        PIN_ExitProcess(1);
    }
    if (tdata->curRtnAddr != insAddr)
    {
        printf("**** BeforeInsHead got unexpected insAddr\n");
        PIN_ExitProcess(1);
    }
    tdata->numBeforeInsHead++;
}

VOID AfterInsHead(ADDRINT insAddr, THREADID tid)
{
    THREAD_DATA* tdata = get_tls(tid);
    if (tdata->curRtnAddr == 0)
    {
        printf("**** AfterInsHead expected curRtnAddr to be non-0\n");
        PIN_ExitProcess(1);
    }
    if (tdata->curRtnAddr != insAddr)
    {
        printf("**** AfterInsHead got unexpected insAddr\n");
        PIN_ExitProcess(1);
    }
    tdata->numAfterInsHead++;
    tdata->curRtnAddr = 0;
}

VOID Image(IMG img, void* v)
{
    for (SEC sec = IMG_SecHead(img); SEC_Valid(sec); sec = SEC_Next(sec))
    {
        for (RTN rtn = SEC_RtnHead(sec); RTN_Valid(rtn); rtn = RTN_Next(rtn))
        {
            RTN_Open(rtn);
            numRtnsFoundInImageCallback++;
            INS ins = RTN_InsHeadOnly(rtn);
            if (INS_Invalid() == ins)
            { // no instruction found - assert that RTN_InsHead(rtn) also doesn't find any INS
                ASSERTX(INS_Invalid() == RTN_InsHead(rtn));
                RTN_Close(rtn);
                continue;
            }

            if (INS_IsValidForIpointAfter(ins))
            {
                ADDRINT insAddress = INS_Address(ins);
                numRtnsInstrumentedFromImageCallback++;
                RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(AtRtn), IARG_ADDRINT, RTN_Address(rtn), IARG_THREAD_ID, IARG_END);

                INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(BeforeInsHeadOnly), IARG_ADDRINT, INS_Address(ins), IARG_THREAD_ID,
                               IARG_END);
                INS_InsertCall(ins, IPOINT_AFTER, AFUNPTR(AfterInsHeadOnly), IARG_ADDRINT, INS_Address(ins), IARG_THREAD_ID,
                               IARG_END);
                ins = RTN_InsHead(rtn);
                ASSERTX(INS_Invalid() != ins);
                ASSERTX(INS_Address(ins) == insAddress);
                INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(BeforeInsHead), IARG_ADDRINT, insAddress, IARG_THREAD_ID, IARG_END);
                INS_InsertCall(ins, IPOINT_AFTER, AFUNPTR(AfterInsHead), IARG_ADDRINT, insAddress, IARG_THREAD_ID, IARG_END);
            }
            RTN_Close(rtn);
        }
    }
}

VOID Fini(INT32 code, VOID* v)
{
    if (numRtnsInstrumentedFromImageCallback == 0)
    {
        printf("***** expected numRtnsInstrumentedFromImageCallback to be > 0\n");
        PIN_ExitProcess(1);
    }
    if (numRtnsFoundInImageCallback == 0)
    {
        printf("***** expected numRtnsFoundInImageCallback to be > 0\n");
        PIN_ExitProcess(1);
    }
    if (numRtnsFoundInImageCallback != numRtnsFoundInRtnCallback)
    {
        printf("***** expected numRtnsFoundInImageCallback == numRtnsFoundInRtnCallback\n");
        PIN_ExitProcess(1);
    }
}

// This function is called when the thread exits
VOID ThreadFini(THREADID threadIndex, const CONTEXT* ctxt, INT32 code, VOID* v)
{
    THREAD_DATA* tdata = get_tls(threadIndex);
    if (tdata->numAtRtn == 0)
    {
        printf("***** expected numAtRtn to be > 0\n");
        PIN_ExitProcess(1);
    }
    if (tdata->numAtRtn != tdata->numBeforeInsHeadOnly)
    {
        printf("***** expected numAtRtn == numBeforeInsHeadOnly\n");
        PIN_ExitProcess(1);
    }
    if (tdata->numAtRtn != tdata->numAfterInsHeadOnly)
    {
        printf("***** expected numAtRtn == numAfterInsHeadOnly\n");
        PIN_ExitProcess(1);
    }
    if (tdata->numAtRtn != tdata->numBeforeInsHead)
    {
        printf("***** expected numAtRtn == numBeforeInsHead\n");
        PIN_ExitProcess(1);
    }
    if (tdata->numAtRtn != tdata->numAfterInsHead)
    {
        printf("***** expected numAtRtn == numAfterInsHead\n");
        PIN_ExitProcess(1);
    }
}

VOID Rtn(RTN rtn, VOID*) { numRtnsFoundInRtnCallback++; }

int main(int argc, char** argv)
{
    PIN_Init(argc, argv);
    PIN_InitSymbols();

    // Obtain  a key for TLS storage.
    tls_key = PIN_CreateThreadDataKey(NULL);
    if (-1 == tls_key)
    {
        printf("number of already allocated keys reached the MAX_CLIENT_TLS_KEYS limit\n");
        PIN_ExitProcess(1);
    }

    PIN_AddThreadStartFunction(ThreadStart, NULL);
    // Register tFini to be called when thread exits.
    PIN_AddThreadFiniFunction(ThreadFini, NULL);
    IMG_AddInstrumentFunction(Image, NULL);
    RTN_AddInstrumentFunction(Rtn, NULL);
    PIN_AddFiniFunction(Fini, NULL);

    PIN_StartProgram();
    return 1;
}
