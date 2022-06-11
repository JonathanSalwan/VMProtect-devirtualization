/*
 * Copyright (C) 2007-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * Verify that Pin can trace an application that uses asynchronous cancel.
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <sched.h>
#include <unistd.h>

#define TIMEOUT 3 * 60

static void* Thread(void*);
static void Terminate(int);

static volatile int Ready = 0;

int main()
{
    pthread_t tid;

    pthread_create(&tid, 0, Thread, 0);

    while (!Ready)
        sched_yield();

    pthread_cancel(tid);
    pthread_join(tid, 0);

    printf("Thread was cancelled\n");
    return 0;
}

volatile unsigned Val;
volatile unsigned A = 5;
volatile unsigned B = 4;

static void* Thread(void* dummy)
{
    struct sigaction act;

    /*
     * This causes the thread to terminate even if it doesn't receive the cancel
     * request.  The handler prints an error message, which causes the test to
     * fail.
     */
    act.sa_handler = Terminate;
    act.sa_flags   = 0;
    sigemptyset(&act.sa_mask);
    sigaction(SIGALRM, &act, 0);
    alarm(TIMEOUT);

    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, 0);
    Ready = 1;

    for (;;)
        Val = A / B;
}

static void Terminate(int sig)
{
    printf("Thread not cancelled\n");
    exit(1);
}
