/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * This test verifies that Pin can change system call numbers and arguments.
 * Use in conjuction with the "change_syscall.c" application.
 */

#include <sys/syscall.h>
#include <signal.h>
#include <string.h>
#include "pin.H"

BOOL IsSigaction(ADDRINT sysnum)
{
#ifdef SYS_sigaction
    if (sysnum == SYS_sigaction) return TRUE;
#endif
#ifdef SYS_rt_sigaction
    if (sysnum == SYS_rt_sigaction) return TRUE;
#endif
    return FALSE;
}

VOID OnSyscallEntry(THREADID threadIndex, CONTEXT* ctxt, SYSCALL_STANDARD std, VOID* v)
{
    ADDRINT sysnum = PIN_GetSyscallNumber(ctxt, std);
    ADDRINT arg0   = PIN_GetSyscallArgument(ctxt, std, 0);

#if defined(TARGET_MAC)
#if defined(TARGET_IA32E)
    // Extract the syscall number without the UNIX mask
    sysnum = sysnum & (~(0xFF << 24));
#else
    // Extract the syscall number without the int80 mask
    sysnum = sysnum & 0xFFFF;
#endif
#endif

    char* filetoopen = NULL;
#ifdef SYS_openat
    if (sysnum == SYS_openat)
    {
        // open() is implemented using openat() in newer Bionic versions. The file is held in the second argument.
        ADDRINT arg1 = (PIN_GetSyscallArgument(ctxt, std, 1));
        filetoopen   = reinterpret_cast< char* >(arg1);
    }
#endif
    if (sysnum == SYS_open)
    {
        filetoopen = reinterpret_cast< char* >(arg0);
    }
    if (filetoopen != NULL && strncmp(filetoopen, "does-not-exist1", sizeof("does-not-exist1") - 1) == 0)
    {
        PIN_SetSyscallNumber(ctxt, std, SYS_getpid);
    }

    if (IsSigaction(sysnum) && (arg0 == SIGUSR1))
    {
        PIN_SetSyscallNumber(ctxt, std, SYS_getpid);
    }

    if (filetoopen && strncmp(filetoopen, "does-not-exist2", sizeof("does-not-exist2") - 1) == 0)
    {
        PIN_SetSyscallNumber(ctxt, std, SYS_exit);
        PIN_SetSyscallArgument(ctxt, std, 0, 0);
    }
}

int main(int argc, char* argv[])
{
    PIN_Init(argc, argv);

    PIN_AddSyscallEntryFunction(OnSyscallEntry, 0);

    PIN_StartProgram();
    return 0;
}
