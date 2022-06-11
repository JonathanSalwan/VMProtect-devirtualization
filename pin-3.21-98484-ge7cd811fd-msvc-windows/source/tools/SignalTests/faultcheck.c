/*
 * Copyright (C) 2007-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * This test tries to raise each type of synchronous signal and prints
 * out various state information about the fault context.  The test can
 * be run under Pin to test whether the fields have the same values
 * when run natively.
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <setjmp.h>
#include <signal.h>
#include <errno.h>
#include "raise-exception-addrs.h"
#include "faultcheck-target.h"

static sigjmp_buf JumpBuffer;
static unsigned int TestNumber = 0;

static void Handle(int, siginfo_t*, void*);
int Initialize();

int main(int argc, char** argv)
{
    struct sigaction sigact;
    unsigned int OneTest;
    int DoOneTest = 0;

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
    if (sigaction(SIGILL, &sigact, 0) == -1)
    {
        fprintf(stderr, "Unable handle SIGILL\n");
        return 1;
    }
    if (sigaction(SIGFPE, &sigact, 0) == -1)
    {
        fprintf(stderr, "Unable handle SIGSEGV\n");
        return 1;
    }
    if (sigaction(SIGTRAP, &sigact, 0) == -1)
    {
        fprintf(stderr, "Unable handle SIGTRAP\n");
        return 1;
    }
    if (!Initialize()) return 1;

    if (argc > 2)
    {
        fprintf(stderr, "Usage: %s [test-number]\n", argv[0]);
        return 1;
    }
    if (argc == 2)
    {
        errno   = 0;
        OneTest = strtoul(argv[1], 0, 10);
        if (errno)
        {
            fprintf(stderr, "Invalid test number '%s'\n", argv[1]);
            return 1;
        }
        DoOneTest  = 1;
        TestNumber = OneTest;
    }

    sigsetjmp(JumpBuffer, 1);
    if (DoOneTest && TestNumber != OneTest) return 0;

    for (;;)
    {
        printf("Starting test %d ...\n", TestNumber);
        switch (DoTest(TestNumber))
        {
            case TSTATUS_DONE:
                printf("  Last test\n");
                return 0;
            case TSTATUS_SKIP:
                printf("  Skipped\n");
                break;
            case TSTATUS_NOFAULT:
                printf("  ** Failed to raise signal **\n");
                break;
            default:
                assert(0);
                break;
        }

        TestNumber++;
        if (DoOneTest) return 0;
    }
}

void SetLabelsForPinTool(const RAISE_EXCEPTION_ADDRS* addrs)
{
    /*
     * The Pin tool instruments this routine in order to find the addresses
     * of the instructions that raise exceptions.
     */
}

static void Handle(int sig, siginfo_t* info, void* ctxt)
{
    PrintSignalContext(sig, info, ctxt);
    TestNumber++;
    siglongjmp(JumpBuffer, 1);
}
