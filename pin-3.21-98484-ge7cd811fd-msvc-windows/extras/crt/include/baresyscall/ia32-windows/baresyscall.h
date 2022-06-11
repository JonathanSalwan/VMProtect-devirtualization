/*
 * Copyright (C) 2017-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// <COMPONENT>: os-apis
// <FILE-TYPE>: component public header

#ifndef OS_APIS_WINDOWS_IA32_BARESYSCALL_H__
#define OS_APIS_WINDOWS_IA32_BARESYSCALL_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "types.h"

// Offset of the WOW64 syscall gate address in 32-bit TEB.
// The same symbol is also defined in ntdll.h  - compiler is expected to generate error
// if definitions are different.
#define TEBOFF_WOW64_GATE 192

#define WOW64_REG_SIZE HEX(4)
#define WOW64_ARGUMENT_OFFSET(argno) (1 + argno) * REG_SIZE

#define REG_SIZE HEX(4)
#define ARGUMENT_OFFSET(argno) (1 + argno) * REG_SIZE
    /*!
* Set of raw return values from a system call.
*/
    typedef struct /*<POD>*/
    {
        long _status;
    } OS_SYSCALLRETURN;

#ifdef __cplusplus
}
#endif

#endif // file guard
