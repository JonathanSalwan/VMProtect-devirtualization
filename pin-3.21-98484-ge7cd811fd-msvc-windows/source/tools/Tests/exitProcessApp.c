/*
 * Copyright (C) 2010-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

// cause an infinite loop at the exit of the app

void sendSignal()
{
    printf("sendSignal SIGUSR1\n");
    kill(getpid(), SIGUSR1);
}

void callSendSignal(int sig)
{
    printf("callSendSignal\n");
    sendSignal();
}

int main()
{
    if (signal(SIGUSR1, callSendSignal) == SIG_ERR) exit(1);

    atexit(sendSignal);
    return 0;
}
