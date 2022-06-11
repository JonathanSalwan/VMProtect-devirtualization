/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <dlfcn.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string>
#include <list>
#include <sys/syscall.h>
#include <sched.h>
#include <errno.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
using std::list;
using std::string;

bool shouldCancelThreads = true;
void CancelAllThreads();
void BlockSignal(int sigNo);
void UnblockSignal(int sigNo);

/* 
 * The total number of threads that should run in this process
 * The number may be changed in command line with -th_num
 */
unsigned int numOfSecondaryThreads = 4;

/*
 * try to attach pin twice to the same process
 */
bool attachTwice   = false;
bool endlessSelect = false;

void UnblockAllSignals()
{
    sigset_t mask;
    sigemptyset(&mask);
    sigprocmask(SIG_SETMASK, &mask, 0);
}

/*
 * A signal handler for canceling all threads
 */
void SigUsr1Handler(int sig)
{
    if (shouldCancelThreads)
    {
        fprintf(stderr, "Cancel all threads\n");
        CancelAllThreads();
        shouldCancelThreads = false;
    }
}

/*
 * An endless-loop function for secondary threads
 */

void* ThreadEndlessLoopFunc(void* arg)
{
    int x = 0;

    //Allow asynchronious cancelation of this thread
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

    while (1)
    {
        x++;
        if (x > 10)
        {
            x = 0;
        }
    }

    return 0;
}

void* ThreadEndlessSelectFunc(void* arg)
{
    int sock[2];
    int err = socketpair(PF_LOCAL, SOCK_DGRAM, 0, sock);
    assert(err >= 0);
    fd_set fd_read;

    FD_ZERO(&fd_read);
    FD_SET(sock[1], &fd_read);

    do
    {
        err = select(sock[1] + 1, &fd_read, NULL, NULL, NULL);
        // We expect select() to fail since nobody writes to the write side of the socket
        assert(-1 == err);
    }
    while (err < 0 && errno == EINTR);
    return 0;
}

#define DECSTR(buf, num)         \
    {                            \
        buf = (char*)malloc(10); \
        sprintf(buf, "%d", num); \
    }

inline void PrintArguments(char** inArgv)
{
    fprintf(stderr, "Going to run: ");
    for (unsigned int i = 0; inArgv[i] != 0; ++i)
    {
        fprintf(stderr, "%s ", inArgv[i]);
    }
    fprintf(stderr, "\n");
}

/* AttachAndInstrument()
 * a special routine that runs $PIN
 */
pid_t AttachAndInstrument(list< string >* pinArgs)
{
    list< string >::iterator pinArgIt = pinArgs->begin();

    string pinBinary = *pinArgIt;
    pinArgIt++;

    pid_t parent_pid = getpid();

    pid_t child = fork();

    assert(child >= 0);
    if (child)
    {
        fprintf(stderr, "Pin injector pid %d\n", child);
        // inside parent
        return child;
    }
    else
    {
        // inside child

        UnblockAllSignals();
        char** inArgv = new char*[pinArgs->size() + 10];

        unsigned int idx = 0;
        inArgv[idx++]    = (char*)pinBinary.c_str();
        inArgv[idx++]    = (char*)"-pid";
        inArgv[idx]      = (char*)malloc(10);
        sprintf(inArgv[idx++], "%d", parent_pid);

        for (; pinArgIt != pinArgs->end(); pinArgIt++)
        {
            inArgv[idx++] = (char*)pinArgIt->c_str();
        }
        inArgv[idx] = 0;

        PrintArguments(inArgv);

        execvp(inArgv[0], inArgv);
        fprintf(stderr, "ERROR: execv %s failed\n", inArgv[0]);
        kill(parent_pid, 9);
        return 0;
    }
}

/*
 * Invoke external process that will send signals
 */
void SendSignals(int signo)
{
    pid_t parentPid = getpid();
    pid_t pid       = fork();
    if (pid)
    {
        fprintf(stderr, "Send signals pid %d\n", pid);
        waitpid(pid, NULL, 0);
        // inside parent
        return;
    }
    if (pid == 0) // child
    {
        char** inArgv = new char*[15];

        unsigned int idx = 0;
        inArgv[idx++]    = (char*)"./send_signals.sh";
        DECSTR(inArgv[idx], parentPid);
        idx++;
        DECSTR(inArgv[idx], signo);
        idx++;
        inArgv[idx] = 0;

        PrintArguments(inArgv);

        execvp(inArgv[0], inArgv);
        fprintf(stderr, "ERROR: execv %s failed\n", inArgv[0]);
    }
}

/*
 * Expected command line: <this exe> [-th_num NUM] -pin $PIN -pinarg <pin args > -t tool <tool args>
 */

void ParseCommandLine(int argc, char* argv[], list< string >* pinArgs)
{
    string pinBinary;
    for (int i = 1; i < argc; i++)
    {
        string arg = string(argv[i]);
        if (arg == "-th_num")
        {
            numOfSecondaryThreads = atoi(argv[++i]) - 1;
        }
        else if (arg == "-pin")
        {
            pinBinary = argv[++i];
        }
        else if (arg == "-pinarg")
        {
            for (int parg = ++i; parg < argc; parg++)
            {
                pinArgs->push_back(string(argv[parg]));
                ++i;
            }
        }
        else if (arg == "-attach_twice")
        {
            attachTwice = true;
        }
        else if (arg == "-keep_threads")
        {
            shouldCancelThreads = false;
        }
        else if (arg == "-endless_select")
        {
            endlessSelect = true;
        }
    }
    assert(!pinBinary.empty());
    pinArgs->push_front(pinBinary);
}

pthread_t* thHandle;

extern "C" int ThreadsReady(unsigned int numOfThreads)
{
    assert(numOfThreads == numOfSecondaryThreads + 1);
    return 0;
}

int main(int argc, char* argv[])
{
    list< string > pinArgs;
    int status = 0;
    int retval = 0;
    ParseCommandLine(argc, argv, &pinArgs);

    signal(SIGUSR1, SigUsr1Handler);

    thHandle = new pthread_t[numOfSecondaryThreads];

    // start all secondary threads
    // in the secondary threads SIGUSR1 should be blocked
    BlockSignal(SIGUSR1);
    for (intptr_t i = 0; i < numOfSecondaryThreads; i++)
    {
        retval = pthread_create(&thHandle[i], 0, endlessSelect ? ThreadEndlessSelectFunc : ThreadEndlessLoopFunc, (void*)i);
        assert(retval == 0);
    }
    UnblockSignal(SIGUSR1);

    sigset_t sigMask, sigMaskAfterAttach;
    // Just to be on the safe side, reset to all zeros.
    memset(&sigMask, 0, sizeof(sigMask));
    memset(&sigMaskAfterAttach, 0, sizeof(sigMaskAfterAttach));
    retval = sigemptyset(&sigMask);
    assert(retval == 0);
    retval = sigemptyset(&sigMaskAfterAttach);
    assert(retval == 0);

    //unsigned char* p = (unsigned char*)&sigMask;
    //for (int i=0; i<(int)sizeof(sigset_t); i++){cerr << dec << i << "\t" << (int)p[i] << endl;}

    // If we block SIGTRAP then Pin attach might unblock it - see Mantis #3879
#ifndef TARGET_LINUX
    retval = sigaddset(&sigMask, SIGTRAP);
    assert(retval == 0);
#endif
    retval = sigaddset(&sigMask, SIGHUP);
    assert(retval == 0);
    retval = sigaddset(&sigMask, SIGQUIT);
    assert(retval == 0);

    retval = pthread_sigmask(SIG_SETMASK, &sigMask, NULL);
    assert(retval == 0);

    pid_t pinInjectorPid = AttachAndInstrument(&pinArgs);
    while (pinInjectorPid != waitpid(pinInjectorPid, &status, 0))
    {
        assert(errno == EINTR);
    }
    if (!WIFEXITED(status))
    {
        printf("ERROR: Pin injector exited abnormally: %x\n", status);
        exit(-1);
    }
    if (WEXITSTATUS(status) != 0)
    {
        printf("ERROR: Pin injector exited with nonzero exit code: %d\n", WEXITSTATUS(status));
        exit(-1);
    }

    // Give enough time for all threads to get started
    while (!ThreadsReady(numOfSecondaryThreads + 1))
    {
        sched_yield();
    }

    // Check that the signal mask was not changed due to Pin attach
    retval = pthread_sigmask(SIG_SETMASK, NULL, &sigMaskAfterAttach);
    assert(retval == 0);
    assert(0 == memcmp(&sigMask, &sigMaskAfterAttach, sizeof(sigMask)));

    if (attachTwice)
    {
        pinInjectorPid = AttachAndInstrument(&pinArgs);
        while (pinInjectorPid != waitpid(pinInjectorPid, &status, 0))
        {
            assert(errno == EINTR);
        }
        if (!WIFEXITED(status))
        {
            printf("ERROR: Pin injector exited abnormally in second attach: %x\n", status);
            exit(-1);
        }
        if (WEXITSTATUS(status) == 0)
        {
            printf("ERROR: Pin was injected twice to the same process\n");
            exit(-1);
        }
        printf("Second attach exited with status %d\n", WEXITSTATUS(status));
    }

    fprintf(stderr, "Sending SIGUSR1\n");
    SendSignals(SIGUSR1);

    while (shouldCancelThreads)
    {
        sched_yield();
    }
    fprintf(stderr, "%s: exiting...\n", argv[0]);

    return 0;
}

void CancelAllThreads()
{
    for (unsigned int i = 0; i < numOfSecondaryThreads; i++)
    {
        int retval = pthread_cancel(thHandle[i]);
        assert(retval == 0);
    }
}

void BlockSignal(int sigNo)
{
    sigset_t mask;
    int retval;

    retval = sigemptyset(&mask);
    assert(retval == 0);

    retval = sigaddset(&mask, sigNo);
    assert(retval == 0);

    retval = sigprocmask(SIG_BLOCK, &mask, 0);
    assert(retval == 0);
}
void UnblockSignal(int sigNo)
{
    sigset_t mask;
    int retval;

    retval = sigemptyset(&mask);
    assert(retval == 0);

    retval = sigaddset(&mask, sigNo);
    assert(retval == 0);

    retval = sigprocmask(SIG_UNBLOCK, &mask, 0);
    assert(retval == 0);
}
