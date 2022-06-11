/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 *  Test pause_tool messages when attaching to Linux process  
 *  The application tests that pause_tool messages appears on INJECTOR stdout  
 */

#include <assert.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string>
#include <list>
#include <sched.h>
#include <sys/types.h>
#include <unistd.h>
using std::list;
using std::string;

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

    int stdoutDupFd = dup(STDOUT_FILENO); // duplicating stdout to new file descriptor
    close(STDOUT_FILENO);                 // closing stdout

    pid_t child = fork();
    if (child)
    {
        // INJECTEE is running with its stdout closed
        return;
    }
    else
    {
        // inside child

        // open stdout using the application original output
        dup2(stdoutDupFd, STDOUT_FILENO);

        // INJECTOR is running with the original stdout

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

        // Activate pin INJECTOR
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

extern "C" int MainThreadReady(void) { return 0; }

int main(int argc, char* argv[])
{
    list< string > pinArgs;
    ParseCommandLine(argc, argv, &pinArgs);

    AttachAndInstrument(&pinArgs);

    // Wait for pin to attach
    while (!MainThreadReady())
    {
        sched_yield();
    }

    return 0;
}
