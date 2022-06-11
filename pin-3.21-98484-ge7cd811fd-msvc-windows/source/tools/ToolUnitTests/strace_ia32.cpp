/*
 * Copyright (C) 2006-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 *  This file contains an IA32 specific test for checking the return value of system calls.
 */

#include <iostream>
#include <fstream>
#include <stdlib.h>

#if defined(TARGET_MAC)
#include <sys/syscall.h>
#else
#include <syscall.h>
#endif

#include "pin.H"
using std::cout;
using std::dec;
using std::endl;
using std::hex;
using std::ofstream;
using std::string;

ofstream trace;

KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "strace.out", "Output file");

// Print syscall number and arguments
VOID SysBefore(ADDRINT ip, ADDRINT num, ADDRINT arg0, ADDRINT arg1, ADDRINT arg2, ADDRINT arg3, ADDRINT arg4, ADDRINT arg5)
{
    trace << "@ip 0x" << hex << ip << ": sys call " << dec << num;
    trace << "(0x" << hex << arg0 << ", 0x" << arg1 << ", 0x" << arg2;
    trace << hex << ", 0x" << arg3 << ", 0x" << arg4 << ", 0x" << arg5 << ")" << endl;
}

// Print the return value of the system call
VOID SysAfter(ADDRINT value, INT32 err, UINT32 gax)
{
    int error       = 0;
    ADDRINT neg_one = (ADDRINT)(0 - 1);

    if (err == 0)
    {
        if (gax != value) error = 1;
    }
    else
    {
        if (value != neg_one) error = 3;
        if (err != -(INT32)gax) error = 4;
    }

    if (error == 0)
        trace << "Success: value=0x" << hex << value << ", errno=" << dec << err << endl;
    else
    {
        trace << "Failure " << error << ": value=0x" << hex << value << ", errno=" << dec << err;
        trace << ", gax=0x" << hex << gax << endl;
    }

    trace << endl;
}

VOID SyscallEntry(THREADID threadIndex, CONTEXT* ctxt, SYSCALL_STANDARD std, VOID* v)
{
    SysBefore(PIN_GetContextReg(ctxt, REG_INST_PTR), PIN_GetSyscallNumber(ctxt, std), PIN_GetSyscallArgument(ctxt, std, 0),
              PIN_GetSyscallArgument(ctxt, std, 1), PIN_GetSyscallArgument(ctxt, std, 2), PIN_GetSyscallArgument(ctxt, std, 3),
              PIN_GetSyscallArgument(ctxt, std, 4), PIN_GetSyscallArgument(ctxt, std, 5));
}

VOID SyscallExit(THREADID threadIndex, CONTEXT* ctxt, SYSCALL_STANDARD std, VOID* v)
{
    SysAfter(PIN_GetSyscallReturn(ctxt, std), PIN_GetSyscallErrno(ctxt, std), PIN_GetContextReg(ctxt, REG_GAX));
}

// Is called for every instruction and instruments syscalls
VOID Instruction(INS ins, VOID* v)
{
    // For O/S's (macOS*) that don't support PIN_AddSyscallEntryFunction(),
    // instrument the system call instruction.

    if (INS_IsSyscall(ins) && INS_IsValidForIpointAfter(ins))
    {
        // Arguments and syscall number is only available before
        INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(SysBefore), IARG_INST_PTR, IARG_SYSCALL_NUMBER, IARG_SYSARG_VALUE, 0,
                       IARG_SYSARG_VALUE, 1, IARG_SYSARG_VALUE, 2, IARG_SYSARG_VALUE, 3, IARG_SYSARG_VALUE, 4, IARG_SYSARG_VALUE,
                       5, IARG_END);

        // return value only available after
        INS_InsertCall(ins, IPOINT_AFTER, AFUNPTR(SysAfter), IARG_SYSRET_VALUE, IARG_SYSRET_ERRNO, IARG_REG_VALUE, REG_GAX,
                       IARG_END);
    }
}

VOID Fini(INT32 code, VOID* v)
{
    trace << "#eof" << endl;
    trace.close();
}

int main(int argc, char* argv[])
{
    PIN_Init(argc, argv);

    trace.open(KnobOutputFile.Value().c_str());
    if (!trace.is_open())
    {
        cout << "Could not open strace.out" << endl;
        exit(1);
    }

    INS_AddInstrumentFunction(Instruction, 0);
    PIN_AddSyscallEntryFunction(SyscallEntry, 0);
    PIN_AddSyscallExitFunction(SyscallExit, 0);

    PIN_AddFiniFunction(Fini, 0);

    // Never returns
    PIN_StartProgram();

    return 0;
}
