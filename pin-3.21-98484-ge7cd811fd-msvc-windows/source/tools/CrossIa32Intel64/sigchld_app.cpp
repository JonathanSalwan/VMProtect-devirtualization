/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>

int main(int argc, char** argv)
{
    if (argc == 1)
    {
        FILE* out = fopen("sigchld_app.out", "w");
        struct sigaction action;
        sigaction(SIGCHLD, 0, &action);
        if (action.sa_handler == SIG_IGN)
        {
            fprintf(out, "SA handler of the child process is SIG_IGN\n");
        }
        else if (action.sa_handler == SIG_DFL)
        {
            fprintf(out, "SA handler of the child process is SIG_DFL\n");
        }
        fclose(out);
        return 0;
    }

    struct sigaction action;
    char* execv_argv[2];
    execv_argv[0] = argv[1];
    execv_argv[1] = NULL;

    // do not transform children into zombies when they terminate
    action.sa_handler = SIG_IGN;
    sigemptyset(&action.sa_mask);
    action.sa_flags = SA_NOCLDWAIT;
    sigaction(SIGCHLD, &action, NULL);

    // it requires a care inside Pin, othrewise injector fails

    execv(execv_argv[0], execv_argv);
    printf("exec failed\n");
    return -1;
}
