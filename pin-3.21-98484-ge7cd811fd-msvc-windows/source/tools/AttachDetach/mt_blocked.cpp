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
#include "../Utils/threadlib.h"
using std::list;
using std::string;

/* 
 * The total number of threads that should run in this process
 * The number may be changed in command line with -th_num
 */
unsigned int numOfSecondaryThreads = 4;

/*
 * An write-read function for secondary threads
 */
extern "C" int ThreadsReady(unsigned int numOfThreads)
{
    assert(numOfThreads == numOfSecondaryThreads + 1);
    return 0;
}

void* ThreadEndlessLoopFunc(void* arg)
{
    pthread_mutex_t* mutex = (pthread_mutex_t*)arg;

    while (!ThreadsReady(numOfSecondaryThreads + 1))
    {
        pthread_mutex_lock(mutex);
        sched_yield();
        pthread_mutex_unlock(mutex);
    }
    fprintf(stderr, "Thread %u leaving the loop\n", GetTid());

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
void AttachAndInstrument(list< string >* pinArgs)
{
    list< string >::iterator pinArgIt = pinArgs->begin();

    string pinBinary = *pinArgIt;
    pinArgIt++;

    pid_t parent_pid = getpid();

    pid_t child = fork();

    if (child)
    {
        fprintf(stderr, "Pin injector pid %d\n", child);
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
        return;
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
        if (arg == "-pin")
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

pthread_t* thHandle;

int main(int argc, char* argv[])
{
    list< string > pinArgs;
    ParseCommandLine(argc, argv, &pinArgs);

    thHandle = new pthread_t[numOfSecondaryThreads];

    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    // start all secondary threads
    for (unsigned int i = 0; i < numOfSecondaryThreads; i++)
    {
        pthread_create(&thHandle[i], 0, ThreadEndlessLoopFunc, (void*)&mutex);
    }

    AttachAndInstrument(&pinArgs);

    for (unsigned int i = 0; i < numOfSecondaryThreads; i++)
    {
        pthread_join(thHandle[i], 0);
    }

    return 0;
}
