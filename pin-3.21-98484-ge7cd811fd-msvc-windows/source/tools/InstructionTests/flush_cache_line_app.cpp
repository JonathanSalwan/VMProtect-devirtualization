/*
 * Copyright (C) 2017-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <signal.h>

#ifdef TARGET_WINDOWS
#define ASMNAME(name)
#define EXPORT extern "C" __declspec(dllexport)
#else
#define ASMNAME(name) asm(name)
#define EXPORT extern "C"
#endif

extern "C" void ClFlushFunc();
extern "C" void ClFlushOptFunc();
extern "C" void ClwbFunc();

extern "C"
{
    void* funcPtr ASMNAME("funcPtr");
}

typedef void (*FUNPTR)(void*);

EXPORT void TellPinFlushParm(void* flushParm)
{
    // Pin tool can place instrumentation here to learn flush parm.
}

int func() { return 5; }

int lastInsLen = 0;

#ifndef TARGET_WINDOWS

/* Returning IP is stored in RIP (64bit) or EIP (32bit).
   We skip the offending instruction length! */
static void HandleSignal(int signal, siginfo_t* si, void* arg)
{
    ucontext_t* ctx = (ucontext_t*)arg;

#ifdef TARGET_MAC
#ifdef TARGET_IA32
    ctx->uc_mcontext->__ss.__eip += lastInsLen;
#else
    ctx->uc_mcontext->__ss.__rip += lastInsLen;
#endif
#else
#ifdef TARGET_IA32
    ctx->uc_mcontext.gregs[REG_EIP] += lastInsLen;
#else
    ctx->uc_mcontext.gregs[REG_RIP] += lastInsLen;
#endif
#endif
}

void SetupSignalHandler()
{
    struct sigaction sigact;

    memset(&sigact, 0, sizeof(sigact));
    sigemptyset(&sigact.sa_mask);
    sigact.sa_sigaction = HandleSignal;
    sigact.sa_flags     = SA_SIGINFO;
    if (-1 == sigaction(SIGILL, &sigact, 0))
    {
        printf("Unable to set up handler\n");
        exit(1);
    }
}

#endif

int main(int ac, char** av)
{
    funcPtr                          = (void*)func;
    volatile FUNPTR tellPinFlushParm = TellPinFlushParm;

#ifndef TARGET_WINDOWS
    SetupSignalHandler();
#endif

    tellPinFlushParm(funcPtr);

    //clflush length is 3
    lastInsLen = 3;

    func();
    try
    {
        ClFlushFunc();
    }
    catch (...)
    {
    }

    //clflushopt, clwb length is 4
    lastInsLen = 4;

    func();
    try
    {
        ClFlushOptFunc();
    }
    catch (...)
    {
    }

    func();
    try
    {
        ClwbFunc();
    }
    catch (...)
    {
    }

    return 0;
}
