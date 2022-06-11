/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <sys/wait.h>
using std::cerr;
using std::endl;
using std::string;

string expected;

void SigUsr1Handler(int sig) { cerr << expected << "Caught signal SIGUSR1" << endl; }

void SigUsr2Handler(int sig) { cerr << expected << "Caught signal SIGUSR2" << endl; }
void BlockSignal(int sigNo)
{
    sigset_t mask;
    sigprocmask(SIG_SETMASK, 0, &mask);
    sigaddset(&mask, sigNo);
    sigprocmask(SIG_SETMASK, &mask, 0);
}

void UnblockAllSignals()
{
    sigset_t mask;
    sigemptyset(&mask);
    sigprocmask(SIG_SETMASK, &mask, 0);
}

void BlockAllSignals()
{
    sigset_t mask;
    sigfillset(&mask);
    sigprocmask(SIG_SETMASK, &mask, 0);
}

void UnblockSignal(int sigNo)
{
    sigset_t mask;
    sigprocmask(SIG_SETMASK, 0, &mask);
    sigdelset(&mask, sigNo);
    sigprocmask(SIG_SETMASK, &mask, 0);
}

int main(int argc, char* argv[])
{
    if (argc == 1)
    {
        expected = "Expected U1 or U2: ";
    }
    else
    {
        expected = "Expected U2: ";
    }
    signal(SIGUSR1, SigUsr1Handler);
    signal(SIGUSR2, SigUsr2Handler);
    kill(getpid(), SIGUSR1);
    kill(getpid(), SIGUSR2);

    if (argc == 1)
    {
        // Second run with non-empty mask
        BlockAllSignals();
        UnblockSignal(SIGUSR2);

        char* execv_argv[3];
        execv_argv[0] = argv[0];
        execv_argv[1] = "1";
        execv_argv[2] = NULL;
        execv(execv_argv[0], execv_argv);
        printf("exec failed\n");
        return -1;
    }
    return 0;
}
