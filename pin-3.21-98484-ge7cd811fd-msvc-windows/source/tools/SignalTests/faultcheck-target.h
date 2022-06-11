/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#ifndef FAULT_CHECK_TARGET_H
#define FAULT_CHECK_TARGET_H

#include <signal.h>
#include "raise-exception-addrs.h"

typedef enum
{
    TSTATUS_NOFAULT, /* test did not raise fault */
    TSTATUS_SKIP,    /* skip this test */
    TSTATUS_DONE     /* there are no more tests */
} TSTATUS;

extern TSTATUS DoTest(unsigned int);
extern void PrintSignalContext(int, const siginfo_t*, void*);
extern void SetLabelsForPinTool(const RAISE_EXCEPTION_ADDRS*);

#endif
