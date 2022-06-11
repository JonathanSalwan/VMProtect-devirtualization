/*
 * Copyright (C) 2012-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <errno.h>
#include <dlfcn.h>
#include <string.h>

#define MAX_COMMAND_LINE_SIZE 15 // the size for the array of arguments to execv (this value is arbitrary)
#define EXPORT_SYM extern "C"

EXPORT_SYM int AfterAttach();

static int MAX_SIZE = 128; /*maximum line size*/

enum ExitType
{
    RES_SUCCESS = 0,     // 0
    RES_FORK_FAILED,     // 1
    RES_EXEC_FAILED,     // 2
    RES_LOAD_FAILED,     // 3
    RES_RES_INVALID_ARGS // 4
};

void UnixOpen(char* filename)
{
    void* dlh = dlopen(filename, RTLD_LAZY);
    if (!dlh)
    {
        fprintf(stderr, " Failed to load: %s because: %s\n", filename, dlerror());
        fflush(stderr);
        exit(RES_LOAD_FAILED);
    }
    dlclose(dlh);
}

int AfterAttach()
{
    // Pin sets an anslysis function here to notify the application when Pin attaches to it.
    return 0;
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
    [8] "-o"
    [9] tool's output file
*/

int main(int argc, char** argv)
{
    if (argc < 8)
    {
        fprintf(stderr, "l_imageLoad_app received too few arguments.\n");
        exit(RES_RES_INVALID_ARGS);
    }
    if (argc > MAX_COMMAND_LINE_SIZE - 2) // added: -pid attachPid NULL, omitted: argv[0..2]
    {
        fprintf(stderr, "l_imageLoad_app received too many arguments\n");
        exit(RES_RES_INVALID_ARGS);
    }

    UnixOpen(argv[1]);

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
        while (!AfterAttach())
        {
            sleep(1);
        }

        UnixOpen(argv[2]);
    }
    if (child == 0)
    {
        // inside child
        char attachPid[MAX_SIZE];
        sprintf(attachPid, "%d", parentPid);
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
        args[args_count++] = argv[argv_count++]; // "-o"
        args[args_count++] = argv[argv_count++]; // tool's output file
        args[args_count++] = NULL;               // end
        execv(argv[3], (char* const*)args);      // never returns
        perror("execv failed while trying to attach Pin to the application\n");
        exit(RES_EXEC_FAILED);
    }
    return RES_SUCCESS;
}
