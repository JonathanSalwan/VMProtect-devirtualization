/*
 * Copyright (C) 2015-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// <COMPONENT>: os-apis
// <FILE-TYPE>: component public header

#ifndef OS_APIS_BARESYSCALL_H__
#define OS_APIS_BARESYSCALL_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "syscalltypes.h"

    typedef int OS_SYSCALL_TYPE;

#if defined(TARGET_WINDOWS)

#include "windows-baresyscall.h"

#endif

#if defined(TARGET_LINUX)

#include "linux-baresyscall.h"

#endif

#if defined(TARGET_MAC)

#include "mac-baresyscall.h"

#endif

    /*!
 * Perform a system call.
 * @param[in] sysno        The system call number.
 * @param[in] type         The system call type (linux, int80 , int81 ....).
 * @param[in] argCount     The number of system call parameters.
 * @param[in] ...          A variable number of system call parameters.
 *
 * @return  Returns a OS_SYSCALLRETURN object, which can be used to
 *          examine success and result values.
 */
    OS_SYSCALLRETURN OS_SyscallDo(ADDRINT sysno, OS_SYSCALL_TYPE type, unsigned argCount, ...);

#ifdef __cplusplus
}
#endif

#endif // file guard
