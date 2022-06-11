/*
 * Copyright (C) 2017-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <pthread.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>

pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

void* threadMain(void* arg)
{
    fprintf(stderr, "In second thread: starting\n");

    pthread_mutex_lock(&mtx);
    pthread_mutex_lock(&mtx);

    /* we should not get here */
    assert(0);
}

int main()
{
    struct sigaction act;
    pthread_t thd;
    int i;

    /* make poke signal restartable to reproduce mantis: 4073 */
    for (i = 1; i < NSIG; i++)
    {
        sigaction(i, 0, &act);
        act.sa_flags |= SA_RESTART;
        sigaction(i, &act, 0);
    }

    pthread_create(&thd, NULL, &threadMain, 0);

    fprintf(stderr, "In the main thread: Waiting for second thread to be ready\n");

    /* wait for the thread to be ready */
    while (1)
    {
        sleep(1);
        if (0 == pthread_mutex_trylock(&mtx))
        {
            pthread_mutex_unlock(&mtx);
        }
        else
        {
            // First pthread_mutex_lock in the other thread was executed.
            break;
        }
    }

    /* wait three more seconds to make sure the thread is blocked in the second pthread_mutex_lock */
    sleep(3);

    fprintf(stderr, "In the main thread: Exiting\n");

    exit(0);
}
