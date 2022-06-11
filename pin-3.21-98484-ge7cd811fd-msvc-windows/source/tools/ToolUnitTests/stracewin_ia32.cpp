/*
 * Copyright (C) 2004-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 *  This file contains an IA32 specific test for checking the return value of system calls.
 */

#include <iostream>
#include <fstream>
//#include <windows.h>

#include "pin.H"

using std::cout;
using std::dec;
using std::endl;
using std::hex;
using std::ofstream;
ofstream trace;

// Print syscall number and arguments
VOID SysBefore(ADDRINT ip, ADDRINT num, ADDRINT arg0, ADDRINT arg1, ADDRINT arg2, ADDRINT arg3, ADDRINT arg4, ADDRINT arg5)
{
    trace << "@ip 0x" << hex << ip << ": sys call " << dec << num;
    trace << "(0x" << hex << arg0 << ", 0x" << arg1 << ", 0x" << arg2;
    trace << hex << ", 0x" << arg3 << ", 0x" << arg4 << ", 0x" << arg5 << ")" << endl;
}

// Print the return value of the system call
VOID SysAfter(ADDRINT err)
{
    if (err == 0)
    {
        trace << "Success: errno=" << err << endl;
    }
    else
    {
        trace << "Failure: errno=" << err << endl;
    }
}

VOID SyscallEntry(THREADID threadIndex, CONTEXT* ctxt, SYSCALL_STANDARD std, VOID* v)
{
    SysBefore(PIN_GetContextReg(ctxt, REG_INST_PTR), PIN_GetSyscallNumber(ctxt, std), PIN_GetSyscallArgument(ctxt, std, 0),
              PIN_GetSyscallArgument(ctxt, std, 1), PIN_GetSyscallArgument(ctxt, std, 2), PIN_GetSyscallArgument(ctxt, std, 3),
              PIN_GetSyscallArgument(ctxt, std, 4), PIN_GetSyscallArgument(ctxt, std, 5));
}

VOID SyscallExit(THREADID threadIndex, CONTEXT* ctxt, SYSCALL_STANDARD std, VOID* v) { SysAfter(PIN_GetSyscallErrno(ctxt, std)); }

VOID Fini(INT32 code, VOID* v)
{
    trace << "#eof" << endl;
    trace.close();
}

int main(int argc, char* argv[])
{
    PIN_Init(argc, argv);

    trace.open("stracewin.out");
    if (!trace.is_open())
    {
        cout << "Could not open strace.out" << endl;
        exit(1);
    }

    PIN_AddSyscallEntryFunction(SyscallEntry, 0);
    PIN_AddSyscallExitFunction(SyscallExit, 0);

    PIN_AddFiniFunction(Fini, 0);

    // Never returns
    PIN_StartProgram();

    return 0;
}
