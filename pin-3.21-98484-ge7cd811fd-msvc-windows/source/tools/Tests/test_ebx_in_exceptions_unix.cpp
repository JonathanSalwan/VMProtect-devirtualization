/*
 * Copyright (C) 2012-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * This test causes and handles access violations/bus errors in
   instructions that use the ebx register implicitly.
   The relevant instructions are cmpxchg8b and xlat (which uses ebx as
   a base register)
 */

#define NEED_UCONTEXT_T
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <setjmp.h>
#include <signal.h>
#include <errno.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <cstdlib>
using std::fflush;
using std::fprintf;
using std::string;

extern "C" unsigned int GetInstructionLenAndDisasm(unsigned char* ip, string* str);
extern "C" int TestAccessViolations();
extern "C" void InitXed();
int numExceptions = 0;

void PrintSignalContext(int sig, const siginfo_t* info, void* vctxt)
{
    ucontext_t* ctxt = (ucontext_t*)vctxt;
    unsigned long rip;
    long int trapno;

#if defined(TARGET_LINUX) && defined(TARGET_IA32E)
    rip    = (unsigned long)ctxt->uc_mcontext.gregs[REG_RIP];
    trapno = (long int)ctxt->uc_mcontext.gregs[REG_TRAPNO];
#elif defined(TARGET_LINUX) && defined(TARGET_IA32)
    rip    = (unsigned long)ctxt->uc_mcontext.gregs[REG_EIP];
    trapno = (long int)ctxt->uc_mcontext.gregs[REG_TRAPNO];
#elif defined(TARGET_MAC) && defined(TARGET_IA32E)
    rip    = (unsigned long)ctxt->uc_mcontext->ss.rip;
    trapno = (long int)ctxt->uc_mcontext->es.trapno;
#elif defined(TARGET_MAC) && defined(TARGET_IA32)
    rip    = (unsigned long)ctxt->uc_mcontext->ss.eip;
    trapno = (long int)ctxt->uc_mcontext->es.trapno;
#endif

    fprintf(stderr, "  PrintSignal: sig %d, pc=0x%lx, si_errno=%d, trap_no=%ld", sig, rip, (int)info->si_errno, trapno);

    fprintf(stderr, "\n");
}

static void Handle(int sig, siginfo_t* info, void* v)
{
    fprintf(stderr, "Handle\n");
    fflush(stderr);

    ucontext_t* ctxt = (ucontext_t*)v;

    PrintSignalContext(sig, info, v);
    numExceptions++;

    unsigned int* ipToContinueAt = (unsigned int*)
#if defined(TARGET_LINUX) && defined(TARGET_IA32)
                                       ctxt->uc_mcontext.gregs[REG_EIP];
#elif defined(TARGET_LINUX) && defined(TARGET_IA32E)
                                       ctxt->uc_mcontext.gregs[REG_RIP];
#elif defined(TARGET_MAC) && defined(TARGET_IA32)
                                       ctxt->uc_mcontext->ss.eip;
#elif defined(TARGET_MAC) && defined(TARGET_IA32E)
                                       ctxt->uc_mcontext->ss.rip;
#else
#error "Undefined code"
#endif
    string str;
    unsigned int instructionLen = GetInstructionLenAndDisasm((unsigned char*)ipToContinueAt, &str);
    if (0 == instructionLen)
    {
        fprintf(stderr, "***Error 0 length instruction at ip %p\n", ipToContinueAt);
        exit(1);
    }
    fprintf(stderr, "segv at: %s\n", str.c_str());
    ipToContinueAt = (unsigned int*)((unsigned char*)ipToContinueAt + instructionLen);
    fprintf(stderr, " setting resume ip to %p\n", ipToContinueAt);
    instructionLen = GetInstructionLenAndDisasm((unsigned char*)ipToContinueAt, &str);
    if (0 == instructionLen)
    {
        fprintf(stderr, "***Error 0 length instruction at ip %p\n", ipToContinueAt);
        exit(1);
    }
    fprintf(stderr, "  resume instruction is %s\n", str.c_str());
#if defined(TARGET_LINUX) && defined(TARGET_IA32)
    ctxt->uc_mcontext.gregs[REG_EIP] =
#elif defined(TARGET_LINUX) && defined(TARGET_IA32E)
    ctxt->uc_mcontext.gregs[REG_RIP] =
#elif defined(TARGET_MAC) && defined(TARGET_IA32)
    ctxt->uc_mcontext->ss.eip =
#elif defined(TARGET_MAC) && defined(TARGET_IA32E)
    ctxt->uc_mcontext->ss.rip =
#else
#error "Undefined code"
#endif
        (unsigned long)(ipToContinueAt);
}

int main(int argc, char** argv)
{
    struct sigaction sigact;

    InitXed();
    sigact.sa_sigaction = Handle;
    sigemptyset(&sigact.sa_mask);
    sigact.sa_flags = SA_SIGINFO;
    if (sigaction(SIGSEGV, &sigact, 0) == -1)
    {
        fprintf(stderr, "Unable handle SIGSEGV\n");
        return 1;
    }
    if (sigaction(SIGBUS, &sigact, 0) == -1)
    {
        fprintf(stderr, "Unable handle SIGBUS\n");
        return 1;
    }

    fprintf(stderr, "calling TestAccessViolations\n");
    int retVal = TestAccessViolations();

    if (!retVal || numExceptions != 3)
    {
        fprintf(stderr, "***Error\n");
        exit(1);
    }
}
