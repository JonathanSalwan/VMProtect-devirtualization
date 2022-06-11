/*
 * Copyright (C) 2016-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// <COMPONENT>: os-apis
// <FILE-TYPE>: component public header

#ifndef OS_APIS_SIGACTION_KERNEL_H
#define OS_APIS_SIGACTION_KERNEL_H

#include "os-apis.h"

// The kernel's sigaction struct is different than the struct
// defined in signal.h.
// We need to define it here in order to pass it correctly to the kernel
#if defined(TARGET_LINUX)
typedef struct /*<POD>*/ kernel_sigaction
{
    void (*_handler)(int);
    unsigned long _flags;
    void (*_restorer)(void);
    UINT64 _mask;
} SIGACTION_KERNEL;
#else // not TARGET_LINUX
typedef struct /*<POD>*/ kernel_sigaction
{
    void (*_handler)(int);
    void (*_tramp)(void*, unsigned int, int, void*, void*);
    UINT32 _mask;
    UINT32 _flags;
} SIGACTION_KERNEL;
#endif // not TARGET_LINUX

#endif // file guard
