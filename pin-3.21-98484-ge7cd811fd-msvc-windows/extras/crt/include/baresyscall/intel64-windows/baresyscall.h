/*
 * Copyright (C) 2017-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// <COMPONENT>: os-apis
// <FILE-TYPE>: component public header

#ifndef OS_APIS_WINDOWS_INTEL64_BARESYSCALL_H__
#define OS_APIS_WINDOWS_INTEL64_BARESYSCALL_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "types.h"
#include "baresyscall/intel64-windows/asm-baresyscall.h"

#define REG_SIZE HEX(8)
// 8 callee-saved registers
#define CALLEE_SAVED_REG HEX(8)
// System call arguments stack offset
// (shadow stack (0x20) + return address (0x8))
#define SYSCALL_ARG_STACK_OFFSET HEX(28)

    /*!
* Set of raw return values from a system call. Return value and scratch register values upon syscall execution.
*/
    typedef struct /*<POD>*/
    {
        long _status;
        ADDRINT _regs[OS_SCRATCH_REGS_NUM];
    } OS_SYSCALLRETURN;

#ifdef __cplusplus
}
#endif

#endif // file guard
