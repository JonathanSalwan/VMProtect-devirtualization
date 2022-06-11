/*
 * Copyright (C) 2010-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <pthread.h>

static void Handle(int);

int main(int argc, char** argv)
{
    int setHandler;
    struct sigaction act;

    if (argc == 2 && strcmp(argv[1], "yes") == 0)
    {
        setHandler = 1;
    }
    else if (argc == 2 && strcmp(argv[1], "no") == 0)
    {
        setHandler = 0;
    }
    else
    {
        fprintf(stderr, "Specify either 'yes' or 'no'\n");
        return 1;
    }

    if (setHandler)
    {
        act.sa_handler = Handle;
        act.sa_flags   = 0;
        sigemptyset(&act.sa_mask);
        if (sigaction(SIGUSR1, &act, 0) != 0)
        {
            fprintf(stderr, "Unable to set up USR1 handler\n");
            return 1;
        }
    }

    pthread_kill(pthread_self(), SIGUSR1);
    printf("Application did not get SIGUSR1\n");

    return 0;
}

static void Handle(int sig)
{
    printf("Application got SIGUSR1\n");
    exit(0);
}
