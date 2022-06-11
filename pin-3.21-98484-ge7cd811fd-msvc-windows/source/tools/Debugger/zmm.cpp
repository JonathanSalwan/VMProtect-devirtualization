/*
 * Copyright (C) 2017-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <iostream>
#include <cstdlib>
#include <signal.h>
#include "tool_macros.h"

static void HandleSigill(int);
extern "C" void loadYmm0(const unsigned char*) ASMNAME("loadYmm0");
extern "C" void loadZmm0(const unsigned char*) ASMNAME("loadZmm0");
extern "C" void loadK0(const unsigned char*) ASMNAME("loadK0");

int main()
{
#if defined(TARGET_LINUX) || defined(TARGET_MAC)
    // Create a SIGILL handler in case this processor does not support
    // AVX512 instructions. for Linux
    //
    struct sigaction act;
    act.sa_handler = HandleSigill;
    act.sa_flags   = 0;
    sigemptyset(&act.sa_mask);
    sigaction(SIGILL, &act, 0);
#endif

    unsigned i;
    unsigned char ymmVal[32];
    unsigned char zmmVal[64];
    unsigned char kVal[8];

    for (i = 0; i < sizeof(ymmVal); i++)
        ymmVal[i] = static_cast< unsigned char >(i + 1);

    for (i = 0; i < sizeof(zmmVal); i++)
        zmmVal[i] = static_cast< unsigned char >(i + 1);

    for (i = 0; i < sizeof(kVal); i++)
        kVal[i] = static_cast< unsigned char >(i + 1);

    // If the processor supports AVX512, the debugger stops at a breakpoint
    // immediately after loading each register.  Otherwise, the debugger stops at
    // a breakpoint in HandleSigill().
    //
    try
    {
        loadK0(kVal);
        loadYmm0(ymmVal);
        loadZmm0(zmmVal);
    }
    catch (...)
    {
        HandleSigill(0);
    }

    return 0;
}

static void HandleSigill(int sig)
{
    std::cout << "Processor does not support AVX512" << std::endl;
    std::exit(0);
}
