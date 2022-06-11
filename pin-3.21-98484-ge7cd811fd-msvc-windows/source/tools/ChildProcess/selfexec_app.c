/*
 * Copyright (C) 2010-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <unistd.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <dirent.h>

int get_num_descr()
{
    DIR* dir = opendir("/proc/self/fd");
    struct dirent* entry;
    int num = 0;

    while ((entry = readdir(dir)) != 0)
    {
        num++;
    }

    closedir(dir);

    return num;
}

int main(int argc, char* argv[])
{
    int i;
    char si[5];
    char fdstr[5];
    int num = 30; /* Number of recursive calls. */
    char fullpath[PATH_MAX];
    int origfds = 0, fds = 0;

    if (argc == 1)
    {
        i       = 1;
        origfds = get_num_descr();
    }
    else
    {
        sscanf(argv[1], "%i", &i);
        sscanf(argv[2], "%i", &origfds);
    }

    if (argv[0][0] != '/')
        readlink("/proc/self/exe", fullpath, PATH_MAX);
    else
        strcpy(fullpath, argv[0]);

    i++;
    if (i <= num)
    {
        sprintf(si, "%i", i);
        sprintf(fdstr, "%i", origfds);
        execlp(fullpath, "selfexec", si, fdstr, (char*)0);
    }

    printf("Number of recursive calls = %i.\n", --i);
    fds = get_num_descr();
    printf("Number of file descriptors: %d\n", fds);

    if (fds > origfds)
    {
        printf("Failed: number of open descriptors was increased\n");
        return 1;
    }

    return 0;
}
