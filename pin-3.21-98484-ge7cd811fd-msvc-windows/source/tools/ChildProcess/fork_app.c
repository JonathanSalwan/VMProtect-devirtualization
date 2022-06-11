/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <pthread.h>

int main()
{
    pid_t child_id = fork();
    if (child_id == 0)
    {
        printf("APPLICATION: After fork in child\n");
    }
    else
    {
        printf("APPLICATION: After fork in parent\n");
    }

    wait(0);
    return 0;
}
