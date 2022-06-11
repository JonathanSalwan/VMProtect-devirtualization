/*
 * Copyright (C) 2012-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <sched.h>
#include <signal.h>
#include <errno.h>
#include "../Utils/threadlib.h"

#define TIMEOUT 20
#define NUM_TH 4

/*
 * This application is part of the test: "fork_hang.test" ". This test checks that no deadlock has occured when  
 * child process tries to acquire the VM lock in a post fork callback in this child process.
 * 
 * It also tests behavior when a multi-threaded app forks and doesn't execv.
 */

enum RESULT
{
    RES_SUCCESS = 0,
    RES_FORK_FAILED,
    RES_TIMEOUT,
    RES_WAITPID_FAILED
};

bool alarmRinged = false;

/*
 * A signal handler for SIGALRM
 */
void SigAlrmHandler(int signum, siginfo_t* siginfo, void* uctxt) { alarmRinged = true; }

void* InfiniteLoop(void* ptr)
{
    for (;;)
        ;
}

int main()
{
    /* Register the signal handler */
    struct sigaction sSigaction;
    sSigaction.sa_sigaction = SigAlrmHandler;
    sigaction(SIGALRM, &sSigaction, NULL);
    THREAD_HANDLE threads[MAXTHREADS];

    for (int i = 0; i < NUM_TH; i++)
        CreateOneThread(&threads[i], InfiniteLoop, NULL);

    pid_t child = fork();

    if (child < 0)
    {
        fprintf(stderr, "FAILED: unable to create the child process\n");
        exit(RES_FORK_FAILED);
    }

    if (child > 0)
    {
        //in parent
        alarm(TIMEOUT); //send SIGALRM after TIMEOUT second.
        while (1)
        {
            if (waitpid(child, 0, 0) < 0)
            {
                // waitpid failed
                if (errno == EINTR)
                {
                    // failure was due to a signal
                    if (!alarmRinged) continue; // signal was not SIGALRM - continue to wait

                    // signal was SIGALRM - kill the child to avoid a hung test and exit
                    fprintf(stderr, "FAILED: the TIMEOUT has passed and the child process didn't terminate\n");
                    kill(child, 9);
                    exit(RES_TIMEOUT);
                }
                else
                {
                    // failure was due to an unexpected reason
                    fprintf(stderr, "FAILED: waitpid failed unexpectedly\n");
                    exit(RES_WAITPID_FAILED);
                }
            }
            else
            {
                // waitpid succeeded - child exited normally
                break;
            }
        }
    }

    if (child == 0)
    {
        // child does nothing
    }

    return RES_SUCCESS;
}
