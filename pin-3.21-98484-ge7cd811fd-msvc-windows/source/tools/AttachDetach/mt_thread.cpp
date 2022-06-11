/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <dlfcn.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/wait.h>
#include <string>

/* 
 * The total number of threads that should run in this process.
 */
const unsigned int numOfSecondaryThreads = 4;

/*
 * Timeout for the application to avoid hung tests.
 */
const unsigned int TIMEOUT = 600; // 10 minute timeout.

/*
 * Signal handler for handling a timeout situation.
 */
static void TimeoutHandler(int sig)
{
    fprintf(stderr, "mt_thread has reached a timeout of %d seconds.\n", TIMEOUT);
    exit(1);
}

/*
 * Main function for secondary threads.
 */
void* ThreadFunc(void* arg)
{
    unsigned int sum1 = 0;
    unsigned int sum2 = 0;
    for (unsigned int i = 0; i < 1000; ++i)
    {
        sum1 += i;
        sum2 -= i;
    }
    return NULL;
}

int main(int argc, char* argv[])
{
    // Set up the timeout handler.
    struct sigaction sigact_timeout;
    sigact_timeout.sa_handler = TimeoutHandler;
    sigact_timeout.sa_flags   = 0;
    sigfillset(&sigact_timeout.sa_mask);
    if (-1 == sigaction(SIGALRM, &sigact_timeout, 0))
    {
        perror("Unable to set up the timeout handler.");
        return 1;
    }
    alarm(TIMEOUT);

    // Create the secondary threads.
    pthread_t* thHandle;
    thHandle = new pthread_t[numOfSecondaryThreads];
    for (unsigned int repeat = 0; repeat < 10; repeat++)
    {
        // Create the threads.
        for (unsigned int i = 0; i < numOfSecondaryThreads; i++)
        {
            pthread_create(&thHandle[i], 0, ThreadFunc, (void*)i);
        }

        // Wait for the threads to exit.
        for (unsigned int i = 0; i < numOfSecondaryThreads; i++)
        {
            pthread_join(thHandle[i], 0);
        }
    }
    return 0;
}
