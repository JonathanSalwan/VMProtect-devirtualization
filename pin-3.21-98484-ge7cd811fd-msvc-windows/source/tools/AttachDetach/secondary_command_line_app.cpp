/*
 * Copyright (C) 2013-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
   Test the scenario where Pin attaches to an application, using the command line,
   which one of its secondary thread is a zombie thread
   Pin wouldn't attach the zombie thread and wouldn't give a thread detach callbacks
   on detach to the zombie thread.
*/

#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>
#include <assert.h>
#include <stdlib.h>
#include <sched.h>
#include <string.h>
#include <iostream>
#include "zombie_utils.h"

#define MAX_COMMAND_LINE_SIZE 15 // the size for the array of arguments to execv (this value is arbitrary)
static int pipefd[2];

void* SecondaryThreadMain(void* v)
{
    close(pipefd[0]); // Close child's read end
    write(pipefd[1], const_cast< char* >("NotifyParent"), strlen(const_cast< char* >("NotifyParent")));
    close(pipefd[1]); // Close parent's write end
    pthread_exit(0);
    return NULL;
}

// Expected argv arguments:
// [1] pin executable
// [2] Pin flags (e.g. -slow_asserts)
//     >> zero or more flags possible
// [3] "-t"
// [4] tool
// [5] output file
int main(int argc, char* argv[])
{
    if (argc < 5)
    {
        fprintf(stderr, "Not enough arguments\n");
        fflush(stderr);
        exit(RES_INVALID_ARGS);
    }
    if (argc > MAX_COMMAND_LINE_SIZE - 4)
    { // added: -pid attachPid -probe -o NULL
        fprintf(stderr, "Too many arguments\n");
        fflush(stderr);
        exit(RES_INVALID_ARGS);
    }
    if (pipe(pipefd) == -1)
    {
        fprintf(stderr, "Pipe Failed.\n");
        return RES_PIPE_CREATION_ERROR;
    }

    // The pipe is used to transfer information from the
    // child process to the secondary thread.
    pid_t child_pid;
    pid_t parentPid = getpid();

    child_pid = fork();

    if (child_pid > 0)
    {
        pthread_t tid;
        pthread_create(&tid, NULL, SecondaryThreadMain, NULL);
        while (1)
            sleep(1);
    }
    else
    {
        // In child
        char attachPid[MAX_SIZE];
        snprintf(attachPid, MAX_SIZE, "%d", parentPid);
        close(pipefd[1]); // Close parent's write end
        int buf[2];
        if (read(pipefd[0], buf, 1) < 0)
        {
            exit(RES_PIPE_ERROR);
        }

        close(pipefd[0]); // Close parent's write end.

        char* args[MAX_COMMAND_LINE_SIZE] = {NULL}; // arguments for execv command
        int args_count                    = 0;
        int argv_count                    = 1;   // to start from argv[1]...
        args[args_count++] = argv[argv_count++]; // by convention, first arg is the filename of the executed file (pin)
        args[args_count++] = (char*)"-pid";
        args[args_count++] = attachPid;
        args[args_count++] = (char*)"-probe";
        while (strcmp(argv[argv_count], "-t") != 0)
        { // additional Pin flags (optional)
            args[args_count++] = argv[argv_count++];
        }
        args[args_count++] = argv[argv_count++]; // "-t"
        args[args_count++] = argv[argv_count++]; // tool
        args[args_count++] = (char*)"-o";
        args[args_count++] = argv[argv_count++]; // output file
        args[args_count++] = NULL;               // end
        // Pin attaches to the application.
        execv(argv[1], (char* const*)args); // never returns
        exit(RES_EXEC_FAILED);
    }

    return RES_SUCCESS;
}
