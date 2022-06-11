/*
 * Copyright (C) 2007-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 *  This file contains a tool for testing the segmented IARG_MEMORY(READ/WRITE)_EA on PIN Windows
 */

#include <stdio.h>
#include <string.h>
#include <iostream>
#include "pin.H"

#include <map>

using std::cerr;
using std::endl;

#ifdef TARGET_WINDOWS
namespace WIND
{
#include <windows.h>
}

inline ADDRINT InitializeThreadData() { return reinterpret_cast< ADDRINT >(WIND::NtCurrentTeb()); }

#endif

#ifdef TARGET_LINUX
#include <unistd.h>
#include <syscall.h>
#include <errno.h>

#ifdef TARGET_IA32E
#include <asm/prctl.h>
#include <sys/prctl.h>

#endif
#ifndef __NR_set_thread_area
#define __NR_set_thread_area 243
#endif
#ifndef __NR_get_thread_area
#define __NR_get_thread_area 244
#endif
#ifndef SYS_set_thread_area
#define SYS_set_thread_area __NR_set_thread_area
#endif
#ifndef SYS_get_thread_area
#define SYS_get_thread_area __NR_get_thread_area
#endif

typedef struct
{
    unsigned int entry_number;
    unsigned int base_addr;
    unsigned int limit;
    unsigned int seg_32bit : 1;
    unsigned int contents : 2;
    unsigned int read_exec_only : 1;
    unsigned int limit_in_pages : 1;
    unsigned int seg_not_present : 1;
    unsigned int useable : 1;
} UserDesc;

#define TLS_GET_GS_REG()                           \
    (                                              \
        {                                          \
            int __seg;                             \
            __asm("movw %%gs, %w0" : "=q"(__seg)); \
            __seg & 0xffff;                        \
        })

#define TLS_SET_GS_REG(val) __asm("movw %w0, %%gs" ::"q"(val))

#define TLS_GET_FS_REG()                           \
    (                                              \
        {                                          \
            int __seg;                             \
            __asm("movw %%fs, %w0" : "=q"(__seg)); \
            __seg & 0xffff;                        \
        })

#define TLS_SET_FS_REG(val) __asm("movw %w0, %%fs" ::"q"(val))

ADDRINT GetTlsBaseAddress()
{
    ADDRINT baseAddress = 0;
#ifdef TARGET_IA32
    unsigned int gsVal = TLS_GET_GS_REG();
    UserDesc td;
    td.entry_number = gsVal >> 3;
    if (gsVal == 0) return 0;
    int res = syscall(SYS_get_thread_area, &td);
    if (res != 0)
    {
        printf("SYS_get_thread_area failed with error: %s\n", strerror(errno));
        return 0;
    }
    baseAddress = td.base_addr;
#else
    int res = syscall(SYS_arch_prctl, ARCH_GET_FS, &baseAddress);
    if (res != 0)
    {
        printf("SYS_arch_prctl failed with error: %s\n", strerror(errno));
        return 0;
    }
#endif
    return baseAddress;
}

inline ADDRINT InitializeThreadData() { return GetTlsBaseAddress(); }
#endif

FILE* trace = stdout;
#define MAX_THREADS 1024
#define DISPLACEMENT_ONLY_ADDRESSING_READ_TYPE 0
#define BASE_DISPLACEMENT_ADDRESSING_READ_TYPE 1
#define INDEX_DISPLACEMENT_ADDRESSING_READ_TYPE 2
#define BASE_INDEX_DISPLACEMENT_ADDRESSING_READ_TYPE 3
#define DISPLACEMENT_ONLY_ADDRESSING_WRITE_TYPE 4
#define BASE_DISPLACEMENT_ADDRESSING_WRITE_TYPE 5
#define INDEX_DISPLACEMENT_ADDRESSING_WRITE_TYPE 6
#define BASE_INDEX_DISPLACEMENT_ADDRESSING_WRITE_TYPE 7
#define MAX_ADDRESSING_TYPES 8
struct tdata
{
    ADDRINT threadTeb;
    int numEffectiveAddressesAnalyzed[MAX_ADDRESSING_TYPES];
} THREAD_DATA;

int numThreads = 0;
BOOL hadError  = FALSE;

static TLS_KEY tls_key = INVALID_TLS_KEY;

tdata* get_tls(THREADID threadid) { return static_cast< tdata* >(PIN_GetThreadData(tls_key, threadid)); }

const char* GetMemoryAccessTypeString(int j)
{
    switch (j)
    {
        case DISPLACEMENT_ONLY_ADDRESSING_READ_TYPE:
            return ("DISPLACEMENT_ONLY_ADDRESSING_READ_TYPE");
        case BASE_DISPLACEMENT_ADDRESSING_READ_TYPE:
            return ("BASE_DISPLACEMENT_ADDRESSING_READ_TYPE");
        case INDEX_DISPLACEMENT_ADDRESSING_READ_TYPE:
            return ("INDEX_DISPLACEMENT_ADDRESSING_READ_TYPE");
        case BASE_INDEX_DISPLACEMENT_ADDRESSING_READ_TYPE:
            return ("BASE_INDEX_DISPLACEMENT_ADDRESSING_READ_TYPE");
        case DISPLACEMENT_ONLY_ADDRESSING_WRITE_TYPE:
            return ("DISPLACEMENT_ONLY_ADDRESSING_WRITE_TYPE");
        case BASE_DISPLACEMENT_ADDRESSING_WRITE_TYPE:
            return ("BASE_DISPLACEMENT_ADDRESSING_WRITE_TYPE");
        case INDEX_DISPLACEMENT_ADDRESSING_WRITE_TYPE:
            return ("INDEX_DISPLACEMENT_ADDRESSING_WRITE_TYPE");
        case BASE_INDEX_DISPLACEMENT_ADDRESSING_WRITE_TYPE:
            return ("BASE_INDEX_DISPLACEMENT_ADDRESSING_WRITE_TYPE");
        default:
            return ("UNKNOWN_ADDRESSING_TYPE");
    }
}

VOID AnalyzeSegmentedMemAccessDispl(VOID* ip, VOID* addr, UINT32 accessType, UINT32 displacement, THREADID tid)
{
    tdata* data = get_tls(tid);
    if (data != NULL)
    {
        if (!data->threadTeb) data->threadTeb = InitializeThreadData();

        ADDRINT threadTeb = data->threadTeb;
        ADDRINT memoryEA  = reinterpret_cast< ADDRINT >(addr);
        data->numEffectiveAddressesAnalyzed[accessType]++;

        if ((threadTeb + displacement) != memoryEA)
        { // memoryEA is the TEB of the thread + displacement that is in the segmented operand
            fprintf(trace, "TRACED_INST_ERROR %p: R %p (tid %x) (teb %p displacement %x  expectedEA %p)\n", ip, addr, tid,
                    reinterpret_cast< VOID* >(threadTeb), displacement, reinterpret_cast< VOID* >(threadTeb + displacement));
            hadError = TRUE;
        }
    }
}

VOID AnalyzeSegmentedMemAccessBaseIndexDispl(VOID* ip, VOID* addr, UINT32 accessType, ADDRINT baseRegVal, ADDRINT indexRegVal,
                                             UINT32 scale, UINT32 displacement, THREADID tid)
{
    tdata* data = get_tls(tid);
    if (data != NULL)
    {
        if (!data->threadTeb) data->threadTeb = InitializeThreadData();

        ADDRINT threadTeb = data->threadTeb;
        ADDRINT memoryEA  = reinterpret_cast< ADDRINT >(addr);
        data->numEffectiveAddressesAnalyzed[accessType]++;

        if ((threadTeb + displacement + baseRegVal + (indexRegVal * scale)) != memoryEA)
        { // memoryEA is the TEB of the thread + displacement that is in the segmented operand
            fprintf(trace,
                    "TRACED_INST_ERROR %p: R %p (tid %x) (teb %p displacement %x  baseRegVal %x indexRegVal %x scale %d "
                    "expectedEA %p)\n",
                    ip, addr, tid, reinterpret_cast< VOID* >(threadTeb), displacement, baseRegVal, indexRegVal, scale,
                    reinterpret_cast< VOID* >(threadTeb + displacement));
            hadError = TRUE;
        }
    }
}

VOID AnalyzeSegmentedMemAccessBaseDispl(VOID* ip, VOID* addr, UINT32 accessType, ADDRINT baseRegVal, UINT32 displacement,
                                        THREADID tid)
{
    tdata* data = get_tls(tid);
    if (data != NULL)
    {
        if (!data->threadTeb) data->threadTeb = InitializeThreadData();

        ADDRINT threadTeb = data->threadTeb;
        ADDRINT memoryEA  = reinterpret_cast< ADDRINT >(addr);
        data->numEffectiveAddressesAnalyzed[accessType]++;

        if ((threadTeb + displacement + baseRegVal) != memoryEA)
        { // memoryEA is the TEB of the thread + displacement that is in the segmented operand
            fprintf(trace, "TRACED_INST_ERROR %p: R %p (tid %x) (teb %p displacement %x  baseRegVal %x expectedEA %p)\n", ip,
                    addr, tid, reinterpret_cast< VOID* >(threadTeb), displacement, baseRegVal,
                    reinterpret_cast< VOID* >(threadTeb + displacement));
            hadError = TRUE;
        }
    }
}

VOID AnalyzeSegmentedMemAccessIndexDispl(VOID* ip, VOID* addr, UINT32 accessType, ADDRINT indexRegVal, UINT32 scale,
                                         UINT32 displacement, THREADID tid)
{
    tdata* data = get_tls(tid);
    if (data != NULL)
    {
        if (!data->threadTeb) data->threadTeb = InitializeThreadData();

        ADDRINT threadTeb = data->threadTeb;
        ADDRINT memoryEA  = reinterpret_cast< ADDRINT >(addr);
        data->numEffectiveAddressesAnalyzed[accessType]++;

        if ((threadTeb + displacement + (indexRegVal * scale)) != memoryEA)
        { // memoryEA is the TEB of the thread + displacement that is in the segmented operand
            fprintf(trace, "TRACED_INST_ERROR %p: R %p (tid %x) (teb %p displacement %x indexRegVal %x scale %d expectedEA %p)\n",
                    ip, addr, tid, reinterpret_cast< VOID* >(threadTeb), displacement, indexRegVal, scale,
                    reinterpret_cast< VOID* >(threadTeb + displacement));
            hadError = TRUE;
        }
    }
}

#ifndef TARGET_LINUX
#ifdef TARGET_IA32
#define TESTED_SEG_REG REG_SEG_FS
#else
#define TESTED_SEG_REG REG_SEG_GS
#endif
#else
#ifdef TARGET_IA32
#define TESTED_SEG_REG REG_SEG_GS
#else
#define TESTED_SEG_REG REG_SEG_FS
#endif
#endif

#define TESTED_SEG_REG_BASE (TESTED_SEG_REG == REG_SEG_FS ? REG_SEG_FS_BASE : REG_SEG_GS_BASE)

VOID HandleAccess(INS ins, BOOL isRead, BOOL* hasSegmentedMemAccess)
{
    UINT32 operandCount = INS_OperandCount(ins);
    UINT32 i, displacement, scale;
    REG baseReg = REG_INVALID(), indexReg = REG_INVALID();
    *hasSegmentedMemAccess = FALSE;

    for (i = 0; i < operandCount; i++)
    {
        if (INS_OperandIsMemory(ins, i) && (INS_OperandMemorySegmentReg(ins, i) == TESTED_SEG_REG) &&
            ((isRead && INS_OperandRead(ins, i)) || (!isRead && INS_OperandWritten(ins, i))))
        {
            displacement           = INS_OperandMemoryDisplacement(ins, i);
            baseReg                = INS_OperandMemoryBaseReg(ins, i);
            indexReg               = INS_OperandMemoryIndexReg(ins, i);
            scale                  = INS_OperandMemoryScale(ins, i);
            *hasSegmentedMemAccess = TRUE;
            break;
        }
    }
    if (baseReg != REG_INVALID())
    {
        if (indexReg != REG_INVALID())
        {
            if (isRead)
            {
                INS_InsertPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)AnalyzeSegmentedMemAccessBaseIndexDispl, IARG_INST_PTR,
                                         IARG_MEMORYREAD_EA, IARG_UINT32, BASE_INDEX_DISPLACEMENT_ADDRESSING_READ_TYPE,
                                         IARG_REG_VALUE, baseReg, IARG_REG_VALUE, indexReg, IARG_UINT32, scale, IARG_UINT32,
                                         displacement, IARG_THREAD_ID, IARG_END);
            }
            else
            {
                INS_InsertPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)AnalyzeSegmentedMemAccessBaseIndexDispl, IARG_INST_PTR,
                                         IARG_MEMORYWRITE_EA, IARG_UINT32, BASE_INDEX_DISPLACEMENT_ADDRESSING_WRITE_TYPE,
                                         IARG_REG_VALUE, baseReg, IARG_REG_VALUE, indexReg, IARG_UINT32, scale, IARG_UINT32,
                                         displacement, IARG_THREAD_ID, IARG_END);
            }
        }
        else
        {
            if (isRead)
            {
                INS_InsertPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)AnalyzeSegmentedMemAccessBaseDispl, IARG_INST_PTR,
                                         IARG_MEMORYREAD_EA, IARG_UINT32, BASE_DISPLACEMENT_ADDRESSING_READ_TYPE, IARG_REG_VALUE,
                                         baseReg, IARG_UINT32, displacement, IARG_THREAD_ID, IARG_END);
            }
            else
            {
                INS_InsertPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)AnalyzeSegmentedMemAccessBaseDispl, IARG_INST_PTR,
                                         IARG_MEMORYWRITE_EA, IARG_UINT32, BASE_DISPLACEMENT_ADDRESSING_WRITE_TYPE,
                                         IARG_REG_VALUE, baseReg, IARG_UINT32, displacement, IARG_THREAD_ID, IARG_END);
            }
        }
    }
    else if (indexReg != REG_INVALID())
    {
        if (isRead)
        {
            INS_InsertPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)AnalyzeSegmentedMemAccessIndexDispl, IARG_INST_PTR,
                                     IARG_MEMORYREAD_EA, IARG_UINT32, INDEX_DISPLACEMENT_ADDRESSING_READ_TYPE, IARG_REG_VALUE,
                                     indexReg, IARG_UINT32, scale, IARG_UINT32, displacement, IARG_THREAD_ID, IARG_END);
        }
        else
        {
            INS_InsertPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)AnalyzeSegmentedMemAccessIndexDispl, IARG_INST_PTR,
                                     IARG_MEMORYWRITE_EA, IARG_UINT32, INDEX_DISPLACEMENT_ADDRESSING_WRITE_TYPE, IARG_REG_VALUE,
                                     indexReg, IARG_UINT32, scale, IARG_UINT32, displacement, IARG_THREAD_ID, IARG_END);
        }
    }
    else if (*hasSegmentedMemAccess)
    {
        if (isRead)
        {
            INS_InsertPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)AnalyzeSegmentedMemAccessDispl, IARG_INST_PTR,
                                     IARG_MEMORYREAD_EA, IARG_UINT32, DISPLACEMENT_ONLY_ADDRESSING_READ_TYPE, IARG_UINT32,
                                     displacement, IARG_THREAD_ID, IARG_END);
        }
        else
        {
            INS_InsertPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)AnalyzeSegmentedMemAccessDispl, IARG_INST_PTR,
                                     IARG_MEMORYWRITE_EA, IARG_UINT32, DISPLACEMENT_ONLY_ADDRESSING_WRITE_TYPE, IARG_UINT32,
                                     displacement, IARG_THREAD_ID, IARG_END);
        }
    }
}

// Is called for every instruction and instruments reads and writes
VOID Instruction(INS ins, VOID* v)
{
    BOOL hasReadSegmentedMemAccess  = FALSE;
    BOOL hasWriteSegmentedMemAccess = FALSE;

    if (INS_SegmentRegPrefix(ins) == TESTED_SEG_REG)
    {
        if (INS_IsMemoryRead(ins))
        {
            HandleAccess(ins, TRUE /* isRead*/, &hasReadSegmentedMemAccess);
        }

        if (INS_IsMemoryWrite(ins))
        {
            HandleAccess(ins, FALSE /* isRead*/, &hasWriteSegmentedMemAccess);
        }
        if (!hasReadSegmentedMemAccess && !hasWriteSegmentedMemAccess)
        {
            fprintf(trace, "**ERROR SegMemAccess-Lies  %p %s\n", INS_Address(ins), INS_Disassemble(ins).c_str());
            hadError = TRUE;
        }
    }
}

VOID Fini(INT32 code, VOID* v)
{
    if (!hadError)
    {
        printf("SUCCESS\n");
        fflush(stdout);
    }
    else
    {
        printf("Had ERRORS - search above for string ERROR\n");
        fflush(stdout);
    }
}

//Thread functions

VOID ThreadFini(THREADID threadid, const CONTEXT* ctxt, INT32 code, VOID* v)
{
    int j;
    tdata* data = get_tls(threadid);
    if (data != NULL)
    {
        for (j = 0; j < MAX_ADDRESSING_TYPES; j++)
        {
            if (data->numEffectiveAddressesAnalyzed[j] == 0)
            {
                fprintf(trace, "ERROR - Thread %x: no segment[%s] based accesses\n", threadid, GetMemoryAccessTypeString(j));
                fflush(trace);
                hadError = TRUE;
            }
            else
            {
                fprintf(trace, "Thread %x: verified %d segmented[%s] accesses\n", threadid,
                        data->numEffectiveAddressesAnalyzed[j], GetMemoryAccessTypeString(j));
                fflush(trace);
            }
        }
        fprintf(trace, "\n");
        fflush(trace);
    }
}

PIN_LOCK pinLock;
THREADID myThread = INVALID_THREADID;

VOID ThreadStartUtil(THREADID threadid, CONTEXT* ctxt)
{
    fprintf(trace, "thread begin %x %x\n", threadid, numThreads);
    numThreads++;
    if (threadid < MAX_THREADS)
    {
#ifdef TARGET_WINDOWS
        tdata* data     = new tdata;
        data->threadTeb = InitializeThreadData();
        if (PIN_SetThreadData(tls_key, data, threadid) == FALSE)
        {
            cerr << "PIN_SetThreadData failed" << endl;
            PIN_ExitProcess(1);
        }
        if ((ADDRINT)PIN_GetContextReg(ctxt, TESTED_SEG_REG_BASE) != InitializeThreadData())
        {
            fprintf(trace, "ERROR - Initial value of tested segment base does not match TEB\n");
            hadError = TRUE;
        }
#endif
    }
    else
    {
        fprintf(trace, "ERROR - maximum #threads exceeded\n");
    }
    fflush(trace);
}

VOID ThreadStart(THREADID threadid, CONTEXT* ctxt, INT32 flags, VOID* v)
{
    if (INVALID_THREADID == myThread)
    {
        myThread = threadid;
        ThreadStartUtil(threadid, ctxt);
    }
}

VOID AppThreadStart(THREADID threadid, CONTEXT* ctxt)
{
    PIN_GetLock(&pinLock, PIN_GetTid());
    ThreadStartUtil(threadid, ctxt);
    PIN_ReleaseLock(&pinLock);
}

//Instrument the app thread rtn
VOID InstrumentRtn(RTN rtn, VOID*)
{
    if (PIN_UndecorateSymbolName(RTN_Name(rtn), UNDECORATION_NAME_ONLY) == "longfun" ||
        PIN_UndecorateSymbolName(RTN_Name(rtn), UNDECORATION_NAME_ONLY) == "shortfun")
    {
        RTN_Open(rtn);
        RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(AppThreadStart), IARG_THREAD_ID, IARG_CONTEXT, IARG_END);
        RTN_Close(rtn);
    }
}

int main(int argc, char* argv[])
{
    PIN_Init(argc, argv);
    PIN_InitSymbols();

    PIN_InitLock(&pinLock);

    tls_key = PIN_CreateThreadDataKey(NULL);
    if (tls_key == INVALID_TLS_KEY)
    {
        cerr << "number of already allocated keys reached the MAX_CLIENT_TLS_KEYS limit" << endl;
        PIN_ExitProcess(1);
    }

    RTN_AddInstrumentFunction(InstrumentRtn, NULL);
    PIN_AddThreadStartFunction(ThreadStart, NULL);
    INS_AddInstrumentFunction(Instruction, NULL);
    PIN_AddThreadFiniFunction(ThreadFini, NULL);
    PIN_AddFiniFunction(Fini, NULL);

    // Never returns
    PIN_StartProgram();

    return 1;
}
