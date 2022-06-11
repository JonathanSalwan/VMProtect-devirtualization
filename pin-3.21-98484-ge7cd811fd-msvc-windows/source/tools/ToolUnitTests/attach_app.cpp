/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/* ===================================================================== */
/*! @file
 *
 * A test for attach to single threaded application.
 * The application launches Pin and waits until Pin attaches
 */

#include <stdio.h>
#include <unistd.h>
#include <sched.h>
#include <signal.h>
#include <stdlib.h>
#include <string>
#include <list>
#include <assert.h>
using std::list;
using std::string;

/* Pin doesn't kill the process if if failed to attach, exit on SIGALRM */
void ExitOnAlarm(int sig)
{
    fprintf(stderr, "Pin is not attached, exit on SIGALRM\n");
    exit(0);
}

extern "C" int PinAttached() { return 0; }

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

void StartPin(list< string >* pinArgs)
{
    pid_t appPid = getpid();
    pid_t child  = fork();
    if (child) return;

    // start Pin from child
    char** inArgv = new char*[pinArgs->size() + 10];

    // Pin binary in the first
    list< string >::iterator pinArgIt = pinArgs->begin();
    string pinBinary                  = *pinArgIt;
    pinArgIt++;

    // build pin arguments:
    // pin -pid appPid -mt 0 [pinarg]
    unsigned int idx = 0;
    inArgv[idx++]    = (char*)pinBinary.c_str();
    inArgv[idx++]    = (char*)"-pid";
    inArgv[idx]      = (char*)malloc(10);
    sprintf(inArgv[idx++], "%d", appPid);

    inArgv[idx++] = (char*)"-mt";
    inArgv[idx++] = (char*)"0";

    for (; pinArgIt != pinArgs->end(); pinArgIt++)
    {
        inArgv[idx++] = (char*)pinArgIt->c_str();
    }
    inArgv[idx] = 0;

    PrintArguments(inArgv);

    execvp(inArgv[0], inArgv);
    fprintf(stderr, "ERROR: execv %s failed\n", inArgv[0]);

    exit(1);
}

int main(int argc, char* argv[])
{
    int i;

    list< string > pinArgs;
    ParseCommandLine(argc, argv, &pinArgs);

    StartPin(&pinArgs);

    /* Exit in 20 sec */
    signal(SIGALRM, ExitOnAlarm);
    alarm(20);

    printf("Before pause\n");
    while (!PinAttached())
    {
        sched_yield();
        printf(".");
        sleep(2);
    }
    printf("After pause\n");

    return 0;
}
