/*
 * Copyright (C) 2007-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * This test verifies that we can call PIN_InterceptSignal() and PIN_UnblockSignal()
 * for all signals.  Some signals can't be intercepted, and Pin used to crash if
 * we attempted to intercept those signals.
 */

#include <pin.H>
#include <signal.h>
#include <iostream>

#ifdef TARGET_BSD
#ifndef SIGRTMIN
#define SIGRTMIN 128
#endif
#endif

#ifdef TARGET_MAC
#ifndef SIGRTMIN
#define SIGRTMIN 32
#endif
#endif

static BOOL Intercept(THREADID, INT32, CONTEXT*, BOOL, const EXCEPTION_INFO*, VOID*);

int main(int argc, char** argv)
{
    PIN_Init(argc, argv);

    for (int sig = 1; sig < SIGRTMIN; sig++)
    {
        PIN_InterceptSignal(sig, Intercept, 0);
        PIN_UnblockSignal(sig, TRUE);
    }

    PIN_StartProgram();
    return 0;
}

static BOOL Intercept(THREADID, INT32 sig, CONTEXT*, BOOL, const EXCEPTION_INFO*, VOID*)
{
    std::cerr << "Intercepted signal " << sig << std::endl;
    return TRUE;
}
