/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 * Test PIN attach - detach for MT application
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

extern "C" int ThreadHoldByPin() { return 0; }

bool threadHoldByApplication = true;
;

/*
 * An endless-loop function for secondary threads
 */

void* ThreadEndlessLoopFunc(void* arg)
{
    // thread should be inside this loop until
    // Pin and application free it
    // 1. App frees thread when Pin attach is completed
    // 2. Pin frees thread when Pin detach is completed
    while (ThreadHoldByPin() || threadHoldByApplication)
    {
        sched_yield();
    }

    fprintf(stderr, "Thread %d runs native\n", GetTid());

    return 0;
}

// Print pin invocation command line
void PrintArguments(char** inArgv)
{
    fprintf(stderr, "Going to run: ");
    for (unsigned int i = 0; inArgv[i] != 0; ++i)
    {
        fprintf(stderr, "%s ", inArgv[i]);
    }
    fprintf(stderr, "\n");
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

/* This function should be replaced by Pin tool */
extern "C" void DetachPin()
{
    fprintf(stderr, "This function shouldn't be called form application\n");
    assert(false);
    return;
}

/* This function should be replaced by Pin tool.
   The Pin-replaced ThreadsReady returns true when Pin is attached
   and each thread sent notification.
 */
extern "C" int ThreadsReady(int numOfThreads) { return 0; }

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

int main(int argc, char* argv[])
{
    list< string > pinArgs;
    ParseCommandLine(argc, argv, &pinArgs);

    pthread_t* thHandle;
    thHandle = new pthread_t[numOfSecondaryThreads];

    threadHoldByApplication = true;
    // start all secondary threads
    for (unsigned int i = 0; i < numOfSecondaryThreads; i++)
    {
        pthread_create(&thHandle[i], 0, ThreadEndlessLoopFunc, (void*)i);
    }

    // Attach Pin
    AttachAndInstrument(&pinArgs);

    // Wait for full attach
    while (!ThreadsReady(numOfSecondaryThreads + 1))
    {
        sched_yield();
    }

    // Application does not hold threads any more
    threadHoldByApplication = false;

    // After Pin detach all threads should exit
    printf("App is ready for Pin detach\n");
    DetachPin();

    for (unsigned int i = 0; i < numOfSecondaryThreads; i++)
    {
        pthread_join(thHandle[i], 0);
    }

    return 0;
}
