/*
 * Copyright (C) 2011-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * This application is meant to be run with the Pin tool "call-app-tool.cpp".
 */

#include <stdio.h>
#include <signal.h>

extern "C" void PIN_TEST_FOO();
extern "C" void PIN_TEST_BAR();
static void Handle(int);

typedef void (*FUN)();
volatile FUN pFoo = PIN_TEST_FOO;

int main()
{
    struct sigaction act;
    act.sa_handler = Handle;
    act.sa_flags   = 0;
    sigemptyset(&act.sa_mask);
    if (sigaction(SIGUSR1, &act, 0) != 0)
    {
        fprintf(stderr, "Unable to set USR1 handler\n");
        return 1;
    }

    // Call through a function pointer to prevent the compiler from in-lining.
    //
    pFoo();

    printf("Main returning\n");
    return 0;
}

void PIN_TEST_FOO()
{
    // The Pin tool places an instrumentation point here, which calls PIN_TEST_BAR().

    printf("Calling Foo\n");
}

void PIN_TEST_BAR() { printf("Calling Bar\n"); }

static void Handle(int sig)
{
    if (sig == SIGUSR1)
        printf("Got SIGUSR1\n");
    else
        printf("Got unexpected signal %d\n", sig);
}
