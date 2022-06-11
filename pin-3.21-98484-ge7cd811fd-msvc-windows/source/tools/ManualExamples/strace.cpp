/*
 * Copyright (C) 2004-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 *  This file contains an ISA-portable PIN tool for tracing system calls
 */

#include <stdio.h>

#if !defined(TARGET_WINDOWS)
#include <sys/syscall.h>
#endif

#include "pin.H"

FILE* trace;

// Print syscall number and arguments
VOID SysBefore(ADDRINT ip, ADDRINT num, ADDRINT arg0, ADDRINT arg1, ADDRINT arg2, ADDRINT arg3, ADDRINT arg4, ADDRINT arg5)
{
#if defined(TARGET_LINUX) && defined(TARGET_IA32)
    // On ia32 Linux, there are only 5 registers for passing system call arguments,
    // but mmap needs 6. For mmap on ia32, the first argument to the system call
    // is a pointer to an array of the 6 arguments
    if (num == SYS_mmap)
    {
        ADDRINT* mmapArgs = reinterpret_cast< ADDRINT* >(arg0);
        arg0              = mmapArgs[0];
        arg1              = mmapArgs[1];
        arg2              = mmapArgs[2];
        arg3              = mmapArgs[3];
        arg4              = mmapArgs[4];
        arg5              = mmapArgs[5];
    }
#endif

    fprintf(trace, "0x%lx: %ld(0x%lx, 0x%lx, 0x%lx, 0x%lx, 0x%lx, 0x%lx)", (unsigned long)ip, (long)num, (unsigned long)arg0,
            (unsigned long)arg1, (unsigned long)arg2, (unsigned long)arg3, (unsigned long)arg4, (unsigned long)arg5);
}

// Print the return value of the system call
VOID SysAfter(ADDRINT ret)
{
    fprintf(trace, "returns: 0x%lx\n", (unsigned long)ret);
    fflush(trace);
}

VOID SyscallEntry(THREADID threadIndex, CONTEXT* ctxt, SYSCALL_STANDARD std, VOID* v)
{
    SysBefore(PIN_GetContextReg(ctxt, REG_INST_PTR), PIN_GetSyscallNumber(ctxt, std), PIN_GetSyscallArgument(ctxt, std, 0),
              PIN_GetSyscallArgument(ctxt, std, 1), PIN_GetSyscallArgument(ctxt, std, 2), PIN_GetSyscallArgument(ctxt, std, 3),
              PIN_GetSyscallArgument(ctxt, std, 4), PIN_GetSyscallArgument(ctxt, std, 5));
}

VOID SyscallExit(THREADID threadIndex, CONTEXT* ctxt, SYSCALL_STANDARD std, VOID* v)
{
    SysAfter(PIN_GetSyscallReturn(ctxt, std));
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
        INS_InsertCall(ins, IPOINT_AFTER, AFUNPTR(SysAfter), IARG_SYSRET_VALUE, IARG_END);
    }
}

VOID Fini(INT32 code, VOID* v)
{
    fprintf(trace, "#eof\n");
    fclose(trace);
}

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage()
{
    PIN_ERROR("This tool prints a log of system calls" + KNOB_BASE::StringKnobSummary() + "\n");
    return -1;
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

int main(int argc, char* argv[])
{
    if (PIN_Init(argc, argv)) return Usage();

    trace = fopen("strace.out", "w");

    INS_AddInstrumentFunction(Instruction, 0);
    PIN_AddSyscallEntryFunction(SyscallEntry, 0);
    PIN_AddSyscallExitFunction(SyscallExit, 0);

    PIN_AddFiniFunction(Fini, 0);

    // Never returns
    PIN_StartProgram();

    return 0;
}
