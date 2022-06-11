/*
 * Copyright (C) 2010-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * This test application creates 2 processes:
 *   Parent process: Just to monitor whether child process got into deadlock or not.
 *   Child process: create 2 threads:
 *        Thread A (main thread): 
 *          1. Set a timer to signal this thread periodically.
 *          2. Call in a loop to DoToolAnalysis function that will be used as a slot to insert analysis func
 *        Thread B
 *          1. Call the sched_yield() system call in a loop.
 *
 * The tool will use PIN_AddSyscallEntryFunction(OnSyscall, 0) to add instumentation func that holds and releases local lock (on Thread B)
 * It will also add an instrumentation func on thread A (before DoToolAnalysis) that will get the same local lock and do a long busy loop before releasing the lock.
 * 
 * The expected scenario is that: 
 *    1. Thread A will take the local lock and wait on long busy loop
 *    2. Thread B will wakeup and will take VM lock (due to the sys call) 
 *    3. Thraed B will try to take local lock (call-back) and fail since thread A holds the lock.
 *    4. An async timer signal on thread A will cause it to get VM lock ---DEADLOCK---
 */

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <pthread.h>
#include <sched.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#define COUNT 10
#define USLEEP_TIME 25000

#define DEADLOCK_TIMEOUT 10

typedef void (*PF)();

volatile unsigned SigCount = 0;

static void Handle(int);
static void TimeoutHandler(int);
static void* DoSysCallTillSignalsDone(void*);
void DoToolAnalysis();
void sig_hand(int);

int main()
{
    sigset_t sigset;
    sigemptyset(&sigset);
    sigaddset(&sigset, SIGCHLD);
    if (sigprocmask(SIG_BLOCK, &sigset, NULL) < 0)
    {
        fprintf(stderr, "Unable to mask SIGCHLD");
        return 1;
    }
    signal(SIGCHLD, sig_hand);

    int fd[2];
    if (pipe(fd) == -1)
    {
        fprintf(stderr, "Unable to open pipe ");
        return 1;
    }
    pid_t child = fork();
    if (child)
    {
        // Parent proc
        close(fd[1]); // Close unused write end

        struct timespec timeout;
        timeout.tv_sec  = 1200; //20 mins
        timeout.tv_nsec = 0;
        char buf[2];

        read(fd[0], &buf, 1); // wait until child activate timer
        close(fd[0]);

        // This process waits enough for the child process to complete
        if (sigtimedwait(&sigset, NULL, &timeout) < 0)
        {
            if (errno == EAGAIN)
            {
                fprintf(stderr, "Timeout, killing child\n");
                kill(child, SIGKILL);
                return 1;
            }
            else
            {
                fprintf(stderr, "Error in sigtimedwait");
                return 1;
            }
        }
        int status;

        // In case child process is still running the test fails (assuming deadlock occurred)
        if (waitpid(child, &status, 0) < 0)
        {
            fprintf(stderr, "Error on waitpid");
            return 1;
        }
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
        {
            fprintf(stderr, "Parent - terminate OK\n");
            return 0;
        }
        return 1;
    }
    else
    {
        // inside child
        close(fd[0]); // Close unused read end
        struct sigaction sigact;
        struct itimerval itval;
        pthread_t tid;

        sigact.sa_handler = Handle;
        sigact.sa_flags   = 0;
        sigemptyset(&sigact.sa_mask);

        if (sigaction(SIGVTALRM, &sigact, 0) == -1)
        {
            fprintf(stderr, "Child - Unable to set up handler\n");
            return 1;
        }

        struct sigaction sigact_timeout;
        sigact_timeout.sa_handler = TimeoutHandler;
        sigact_timeout.sa_flags   = 0;
        sigfillset(&sigact_timeout.sa_mask);

        if (sigaction(SIGALRM, &sigact_timeout, 0) == -1)
        {
            fprintf(stderr, "Child - Unable to set up timeout handler\n");
            return 1;
        }

        // Create a new thread that calls a system call in a loop
        if (pthread_create(&tid, 0, DoSysCallTillSignalsDone, 0) != 0)
        {
            fprintf(stderr, "Child - Unable to create thread\n");
            return 1;
        }

        // Set timer to create an async signals
        itval.it_interval.tv_sec  = 0;
        itval.it_interval.tv_usec = USLEEP_TIME;
        itval.it_value.tv_sec     = 0;
        itval.it_value.tv_usec    = USLEEP_TIME;
        if (setitimer(ITIMER_VIRTUAL, &itval, 0) == -1)
        {
            fprintf(stderr, "Child - Unable to set up timer\n");
            return 1;
        }

        close(fd[1]); // send EOF to signal parent that timer is on

        alarm(DEADLOCK_TIMEOUT);

        /*
        * Call an analysis function in a loop .
        */
        while (SigCount < COUNT)
        {
            volatile PF doToolAnalysis = DoToolAnalysis;
            doToolAnalysis();
        }

        itval.it_value.tv_sec  = 0;
        itval.it_value.tv_usec = 0;
        if (setitimer(ITIMER_VIRTUAL, &itval, 0) == -1)
        {
            fprintf(stderr, "Child - Unable to disable timer\n");
            return 1;
        }

        pthread_join(tid, 0);
        fprintf(stderr, "Child - terminate OK\n");
        return 0;
    }
}

void sig_hand(int a) {};

static void Handle(int sig)
{
    SigCount++;
    printf("Signal Count: %u\n", SigCount);
    alarm(DEADLOCK_TIMEOUT);
}

static void TimeoutHandler(int a)
{
    printf(
        "Deadlock timeout occured, it seems that PIN couldn't get the app and tool out of a deadlock situation. SigCount: %u\n",
        SigCount);
    exit(1);
}

static void* DoSysCallTillSignalsDone(void* v)
{
    /*
     * Call a system call in the loop .
     */
    while (SigCount < COUNT)
    {
        sched_yield();
    }
    return 0;
}

void DoToolAnalysis()
{
    /* Pin tool places an instrumentation point here */

    /*
     * Yield the processor to give the child thread a chance to run.
     * Otherwise, there is a danger that other thread will starve while
     * trying to acquire the PIN_LOCK in the tool.
     */
    sched_yield();
}
