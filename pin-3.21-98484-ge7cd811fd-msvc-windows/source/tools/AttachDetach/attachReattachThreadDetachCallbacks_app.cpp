/*
 * Copyright (C) 2012-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sched.h>
#include <pthread.h>
#include <string.h>
#define MAX_COMMAND_LINE_SIZE 15 // the size for the array of arguments to execv (this value is arbitrary)

#define EXPORT_SYM extern "C"

#define NTHREADS 20

volatile bool loop;

EXPORT_SYM bool AfterAttach1();

EXPORT_SYM bool AfterAttach2();

static int MAX_SIZE = 128; /*maximum line size*/

enum ExitType
{
    RES_SUCCESS = 0, // 0
    RES_FORK_FAILED, // 1
    RES_EXEC_FAILED, // 2
    RES_LOAD_FAILED, // 3
    RES_INVALID_ARGS // 4
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

void* thread_func(void* arg)
{
    while (loop)
    {
        sched_yield();
    }
    return 0;
}

/*
    Expected argv arguments:
    [1] pin executable
    [2] Pin flags (e.g. -slow_asserts)
        >> zero or more flags possible
    [3] "-t"
    [4] tool
    [5] output file
    [6] First imageName
	[7] Second imageName
*/
int main(int argc, char** argv)
{
    fprintf(stderr, "Start main\n");
    if (argc < 7)
    {
        fprintf(stderr, "Not enough arguments\n");
        fflush(stderr);
        exit(RES_INVALID_ARGS);
    }
    if (argc > MAX_COMMAND_LINE_SIZE - 3)
    { // added: -pid attachPid -o NULL
        fprintf(stderr, "Too many arguments\n");
        fflush(stderr);
        exit(RES_INVALID_ARGS);
    }

    loop = true;

    int ret_val;
    pthread_t h[NTHREADS];
    for (unsigned long i = 0; i < NTHREADS; i++)
    {
        ret_val = pthread_create(&h[i], 0, thread_func, 0);
        if (ret_val)
        {
            perror("ERROR, pthread_create failed");
            exit(1);
        }
    }

    pid_t parentPid = getpid();
    pid_t child     = fork();
    if (child < 0)
    {
        perror("Fork failed while creating application process");
        exit(RES_FORK_FAILED);
    }

    if (child)
    {
        while (!AfterAttach1())
        {
            sleep(1);
        }
        void* handle = dlopen(argv[argc - 2], RTLD_LAZY); // argv[argc-2] is First imageName
        if (!handle)
        {
            fprintf(stderr, " Failed to load: %s because: %s\n", argv[argc - 2], dlerror());
            fflush(stderr);
            exit(RES_LOAD_FAILED);
        }
        while (!AfterAttach2())
        {
            sleep(1);
        }
        handle = dlopen(argv[argc - 1], RTLD_LAZY); // argv[argc-1] is Second imageName
        if (!handle)
        {
            fprintf(stderr, " Failed to load: %s because: %s\n", argv[argc - 1], dlerror());
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
        // Inside child
        char attachPid[MAX_SIZE];
        snprintf(attachPid, MAX_SIZE, "%d", parentPid);
        char* args[MAX_COMMAND_LINE_SIZE] = {NULL}; // arguments for execv command
        int args_count                    = 0;
        int argv_count                    = 1;   // to start from argv[1]...
        args[args_count++] = argv[argv_count++]; // by convention, first arg is the filename of the executed file (pin)
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
    return RES_SUCCESS;
}
