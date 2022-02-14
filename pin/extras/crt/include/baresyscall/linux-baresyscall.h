/*
 * Copyright (C) 2017-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// <COMPONENT>: os-apis
// <FILE-TYPE>: component public header

#ifndef OS_APIS_LINUX_BARESYSCALL_H__
#define OS_APIS_LINUX_BARESYSCALL_H__

#ifdef __cplusplus
extern "C"
{
#endif

#if defined(TARGET_IA32)

#include "ia32-linux/baresyscall.h"
#else
#include "intel64-linux/baresyscall.h"

#endif

#include "unix-baresyscall.h"

    /*!
 *  The OS_SyscallDoClone system  call creates a new process, thae child execute continues from the point of the call.
 *  @param[in]  flags        Flags to create the child process with.
 *  @param[in]  childStack   The child_stack argument specifies the location of the stack used by the child process.
 *  @param[out] parentTid    The parent Tid.
 *  @param[in]  childTls     The tls of the new process.
 *  @param[out] childTid     The new process Tid.
 *
 * @return  Returns a OS_SYSCALLRETURN object, which can be used to
 *          examine success and result values.
 */
    OS_SYSCALLRETURN OS_SyscallDoClone(ADDRINT flags, void* childStack, ADDRINT* parentTid, void* childTls, ADDRINT* childTid);

    /*
 * The address of the instruction that performs a system call inside OS-APIs.
 */
    void* OS_GetInstructionAddressOfSyscall();

    /*
 * This is the signal restorer which is called after a signal handler
 * had returned. (from RT signals)
 * This is basically a system call to restore to restore original thread
 * context.
 * This syscall never returns
 */
    void OS_RtSigReturn();

#ifdef __cplusplus
}
#endif

#endif // file guard
