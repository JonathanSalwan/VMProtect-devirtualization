/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sched.h>
#include <string>
#include <signal.h>
using std::string;

int main(int argc, char** argv)
{
    printf("parent:%u-%u\n", getppid(), getpid());

    if (argc < 2)
    {
        fprintf(stderr, "Specify child application name: %s <child app name>\n", argv[0]);
        exit(-1);
    }

    string commandLine(argv[1]);
    char pidArg[10];
    sprintf(pidArg, " %d", getpid());
    commandLine += pidArg;

    int res = system(commandLine.c_str());
    if (res != 0)
    {
        fprintf(stderr, "command %s failed\n", commandLine.c_str());
        exit(-1);
    }

    if (execv(argv[1], NULL) == -1)
    {
        fprintf(stderr, "%d:%s\n", errno, strerror(errno));
    }

    return 0;
}
