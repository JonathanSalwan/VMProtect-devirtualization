/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include "pin.h"
#include <stdio.h>

namespace WINDOWS
{
#include <windows.h>
}

typedef WINDOWS::PVOID(__stdcall* RtlAllocateHeapType)(WINDOWS::PVOID, WINDOWS::ULONG, WINDOWS::SIZE_T);

using std::string;
/* ===================================================================== */
/*          Analysis Routines                                            */
/* ===================================================================== */

/* ===================================================================== */
// Print every instruction that is executed.

void printIp(ADDRINT v, char* dis) { fprintf(stderr, "Ip: 0x%lx %s\n", (unsigned long)v, dis); }

/* ===================================================================== */
// Print the return value of the system call.

void sysret(ADDRINT v) { fprintf(stderr, "sysret: 0x%lx\n", (unsigned long)v); }

/* ===================================================================== */
// Print the arguments to the system call.

void sysargs(ADDRINT num, ADDRINT p0, ADDRINT p1, ADDRINT p2, ADDRINT p3, ADDRINT p4, ADDRINT p5)
{
    fprintf(stderr, "syscall: %ld  sysargs: 0x%lx 0x%lx 0x%lx 0x%lx 0x%lx 0x%lx\n", (long)num, (unsigned long)p0,
            (unsigned long)p1, (unsigned long)p2, (unsigned long)p3, (unsigned long)p4, (unsigned long)p5);
}
/* ===================================================================== */

VOID SyscallEntry(THREADID threadIndex, CONTEXT* ctxt, SYSCALL_STANDARD std, VOID* v)
{
    sysargs(PIN_GetSyscallNumber(ctxt, std), PIN_GetSyscallArgument(ctxt, std, 0), PIN_GetSyscallArgument(ctxt, std, 1),
            PIN_GetSyscallArgument(ctxt, std, 2), PIN_GetSyscallArgument(ctxt, std, 3), PIN_GetSyscallArgument(ctxt, std, 4),
            PIN_GetSyscallArgument(ctxt, std, 5));
}
/* ===================================================================== */

VOID SyscallExit(THREADID threadIndex, CONTEXT* ctxt, SYSCALL_STANDARD std, VOID* v) { sysret(PIN_GetSyscallReturn(ctxt, std)); }

/* ===================================================================== */
/*          Replacement Routines                                         */
/* ===================================================================== */

/*
 * replacement_RtlAllocateHeap
 *
 */
void* replacement_RtlAllocateHeap(AFUNPTR pfnRtlAllocateHeap, WINDOWS::PVOID HeapHandle, WINDOWS::ULONG Flags,
                                  WINDOWS::SIZE_T Size, CONTEXT* ctxt)
{
    fprintf(stderr, "In " __FUNCTION__ ", pfnRtlAllocateHeap: %p,HeapHandle: %p, Flags: %08x, Size: %d\n", pfnRtlAllocateHeap,
            HeapHandle, Flags, Size);

    // Call the original function
    void* result;
    PIN_CallApplicationFunction(ctxt, PIN_ThreadId(), CALLINGSTD_STDCALL, pfnRtlAllocateHeap, NULL, PIN_PARG(void*), &result,
                                PIN_PARG(WINDOWS::PVOID), HeapHandle, PIN_PARG(WINDOWS::ULONG), Flags, PIN_PARG(WINDOWS::SIZE_T),
                                Size, PIN_PARG_END());

    fprintf(stderr, "pfnRtlAllocateHeap returned %p\n", result);

    return result;
}

/* ===================================================================== */
/*          Instrumentation Routines                                     */
/* ===================================================================== */

/* ===================================================================== */
// Instrument each system call to print arguments and return value.
// Instrument each instruction to print itself.

void Ins(INS ins, void* v)
{
    string* st = new string(INS_Disassemble(ins));

    // For O/S's (macOS*) that don't support PIN_AddSyscallEntryFunction(),
    // instrument the system call instruction.

    if (INS_IsSyscall(ins) && INS_IsValidForIpointAfter(ins))
    {
        INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(sysargs), IARG_SYSCALL_NUMBER, IARG_SYSARG_VALUE, 0, IARG_SYSARG_VALUE, 1,
                       IARG_SYSARG_VALUE, 2, IARG_SYSARG_VALUE, 3, IARG_SYSARG_VALUE, 4, IARG_SYSARG_VALUE, 5, IARG_END);
        INS_InsertCall(ins, IPOINT_AFTER, AFUNPTR(sysret), IARG_SYSRET_VALUE, IARG_END);
    }

    INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(printIp), IARG_INST_PTR, IARG_PTR, st->c_str(), IARG_END);
}

/* ===================================================================== */

void ImageLoad(IMG img, void* context)
{
    fprintf(stderr, "Notified of load of %s at [%p,%p]\n", IMG_Name(img).c_str(), (char*)IMG_LowAddress(img),
            (char*)IMG_HighAddress(img));

    // See if this is ntdll.dll

    char szName[_MAX_FNAME];
    char szExt[_MAX_EXT];

    _splitpath_s(IMG_Name(img).c_str(), NULL, 0, NULL, 0, szName, _MAX_FNAME, szExt, _MAX_EXT);
    strcat_s(szName, _MAX_FNAME, szExt);

    if (0 != _stricmp("ntdll.dll", szName)) return;

    RTN rtn = RTN_FindByName(img, "RtlAllocateHeap");

    if (RTN_Invalid() == rtn)
    {
        fprintf(stderr, "Failed to find RtlAllocateHeap in %s\n", IMG_Name(img).c_str());
        return;
    }
    fprintf(stderr, "Replacing\n");
    PROTO protoRtlAllocateHeap = PROTO_Allocate(PIN_PARG(void*), CALLINGSTD_STDCALL, "RtlAllocateHeap",
                                                PIN_PARG(WINDOWS::PVOID),  // HeapHandle
                                                PIN_PARG(WINDOWS::ULONG),  // Flags
                                                PIN_PARG(WINDOWS::SIZE_T), // Size
                                                PIN_PARG_END());

    RTN_ReplaceSignature(rtn, (AFUNPTR)replacement_RtlAllocateHeap, IARG_PROTOTYPE, protoRtlAllocateHeap, IARG_ORIG_FUNCPTR,
                         IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_FUNCARG_ENTRYPOINT_VALUE, 1, IARG_FUNCARG_ENTRYPOINT_VALUE, 2,
                         IARG_CONTEXT, IARG_END);

    PROTO_Free(protoRtlAllocateHeap);
}

/* ===================================================================== */

void Fini(int code, void* context) { fprintf(stderr, "Finishing with code %d\n", code); }

/* ===================================================================== */

int main(int argc, char* argv[])
{
    PIN_InitSymbols();
    PIN_Init(argc, argv);

    IMG_AddInstrumentFunction(ImageLoad, 0);
    //    INS_AddInstrumentFunction(Ins, 0);
    //    PIN_AddSyscallEntryFunction(SyscallEntry, 0);
    //    PIN_AddSyscallExitFunction(SyscallExit, 0);

    PIN_AddFiniFunction(Fini, 0);
    PIN_StartProgram();

    return 0;
}
