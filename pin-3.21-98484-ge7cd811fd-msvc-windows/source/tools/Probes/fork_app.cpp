/*
 * Copyright (C) 2014-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <pthread.h>
#include <sys/wait.h>

/***************************************************************************/

char* father_file_name = NULL;
char* child_file_name  = NULL;

static void event_on_process_fork_before(void)
{
    fprintf(stdout, "event_on_process_fork_before\n");
    fflush(stdout);
}

static void event_on_process_fork_after_in_parent(void)
{
    fprintf(stdout, "event_on_process_fork_after_in_parent\n");
    fflush(stdout);
    FILE* fd = fopen(father_file_name, "w");
    fprintf(fd, "This is temporary log written from within pthread_atfork hook");
    fclose(fd);
}

static void event_on_process_fork_after_in_child(void)
{
    fprintf(stdout, "event_on_process_fork_after_in_child\n");
    fflush(stdout);
    FILE* fd = fopen(child_file_name, "w");
    fprintf(fd, "This is temporary log written from within pthread_atfork hook");
    fclose(fd);
}

/***************************************************************************/

int testFork()
{
    pthread_atfork(event_on_process_fork_before, event_on_process_fork_after_in_parent, event_on_process_fork_after_in_child);
    if (fork())
    {
        int res = 0;

        fprintf(stdout, "after fork before wait of child\n");
        fflush(stdout);
        wait(&res);
    }
    else
    {
        fprintf(stdout, "from child before exit\n");
        fflush(stdout);
        exit(0);
    }

    return 0;
}

/***************************************************************************/

int main(int argc, char* argv[])
{
    if (argc < 3)
    {
        fprintf(stderr, "fork_app requires 2 parameters. father/child file names");
        exit(1);
    }

    father_file_name = argv[1];
    child_file_name  = argv[2];

    fprintf(stdout, "main before fork\n");
    fflush(stdout);

    testFork();

    fprintf(stdout, "main after fork\n");
    fflush(stdout);

    return 0;
}
