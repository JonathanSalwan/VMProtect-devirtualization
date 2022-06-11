/*
 * Copyright (C) 2016-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/mman.h>
#include <unistd.h>

static void* ptr = NULL;

void allocate_memory() __attribute__((noinline));
void free_memory() __attribute__((noinline));

void allocate_memory() { ptr = mmap(NULL, getpagesize(), PROT_READ, MAP_PRIVATE | MAP_ANON, -1, 0); }

void free_memory() { munmap(ptr, getpagesize()); }

int main()
{
    allocate_memory();
    pid_t child_id = fork();
    if (child_id == 0)
    {
        printf("APPLICATION: After fork in child\n");
    }
    else
    {
        int status;
        free_memory();
        printf("APPLICATION: After fork in parent\n");
        waitpid(child_id, &status, 0);
        if (WIFEXITED(status))
        {
            status = WEXITSTATUS(status);
        }
        else if (WIFSIGNALED(status))
        {
            status = 128 + WTERMSIG(status);
        }
        return status;
    }

    return 0;
}
