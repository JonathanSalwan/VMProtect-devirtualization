/*
 * Copyright (C) 2013-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
   Test the scenario where Pin tries to attach to an application, using the command line,
   when the main thread of the application is a zombie thread.
   Pin wouldn't attach to the application in this scenario.  
   Instead, Pin will give a message to the user notifying why Pin didn't attach to the application and the
   application will continue to run natively.
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <sys/syscall.h>
#include "zombie_utils.h"
#include <string.h>
#define MAX_COMMAND_LINE_SIZE 15 // the size for the array of arguments to execv (this value is arbitrary)

pid_t zombieThreadPid;

const char* fileName;

// This function is invoked when the secondary thread starts to execute.
void* SecondaryThreadMain(void* v)
{
    // Wait until Pin notifies the user that it can't attach to it since
    // the main thread of the application is a zombie thread.
    while (!NotifyUserPinUnableToAttach(fileName))
        sleep(1);
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
    pid_t first_child_pid;
    pid_t parentPid = getpid();
    fileName        = argv[argc - 1]; // argv[argc-1] is output file
    first_child_pid = fork();
    if (first_child_pid > 0)
    {
        //Inside parent 1
        pthread_t tid;
        pthread_create(&tid, NULL, SecondaryThreadMain, NULL);

        // After calling to thread_exit(0) function, the main thread is going to become a zombie thread.
        // Pin should not attach to the application.
        // Instead, Pin should give a message to the user notifing why Pin didn't attach to the
        // application and the application will continue to run natively.
        pthread_exit(0);
    }
    else
    {
        // Inside child 1
        pid_t second_child_pid = fork();
        if (second_child_pid > 0)
        {
            // Inside parent 2
            waitpid(second_child_pid, 0, 0);
            exit(RES_SUCCESS);
        }
        else
        {
            // Inside child 2
            char attachPid[MAX_SIZE];
            sprintf(attachPid, "%d", parentPid);

            // Wait until the main thread completes its execution and turns to zombie.
            while (!isZombie(parentPid))
                sleep(10);

            char* args[MAX_COMMAND_LINE_SIZE] = {NULL}; // arguments for execv command
            int args_count                    = 0;
            int argv_count                    = 1;   // to start from argv[1]...
            args[args_count++] = argv[argv_count++]; // by convention, first arg is the filename of the executed file (pin)
            args[args_count++] = (char*)"-probe";
            args[args_count++] = (char*)"-pid";
            args[args_count++] = attachPid;
            while (strcmp(argv[argv_count], "-t") != 0)
            { // additional Pin flags (optional)
                args[args_count++] = argv[argv_count++];
            }
            args[args_count++] = argv[argv_count++]; // "-t"
            args[args_count++] = argv[argv_count++]; // tool
            args[args_count++] = (char*)"-o";
            args[args_count++] = argv[argv_count++]; // output file
            args[args_count++] = NULL;               // end
            execv(argv[1], (char* const*)args);      // never returns
            perror("execv failed while trying to attach Pin to the application\n");
            exit(RES_EXEC_FAILED);
        }
    }

    return RES_SUCCESS;
}
