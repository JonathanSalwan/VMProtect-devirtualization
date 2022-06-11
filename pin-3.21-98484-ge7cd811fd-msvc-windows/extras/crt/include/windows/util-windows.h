/*
 * Copyright (C) 2016-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// <COMPONENT>: os-apis
// <FILE-TYPE>: component public header

#ifndef OS_APIS_UITL_WINDOWS_H_
#define OS_APIS_UITL_WINDOWS_H_

#include "os-apis.h"
#include "types.h"
#include "baresyscall/baresyscall.h"
#include "win_syscalls.h"

void OS_SetSysCallTable(SYSCALL_NUMBER_T* input);

/*
 * Returns TRUE when Pin OS API uses native kernel32.dll functions
 * instead of internal implementation.
 */
BOOL_T UseKernel32();

typedef enum _BrokerApi
{
    writeFd                 = 0b00000001,
    readFd                  = 0b00000010,
    isConsoleFd             = 0b00000100,
    ntCreateFile            = 0b00001000,
    ntQueryAttributesFile   = 0b00010000,
    removeFile              = 0b00100000,
    ntAllocateVirtualMemory = 0b01000000,
    ntProtectVirtualMemory  = 0b10000000
} BrokerApi;

extern UINT32 OS_GetApiOverridesMask();
extern VOID OS_EnableBrokerApi(BrokerApi apiToEnable);
extern VOID OS_DisableBrokerApi(BrokerApi apiToDisable);

#endif // file guard
