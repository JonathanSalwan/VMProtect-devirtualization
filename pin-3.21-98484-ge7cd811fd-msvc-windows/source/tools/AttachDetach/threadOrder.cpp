/*
 * Copyright (C) 2011-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <pthread.h>
#include <cstdio>
#include <cassert>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#define MAX_COMMAND_LINE_SIZE 15 // the size for the array of arguments to execv (this value is arbitrary)

const int READ  = 0;
const int WRITE = 1;
int appToTool[2];

void* doSecondaryThread(void* dummy)
{
    close(appToTool[WRITE]); // This will release the parent and start Pin.
    sleep(5 * 60);           // Wait here for program termination.
    return NULL;
}

int doMainAppThread()
{
    pthread_t tid;
    pthread_create(&tid, 0, doSecondaryThread, 0);
    pthread_join(tid, 0); // Wait here for program termination.
    return ETIME;         // If we got here (instead of being terminated by Pin) this means we timed out.
}

/*
 * Expected argv arguments:
 * [1] pin executable
 * [2] Pin flags (e.g. -slow_asserts)
 *     >> zero or more flags possible
 * [3] "-t"
 * [4] tool
 */
int main(int argc, char* argv[])
{
    if (argc > MAX_COMMAND_LINE_SIZE - 2)
    { // added: -pid attachPid NULL
        fprintf(stderr, "Too many arguments\n");
        fflush(stderr);
        exit(1);
    }

    if (pipe(appToTool) != 0)
    {
        fprintf(stderr, "pipe creation failed...\n");
        return -1;
    }

    pid_t parentPid = getpid();

    pid_t childPid = fork();
    if (childPid < 0)
    {
        fprintf(stderr, "fork failed...\n");
        return -1;
    }

    // Parent's code - runs the application.
    if (childPid != 0)
    {
        close(appToTool[READ]);
        doMainAppThread(); // never returns
        return -1;
    }
    // Child's code - attaches pin and the tool to the application.
    else
    {
        close(appToTool[WRITE]);
        char buf[2];
        read(appToTool[READ], buf, 1); // Wait here until both threads of the app have been created.
        close(appToTool[READ]);
        char parentPidStr[24]; // Long enough to hold at least 64 bits of integer as a string.
        sprintf(parentPidStr, "%d", parentPid);
        char* args[MAX_COMMAND_LINE_SIZE] = {NULL}; // arguments for execv command
        int args_count                    = 0;
        int argv_count                    = 1;   // to start from argv[1]...
        args[args_count++] = argv[argv_count++]; // by convention, first arg is the filename of the executed file (pin)
        args[args_count++] = (char*)"-pid";
        args[args_count++] = parentPidStr;
        while (strcmp(argv[argv_count], "-t") != 0)
        { // additional Pin flags (optional)
            args[args_count++] = argv[argv_count++];
        }
        args[args_count++] = argv[argv_count++]; // "-t"
        args[args_count++] = argv[argv_count++]; // tool
        args[args_count++] = NULL;               // end
        // Pin  attaches to the application.
        execv(argv[1], (char* const*)args); // never returns
        fprintf(stderr, "execv failed with errno: %d\n", errno);
        kill(parentPid, 9);
        return -1;
    }
}
