/*
 * Copyright (C) 2014-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*!
 * The purpose of this application, combined with the tool: "syscalls_and_locks_tool", is to check the correctness of the
 * lock acquisition algorithm enhancement, which designed to be clone-proof for probe mode
 *
 * This application verifies that a deadlock won't be encountered when the application calls to clone directly/indirectly,
 * while a different thread (in a different/same process) holds the lock (PIN_LOCK/PIN_RWMUTEX/client lock).
 */

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

#define TIMEOUT 120

#define STACK_SIZE (1024 * 1024) /* Stack size for cloned child */
#include <sys/syscall.h>

volatile bool thread1Init = false;

int syscall_type;

enum ExitType
{
    RES_SUCCESS = 0,     // 1
    RES_ALARM_TIMEOUT,   // 2
    RES_MALLOC_FAILED,   // 3
    RES_RES_INVALID_ARGS // 4
};

// Pin doesn't kill the process if the application encounters a deadlock, exit on SIGALRM.
void ExitOnAlarm(int sig)
{
    fprintf(stderr, "Timeout has passed, stuck in the system function library call, exit on SIGALRM\n");
    exit(RES_ALARM_TIMEOUT);
}

// The tool puts an analysis routine on this function to notify t1
// when the lock has been acquired.
extern "C" void WaitThread2AcquireLock()
{
    // do nothing
}

// The tool sets an analysis function here to notify t1 when t2
// has acquired and released the lock.
extern "C" void WaitUntilLockAcquiredAndReleased()
{
    // Do nothing
}

// When the child process is created with clone(), it executes this function.
int childFunc(void* arg)
{
    // Do nothing
    return 0;
}

void* thread_func1(void* arg)
{
    // Notify t2 that the lock should be acquired.
    thread1Init = true;

    // The tool puts an analysis routine on this function to notify t1
    // when the lock has been acquired.
    WaitThread2AcquireLock();

    // If syscall_type equals 1, the system function will be called
    // During this function call, the clone system call is called. We want to verify that
    // a deadlock won't be encountered while the lock is by t2.
    if (syscall_type == 1)
    {
        system("/bin/ls");
    }

    // If syscall_type equals 2 the popen function will be called. This function opens a process by creating a pipe,
    // forking and invoking a the shell. We want to verify that a deadlock won't be encounter when this function is being
    // called while the lock is held by t2.
    else if (syscall_type == 2)
    {
        FILE* pipe = popen("uname -r", "r");
        pclose(pipe);
    }

    // If syscall_type equals 3, the new thread will be created in a new thread group.
    // If syscall_type equals 4, the new thread will be in the same thread group as the caller and will share its
    // virtual memory.
    // We want to verify, in both of these scenarios, that a deadlock won't be encountered when the clone system call
    // is being called while the lock is held  by t2.
    else if (syscall_type == 3 || syscall_type == 4)
    {
        char* stack;
        char* stackTop;
        stack = (char*)malloc(STACK_SIZE);
        if (stack == NULL) exit(RES_MALLOC_FAILED);
        stackTop = stack + STACK_SIZE;

        if (3 == syscall_type)
        {
            clone(childFunc, stackTop, 0, 0);
        }
        else if (4 == syscall_type)
        {
            clone(childFunc, stackTop, CLONE_VM | CLONE_SIGHAND | CLONE_THREAD, 0);
        }

        // Sleep for 30 seconds to increase the probability that the new process which has been created by clone
        // will finish its execution.
        sleep(30);
    }

    return 0;
}

void* thread_func2(void* arg)
{
    // Wait until t1 starts to run.
    while (!thread1Init)
        sched_yield();

    // Wait until the lock is acquired and released.
    WaitUntilLockAcquiredAndReleased();

    return 0;
}

//  Expected argv arguments:
//  [1] syscall type
//    1 - execute the system function.
//    2 - execute the popen function.
//    3 - execute the clone system call where the parent of the new child will be different
//        from that of the calling process.
//    4 - execute the clone system call where the parent of the new child will be the same as
//        that of the calling process.
int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        fflush(stderr);
        exit(RES_RES_INVALID_ARGS);
    }

    syscall_type = atoi(argv[1]);

    pthread_t t1;
    pthread_t t2;

    pthread_create(&t1, 0, thread_func1, 0);
    pthread_create(&t2, 0, thread_func2, 0);

    //Exit in 120 sec
    signal(SIGALRM, ExitOnAlarm);
    alarm(TIMEOUT);

    pthread_join(t1, 0);
    pthread_join(t2, 0);

    printf("All threads exited. The test PASSED\n");
    return RES_SUCCESS;
}
