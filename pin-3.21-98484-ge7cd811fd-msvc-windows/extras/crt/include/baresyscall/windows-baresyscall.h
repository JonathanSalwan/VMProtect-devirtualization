/*
 * Copyright (C) 2017-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// <COMPONENT>: os-apis
// <FILE-TYPE>: component public header

#ifndef OS_APIS_WINDOWS_BARESYSCALL_H__
#define OS_APIS_WONDOWS_BARESYSCALL_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "types.h"

#if defined(TARGET_IA32)

#include "ia32-windows/baresyscall.h"
#else
#include "intel64-windows/baresyscall.h"

#endif

    /*!
 * Perform a system call.
 *  @param[in] sysno        The system call number.
 *  @param[in] args         Array of the system call parameters.
 *  @param[in] argCount     The number of system call parameters.
 *  @param[in] type         The system call type (linux, int80 , int81 ....).
 *
 * @return  Returns a OS_SYSCALLRETURN object, which can be used to
 *          examine success and result values.
 */
    OS_SYSCALLRETURN OS_SyscallDoCall(ADDRINT sysno, const ADDRINT* args, ADDRINT argCount, OS_SYSCALL_TYPE);

#ifdef __cplusplus
}
#endif

#endif // file guard
