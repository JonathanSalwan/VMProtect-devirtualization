/*
 * Copyright (C) 2016-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
* This file together with tool and makefile checks that Pin guards the file descriptors opened by itself and the
* tool (including pin.log), and doesn't let the application to close them.
*/
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>

#if defined(TARGET_MAC)
const char* DIR_FD = "/dev/fd";
#elif defined(TARGET_LINUX)
const char* DIR_FD = "/proc/self/fd";
#endif

void closeAllFiles()
{
    DIR* d = opendir(DIR_FD);
    assert(NULL != d);
    struct dirent* ent;
    while (NULL != (ent = readdir(d)))
    {
        if (ent->d_name[0] == '.')
        {
            continue;
        }
        char* endptr;
        errno             = 0;
        const long int fd = strtol(ent->d_name, &endptr, 10);
        if (*endptr || fd < 0 || errno)
        {
            continue;
        }
        if (fd == STDIN_FILENO || fd == STDOUT_FILENO || fd == STDERR_FILENO)
        {
            continue;
        }
        int ret;
        do
        {
            ret = close(fd);
        }
        while (ret == -1 && errno == EINTR);
        assert(0 == ret);
    }
}

int main()
{
    closeAllFiles();
    printf("Application is done\n");
    fflush(stdout);
    int pid = fork();
    if (pid != 0)
    {
        // This is just for safety, letting forked child finish before checking stuff in makefile
        sleep(1);
    }
    return 0;
}
