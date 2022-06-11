/*
 * Copyright (C) 2007-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/* ===================================================================== */
/*! @file
  Replace an original function with a custom function defined in the tool. The
  new function can have either the same or different signature from that of its
  original function.  This tool is written for IA-32 and Intel(R) 64 architectures.
*/

/* ===================================================================== */
#include "pin.H"
#include <iostream>
#include <stdio.h>
using std::cout;
using std::dec;
using std::endl;
using std::hex;
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

VOID SyscallEntry(THREADID threadIndex, CONTEXT* ctxt, SYSCALL_STANDARD std, VOID* v)
{
    sysargs(PIN_GetSyscallNumber(ctxt, std), PIN_GetSyscallArgument(ctxt, std, 0), PIN_GetSyscallArgument(ctxt, std, 1),
            PIN_GetSyscallArgument(ctxt, std, 2), PIN_GetSyscallArgument(ctxt, std, 3), PIN_GetSyscallArgument(ctxt, std, 4),
            PIN_GetSyscallArgument(ctxt, std, 5));
}

VOID SyscallExit(THREADID threadIndex, CONTEXT* ctxt, SYSCALL_STANDARD std, VOID* v) { sysret(PIN_GetSyscallReturn(ctxt, std)); }

/* ===================================================================== */
/*          Replacement Routines                                         */
/* ===================================================================== */

/* ===================================================================== */

VOID* Jit_Malloc_IA32(CONTEXT* context, AFUNPTR orgFuncptr, size_t arg0)
{
    cout << "Jit_Malloc_IA32(" << hex << (ADDRINT)orgFuncptr << ", " << hex << arg0 << ") " << dec << endl;

    VOID* ret;

    PIN_CallApplicationFunction(context, PIN_ThreadId(), CALLINGSTD_DEFAULT, orgFuncptr, NULL, PIN_PARG(void*), &ret,
                                PIN_PARG(size_t), arg0, PIN_PARG_END());

    cout << "return value = " << hex << (ADDRINT)ret << dec << endl;

    return ret;
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
// This routine replaces malloc.

VOID ImageLoad(IMG img, VOID* v)
{
    cout << IMG_Name(img) << endl;

    PROTO proto_malloc = PROTO_Allocate(PIN_PARG(void*), CALLINGSTD_DEFAULT, "malloc", PIN_PARG(size_t), PIN_PARG_END());

    RTN rtn = RTN_FindByName(img, "malloc");
    if (RTN_Valid(rtn))
    {
        cout << "Replacing malloc in " << IMG_Name(img) << endl;

        RTN_ReplaceSignature(rtn, AFUNPTR(Jit_Malloc_IA32), IARG_PROTOTYPE, proto_malloc, IARG_CONTEXT, IARG_ORIG_FUNCPTR,
                             IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_END);
    }
}

/* ===================================================================== */

int main(INT32 argc, CHAR* argv[])
{
    PIN_InitSymbols();

    PIN_Init(argc, argv);

    IMG_AddInstrumentFunction(ImageLoad, 0);
    INS_AddInstrumentFunction(Ins, 0);
    PIN_AddSyscallEntryFunction(SyscallEntry, 0);
    PIN_AddSyscallExitFunction(SyscallExit, 0);

    PIN_StartProgram();

    return 0;
}

/* ===================================================================== */
/* eof                                                                   */
/* ===================================================================== */
