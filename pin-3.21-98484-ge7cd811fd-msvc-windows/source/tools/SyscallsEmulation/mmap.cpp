/*
 * Copyright (C) 2004-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <sys/mman.h>
#include "pin.H"
using std::string;

#if defined(TARGET_MAC) && defined(TARGET_IA32E)
#define DECODE_SYSCALL(no) (0x2000000 | no)
#else
#define DECODE_SYSCALL(no) (no)
#endif

typedef enum
{
    SyscallCallbackType_PIN_AddSyscallEntryFunction = 1,
    SyscallCallbackType_INS_InsertCall              = 2
} SyscallCallbackType;

KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "mmap.out", "specify trace file name");
FILE* trace;
bool _matched_PIN_AddSyscallEntryFunction = false;
bool _matched_INS_InsertCall              = false;

// Print syscall number and arguments
VOID SysBefore(ADDRINT ip, ADDRINT num, ADDRINT arg0, ADDRINT arg1, ADDRINT arg2, ADDRINT arg3, ADDRINT arg4, ADDRINT arg5,
               ADDRINT type0)
{
    SyscallCallbackType type = (SyscallCallbackType)type0;
    ADDRINT offsetMultiplier = 1;
    bool isMmap              = (DECODE_SYSCALL(SYS_mmap) == num);
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
    else if (SYS_mmap2 == num)
    {
        isMmap           = true;
        offsetMultiplier = 0x1000;
    }
#endif

    if (isMmap)
    {
        if (arg0 == 0 && arg1 == (ADDRINT)getpagesize() && arg2 == PROT_READ && arg3 == (MAP_FILE | MAP_PRIVATE) &&
            arg4 != (ADDRINT)-1 && arg5 * offsetMultiplier == (ADDRINT)getpagesize() * 3)
        {
            switch (type)
            {
                case SyscallCallbackType_PIN_AddSyscallEntryFunction:
                    _matched_PIN_AddSyscallEntryFunction = true;
                    break;
                case SyscallCallbackType_INS_InsertCall:
                    _matched_INS_InsertCall = true;
                    break;
                default:
                    ASSERTX(FALSE);
            }
        }
    }
}

VOID SyscallEntry(THREADID threadIndex, CONTEXT* ctxt, SYSCALL_STANDARD std, VOID* v)
{
    SysBefore(PIN_GetContextReg(ctxt, REG_INST_PTR), PIN_GetSyscallNumber(ctxt, std), PIN_GetSyscallArgument(ctxt, std, 0),
              PIN_GetSyscallArgument(ctxt, std, 1), PIN_GetSyscallArgument(ctxt, std, 2), PIN_GetSyscallArgument(ctxt, std, 3),
              PIN_GetSyscallArgument(ctxt, std, 4), PIN_GetSyscallArgument(ctxt, std, 5),
              (ADDRINT)SyscallCallbackType_PIN_AddSyscallEntryFunction);
}

// Is called for every instruction and instruments syscalls
VOID Instruction(INS ins, VOID* v)
{
    // Try also to instrument the system call instruction.
    if (INS_IsSyscall(ins))
    {
        // Arguments and syscall number is only available before
        INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(SysBefore), IARG_INST_PTR, IARG_SYSCALL_NUMBER, IARG_SYSARG_VALUE, 0,
                       IARG_SYSARG_VALUE, 1, IARG_SYSARG_VALUE, 2, IARG_SYSARG_VALUE, 3, IARG_SYSARG_VALUE, 4, IARG_SYSARG_VALUE,
                       5, IARG_ADDRINT, (ADDRINT)SyscallCallbackType_INS_InsertCall, IARG_END);
    }
}

VOID Fini(INT32 code, VOID* v)
{
    if (!_matched_PIN_AddSyscallEntryFunction)
    {
        fprintf(trace, "Didn't had a callback for mmap with PIN_AddSyscallEntryFunction\n");
    }
    else if (!_matched_INS_InsertCall)
    {
        fprintf(trace, "Didn't had a callback for mmap with INS_InsertCall\n");
    }
    else
    {
        fprintf(trace, "Success!\n");
    }
    fclose(trace);
}

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage()
{
    PIN_ERROR("This tool prints a log of mmap system calls" + KNOB_BASE::StringKnobSummary() + "\n");
    return 1;
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

int main(int argc, char* argv[])
{
    if (PIN_Init(argc, argv)) return Usage();

    trace = fopen(KnobOutputFile.Value().c_str(), "w");
    ASSERTX(NULL != trace);

    INS_AddInstrumentFunction(Instruction, 0);
    PIN_AddSyscallEntryFunction(SyscallEntry, 0);

    PIN_AddFiniFunction(Fini, 0);

    // Never returns
    PIN_StartProgram();

    return 1;
}
