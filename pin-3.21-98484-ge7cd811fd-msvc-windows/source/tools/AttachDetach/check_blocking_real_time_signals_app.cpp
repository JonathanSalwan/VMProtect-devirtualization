/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <assert.h>
#include <stdio.h>
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
using std::list;
using std::string;

/* Assuming that 40 is real time signal in both glibc and pincrt */
#define REALTIMESIGNAL 40

/* 
 * The total number of threads that should run in this process
 * The number may be changed in command line with -th_num
 */
unsigned int numOfSecondaryThreads = 4;

void* Do_Nothing(void* input)
{
    int x = 0;
    while (1)
    {
        x++;
        --x;
    }
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
 * 
 */
void AttachAndInstrument(list< string >* pinArgs)
{
    list< string >::iterator pinArgIt = pinArgs->begin();

    string pinBinary = *pinArgIt;
    pinArgIt++;

    pid_t parent_pid = getpid();

    pid_t child = fork();

    if (child)
    {
        // inside parent
        return;
    }
    else
    {
        // inside child

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
    }
    assert(!pinBinary.empty());
    pinArgs->push_front(pinBinary);
}

extern "C" int ThreadsReady(unsigned int numOfThreads) { return 0; }

int main(int argc, char* argv[])
{
    list< string > pinArgs;
    ParseCommandLine(argc, argv, &pinArgs);

    pthread_t* thHandle = new pthread_t[numOfSecondaryThreads + 1];

    sigset_t set;
    if (sigemptyset(&set))
    {
        fprintf(stderr, "ERROR: sigemptysit failed\n");
        return -1;
    }
    if (sigaddset(&set, REALTIMESIGNAL))
    {
        fprintf(stderr, "ERROR: sigaddset failed (%d)\n", REALTIMESIGNAL);
        return -1;
    }
    if (pthread_sigmask(SIG_BLOCK, &set, NULL))
    {
        fprintf(stderr, "ERROR: pthread_sigmask failed\n");
        return -1;
    }

    // start all secondary threads
    unsigned long numOfRunningThreads = 1; // the main thread
    for (unsigned long i = 0; i < numOfSecondaryThreads; ++i)
    {
        pthread_create(&thHandle[i], 0, Do_Nothing, NULL);
        ++numOfRunningThreads;
    }

    AttachAndInstrument(&pinArgs);

    // Give enough time for all threads to get started
    while (!ThreadsReady(numOfRunningThreads))
    {
        sched_yield();
    }

    return 0;
}
