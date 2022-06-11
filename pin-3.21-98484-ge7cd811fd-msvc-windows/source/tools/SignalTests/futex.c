/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * This is a test case sent by a pinhead.  It used to demonstrate a problem on newer linux
 * kernels (e.g. 2.6.25) when a signal interrupts a futex system call.
 */

#include <stdio.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>

volatile int flag = 0;

static void Handler(int);
static void* Start(void*);

int main()
{
    pthread_t thrd1;
    pthread_t thrd2;

    pthread_create(&thrd1, 0, Start, 0);
    pthread_create(&thrd2, 0, Start, (void*)1);
    pthread_join(thrd1, 0);
    pthread_join(thrd2, 0);

    return 0;
}

static void Handler(int sig)
{
    flag = 1;
    printf("received signal\n");
}

static void* Start(void* v)
{
    if (v != 0)
    {
        printf("setting alarm\n");
        signal(SIGALRM, &Handler);
        alarm(2);
    }
    printf("waiting for signal at %p\n", &flag);
    while (!flag)
        ;
    return 0;
}
