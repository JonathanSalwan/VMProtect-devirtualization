/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 *  Test attaching to an application that was launched from a directory different than the directory PIN is being launched.
 */

#include <assert.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string>
#include <list>
#include <sys/types.h>
#include <unistd.h>
#include <limits.h>
#include <sys/stat.h>
#include <sched.h>
using std::list;
using std::string;

string main_cwd;

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

    // Change cwd one level up
    chdir("..");

    pid_t child = fork();
    if (child)
    {
        // inside parent
        return;
    }
    else
    {
        // Change cwd back to the original cwd when this app was called
        chdir(main_cwd.c_str());

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
 * Expected command line: <this exe> -pin $PIN -pinarg <pin args > -t tool <tool args>
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
    // save cwd
    char cwd[PATH_MAX];
    getcwd(cwd, PATH_MAX);
    main_cwd = cwd;

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
