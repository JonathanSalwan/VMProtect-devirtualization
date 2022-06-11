/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * This test verifies the flags are correctly delivered at exceptions
 */
#define _GNU_SOURCE
#include <signal.h>
#include <sys/ucontext.h>
#include <stdio.h>
#include <setjmp.h>
#include <assert.h>
#include <sys/mman.h>

static sigjmp_buf JumpBuffer;
static unsigned int TestNumber = 0;

extern int GetFlags_asm();
// from faultcheck_flags_ia32_asm.s
extern int SetAppFlagsAndSegv_asm();
extern int ClearAppFlagsAndSegv_asm();
int DoTest(unsigned int tnum)
{
    switch (tnum)
    {
        case 0:
            printf("AllFlagsSet\n");
            SetAppFlagsAndSegv_asm();
            return (1);

        case 1:
            printf("AllFlagsClear\n");
            ClearAppFlagsAndSegv_asm();
            return (1);

        default:
            return (0);
    }
}

void PrintEflagsAtSignal(int sig, const siginfo_t* info, void* vctxt)
{
    ucontext_t* ctxt = vctxt;
    // note that the resume flag is turned on when exception occurs,
    // but if pin needs to retrieve the flags from memory then the
    // resume flag will not be there - so we test only the lower 16 bits
#if defined(TARGET_MAC)
    unsigned long flags = (unsigned long)ctxt->uc_mcontext->__ss.__eflags & 0xffff;
#else
    unsigned long flags = (unsigned long)ctxt->uc_mcontext.gregs[REG_EFL] & 0xffff;
#endif
    printf("  Signal %d, eflags=0x%lx\n", sig, flags);
}

static void Handle(int sig, siginfo_t* info, void* ctxt)
{
    PrintEflagsAtSignal(sig, info, ctxt);
    TestNumber++;
    siglongjmp(JumpBuffer, 1);
}

int main()
{
    struct sigaction sigact;

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

    sigsetjmp(JumpBuffer, 1);

    for (;;)
    {
        if (!DoTest(TestNumber))
        {
            printf("Last test\n");
            break;
        }
        printf("Failed to raise signal\n");
        TestNumber++;
    }

    return 0;
}
