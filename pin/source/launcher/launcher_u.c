/*
 * Copyright (C) 2012-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 *
 * Launcher code common to unix platforms.
 *
 * To modify the launcher to launch a specific tool, use the auxiliary function
 * build_user_argv. This function should return an array of the tool specific arguments
 * and their count. These arguments will be added to the pin command line.
 *
 * An example of adding a tool name and its 64bit version is shown in the comment inside
 * build_user_argv.
 *
 */

#include "os_specific.h"

static char** build_user_argv(int* argc)
{
    char** argv = NULL;
    /* Usage Example:
     ====================================================
     *argc = 4; // Number of user defined arguments

     argv = (char**) malloc(sizeof(char*) * (*argc));

     argv[0] = "-t";
     argv[1] = appendPath("path_to_tool", "/", "toolname32");
     argv[2] = "-t64";
     argv[3] = appendPath("path_to_tool", "/", "toolname64");
     */

    return argv;
}

/* For testing purposes only */
#if 0
static void check_environment()
{
    char* s;
    int i;
    const char* array[] = {
            "LD_LIBRARY_PATH",
            "PIN_VM32_LD_LIBRARY_PATH",
            "PIN_VM64_LD_LIBRARY_PATH",
            "PIN_LD_RESTORE_REQUIRED",
            "PIN_APP_LD_ASSUME_KERNEL",
            "PIN_APP_LD_LIBRARY_PATH",
            "PIN_APP_LD_PRELOAD",
            0
    };
    for (i = 0; array[i] != NULL; i++)
    {
        s = getenv(array[i]);
        printf("env[%s] = %s\n", array[i], s);
    }
}

/*!
 * Prints the command line arguments.
 * @param child_argv Command line arguments array. Must be null terminated.
 */
static void print_argv_chunks(char** child_argv)
{
    char** p = child_argv;
    unsigned int i = 0;
    printf("\n");
    while (*p)
    {
        printf("argv[%d] = [%s]\n", i, *p);
        p++;
        i++;
    }
}
#endif

static void update_environment_common(char* base_path)
{
#ifdef PIN_CRT
    char buf[PATH_MAX];
    if (NULL != realpath(base_path, buf))
    {
        strcat(buf, "/extras/crt/tzdata");
        setenv("PIN_CRT_TZDATA", buf, 1);
    }
#endif
    update_environment(base_path);
}

/* Pin launcher which runs pinbin */
int main(int orig_argc, char** orig_argv)
{
    char* path_to_cmd;
    char **child_argv, **user_argv;
    int user_argc = 0;
    char* base_path;
    char* driver_name;

    if (orig_argv == NULL || orig_argv[0] == NULL) abort();
    driver_name = find_driver_name(orig_argv[0]);
    if (driver_name == NULL) abort();
    base_path = find_base_path(driver_name);
    if (base_path == NULL) abort();
    update_environment_common(base_path);

    user_argv  = build_user_argv(&user_argc);
    child_argv = build_child_argv(base_path, orig_argc, orig_argv, user_argc, user_argv);
    if (driver_name) free(driver_name);
    if (base_path) free(base_path);
    path_to_cmd = child_argv[0];

    /* For testing purposes */
#if 0
     check_environment();
     print_argv_chunks(child_argv);
#endif

    return execv(path_to_cmd, child_argv);
}
