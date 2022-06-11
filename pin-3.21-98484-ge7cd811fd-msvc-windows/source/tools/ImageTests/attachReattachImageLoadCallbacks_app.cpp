/*
 * Copyright (C) 2012-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdlib.h>
#include <dlfcn.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>

#define MAX_COMMAND_LINE_SIZE 15 // the size for the array of arguments to execv (this value is arbitrary)
#define EXPORT_SYM extern "C"

EXPORT_SYM bool AfterAttach1();

EXPORT_SYM bool AfterAttach2();

static int MAX_SIZE = 128; /*maximum line size*/

enum ExitType
{
    RES_SUCCESS = 0,     // 0
    RES_FORK_FAILED,     // 1
    RES_EXEC_FAILED,     // 2
    RES_LOAD_FAILED,     // 3
    RES_RES_INVALID_ARGS // 4
};

bool AfterAttach1()
{
    // Pin sets an anslysis function here to notify the application when Pin attaches to it.
    return false;
}

bool AfterAttach2()
{
    // Pin sets an anslysis function here to notify the application when Pin attaches to it.
    return false;
}

/*
 * block only the signals in the list: "signalsListToBlock"
 */
void BlockSignals(int signalsListToBlock[], int len, sigset_t* sigmask)
{
    sigemptyset(sigmask);
    int i;
    for (i = 0; i < len; ++i)
        sigaddset(sigmask, signalsListToBlock[i]);
    pthread_sigmask(SIG_SETMASK, sigmask, NULL);
}

/*
    Expected argv arguments:
    [1] first image to load
    [2] second image to load
    [3] pin executable
    [4] Pin flags (e.g. -slow_asserts)
        >> zero or more flags possible
    [5] "-probe"
    [6] "-t"
    [7] tool
    [8] output file
    [9] represent if SIGTRAP should be blocked by the application
    argv[9]=0 - SIGTRAP shouldn't be blocked
    argv[9]=1 - SIGTRAP should be blocked
*/

int main(int argc, char** argv)
{
    if (argc < 9)
    {
        fprintf(stderr, "No enough arguments\n");
        fflush(stderr);
        exit(RES_RES_INVALID_ARGS);
    }
    if (argc > MAX_COMMAND_LINE_SIZE)
    { // added: -pid attachPid -o NULL, omitted: argv[0..2], argv[9]
        fprintf(stderr, "Too many arguments\n");
        fflush(stderr);
        exit(RES_RES_INVALID_ARGS);
    }
    if (strcmp(argv[argc - 1], "1") == 0) // Need to block the SIGTRAP signal
    {
        int sigList[1] = {SIGTRAP};
        sigset_t sigmask;
        BlockSignals(sigList, 1, &sigmask);
    }

    pid_t parentPid = getpid();
    pid_t child     = fork();
    if (child < 0)
    {
        perror("fork failed while creating application process");
        exit(RES_FORK_FAILED);
    }

    if (child)
    {
        // inside parent
        while (!AfterAttach1())
        {
            sleep(1);
        }

        void* handle = dlopen(argv[1], RTLD_LAZY);
        if (!handle)
        {
            fprintf(stderr, " Failed to load: %s because: %s\n", argv[1], dlerror());
            fflush(stderr);
            exit(RES_LOAD_FAILED);
        }

        while (!AfterAttach2())
        {
            sleep(1);
        }

        handle = dlopen(argv[2], RTLD_LAZY);
        if (!handle)
        {
            fprintf(stderr, " Failed to load: %s because: %s\n", argv[2], dlerror());
            fflush(stderr);
            exit(RES_LOAD_FAILED);
        }

        while (1)
        {
            // expected to be stopped by tool.
            sleep(1);
        }
    }

    if (child == 0)
    {
        // inside child
        char attachPid[MAX_SIZE];
        snprintf(attachPid, MAX_SIZE, "%d", parentPid);
        char* args[MAX_COMMAND_LINE_SIZE] = {NULL}; // arguments for execv command
        int args_count                    = 0;
        int argv_count                    = 3;   // to start from argv[3]...
        args[args_count++] = argv[argv_count++]; // by convention, first arg is the filename of the executed file (pin)
        args[args_count++] = (char*)"-pid";
        args[args_count++] = attachPid;
        while (strcmp(argv[argv_count], "-t") != 0)
        {                                            // additional Pin flags (optional)
            args[args_count++] = argv[argv_count++]; // including "-probe" (mandatory for test)
        }
        args[args_count++] = argv[argv_count++]; // "-t"
        args[args_count++] = argv[argv_count++]; // tool
        args[args_count++] = (char*)"-o";
        args[args_count++] = argv[argv_count++]; // output file
        args[args_count++] = NULL;               // end
        execv(argv[3], (char* const*)args);      // never returns
        perror("execv failed while trying to attach Pin to the application\n");
        exit(RES_EXEC_FAILED);
    }
    return RES_SUCCESS;
}
