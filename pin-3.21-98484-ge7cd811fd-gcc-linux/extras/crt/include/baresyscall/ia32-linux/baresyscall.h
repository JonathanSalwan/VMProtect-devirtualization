/*
 * Copyright (C) 2017-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// <COMPONENT>: os-apis
// <FILE-TYPE>: component public header

#ifndef OS_APIS_LINUX_IA32_BARESYSCALL_H__
#define OS_APIS_LINUX_IA32_BARESYSCALL_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "types.h"

    /*!
 * Set of raw return values from a system call.
 */
    typedef struct /*<POD>*/
    {
        ADDRINT _eax;
        BOOL_T _success;
    } OS_SYSCALLRETURN;

    /*
 * This is the signal restorer which is called after a signal handler
 * had returned.
 * This is basically a system call to restore the original application's
 * stack.
 * This syscall never returns
 */
    void OS_SigReturn();

#ifdef __cplusplus
}
#endif

#endif // file guard
