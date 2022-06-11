/*
 * Copyright (C) 2013-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
   Test the scenario when Pin attaches to an application, using PIN_AttachProbed API,
   which one of its secondary thread is a zombie thread .
   Pin wouldn't attach to the zombie thread and wouldn't give a thread detach callbacks on detach
   to the zombie thread.
*/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <pthread.h>
#include <sstream>
#include <fstream>
#include <sys/wait.h>
#include "zombie_utils.h"
#include <string.h>
#define MAX_COMMAND_LINE_SIZE 15 // the size for the array of arguments to execv (this value is arbitrary)

EXPORT_SYM bool AfterAttach1();

const char* imageToLoad;

pid_t zombieThreadPid;

pid_t parentPid;

bool AfterAttach1()
{
    // Pin sets an analysis function here to notify the application when Pin attaches to it.
    return false;
}

// This function is invoked when the secondary thread starts to execute.
void* SecondaryThreadMain(void* v)
{
    void* handle = dlopen(imageToLoad, RTLD_LAZY);

    if (!handle)
    {
        fprintf(stderr, " Failed to load: %s because: %s\n", imageToLoad, dlerror());
        fflush(stderr);
        exit(RES_LOAD_FAILED);
    }

    pthread_exit(0);
    return NULL;
}

// Expected argv arguments:
// [1] pin executable
// [2] Pin flags (e.g. -slow_asserts)
//     >> zero or more flags possible
// [3] "-t"
// [4] tool
// [5] imageName
// [6] output file

int main(int argc, char** argv)
{
    if (argc < 6)
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
    imageToLoad = argv[argc - 2]; // argv[argc-2] is imageName
    parentPid   = getpid();
    pid_t child = fork();
    if (child < 0)
    {
        perror("Fork failed while creating application process");
        exit(RES_FORK_FAILED);
    }

    if (child)
    {
        // Pin sets an analysis function here to notify the application
        // when Pin attaches to it in the first attach session.
        while (!AfterAttach1())
        {
            sleep(1);
        }

        NotifyTestType(TEST_TYPE_SECONDARY_THREAD_ZOMBIE_IN_REATTACH);

        pthread_t tid;
        pthread_create(&tid, NULL, SecondaryThreadMain, NULL);
        fprintf(stderr, "APP: tid of zombie: %d\n", (int)tid);

        while (1)
            ;
    }

    if (child == 0)
    {
        // Inside child
        pid_t second_child_pid = fork();

        if (second_child_pid < 0)
        {
            perror("Fork failed while creating application process");
            exit(RES_FORK_FAILED);
        }

        if (second_child_pid > 0)
        {
            // Inside parent 2
            waitpid(second_child_pid, 0, 0); // wait until Pin is injected to the application process.
            exit(RES_SUCCESS);
        }
        else
        {
            // Inside child 2
            char attachPid[MAX_SIZE];
            snprintf(attachPid, MAX_SIZE, "%d", parentPid);

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
            argv_count++;                            // skip the imageName, not needed for the execv command
            args[args_count++] = (char*)"-o";
            args[args_count++] = argv[argv_count++]; // output file
            args[args_count++] = NULL;               // end
            // Pin attaches to the application.
            execv(argv[1], (char* const*)args); // never returns
            perror("execv failed while trying to attach Pin to the application\n");
            exit(RES_EXEC_FAILED);
        }
    }

    return RES_SUCCESS;
}
