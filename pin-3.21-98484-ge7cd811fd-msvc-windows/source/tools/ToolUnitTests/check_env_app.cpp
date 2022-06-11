/*
 * Copyright (C) 2010-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <iostream>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
using std::string;

void run_again(char** argv);

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        printf("Expected at least 1 input parameter\n");
        return -1;
    }

    string expected_ld_preload_val = argv[1];

    bool second_run = false;
    if (argc == 3)
    {
        second_run = true;
    }

    char* ld_preload_val = getenv("LD_PRELOAD");
    if (!ld_preload_val)
    {
        printf("check_env ERROR: Unexpected or missing LD_PRELOAD\n");
        return -1;
    }
    if (ld_preload_val == expected_ld_preload_val)
    {
        if (!second_run)
        {
            run_again(argv);
        }
        return 0;
    }
    else
    {
        printf("%s\n", ld_preload_val);
    }
}

void run_again(char** argv)
{
    char* ld_preload_val      = getenv("LD_PRELOAD");
    char ld_preload_new_val[] = "libm.so.6";

    setenv("LD_PRELOAD", ld_preload_new_val, 1);

    char* execv_argv[4];
    execv_argv[0] = argv[0];
    execv_argv[1] = (char*)"libm.so.6";
    execv_argv[2] = (char*)"1";
    execv_argv[3] = NULL;

    execv(execv_argv[0], execv_argv);
}
