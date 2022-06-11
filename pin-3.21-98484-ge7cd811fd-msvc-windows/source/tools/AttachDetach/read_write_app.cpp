/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 *  Test detaching - reattach Pin on Linux
 *  The application tests Pin correctness in interrupted system calls 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#define READ_WRITE_APP_TIMEOUT 300 // seconds to timeout in case Pin's attach/detach is stuck.

/* Signal Handler. Pin doesn't kill the process if it failed to attach, exit on SIGALRM */
void ExitOnAlarm(int sig)
{
    fprintf(stderr, "Pin is not attached, exit on SIGALRM\n");
    exit(0);
}

extern "C" int AppShouldExit() { return 0; }

int main()
{
    char filename[PATH_MAX];
    sprintf(filename, "rw_test.%d.txt", getpid());

    /* perform exit after READ_WRITE_APP_TIMEOUT seconds */
    signal(SIGALRM, ExitOnAlarm);
    alarm(READ_WRITE_APP_TIMEOUT);

    while (!AppShouldExit())
    {
        FILE* out = fopen(filename, "w+");
        if (NULL == out)
        {
            int err = errno;
            printf("Failed to open output file %d\n", err);
            return -1;
        }

        size_t longStrSize = sizeof("aaaaabbbbccccddddd_") * 100;
        char* orgBuf       = (char*)malloc(longStrSize + 2);
        orgBuf[0]          = '\0';

        for (int i = 0; i < 100; i++)
        {
            sprintf(orgBuf, "%saaaaabbbbccccddddd_", orgBuf);
        }
        char* buf = (char*)malloc(longStrSize + 2);

        for (int i = 0; i < 10; i++)
        {
            if (fseek(out, 0, SEEK_SET) != 0)
            {
                printf("fseek failed\n");
                return -1;
            }
            size_t writtenBytes = fwrite(orgBuf, 1, longStrSize, out);
            if (writtenBytes != longStrSize)
            {
                printf("Write operation ended with %d bytes; the full size is %d\n", (int)writtenBytes, (int)longStrSize);
                return -1;
            }
            if (fseek(out, 0, SEEK_SET) != 0)
            {
                printf("fseek failed\n");
                return -1;
            }
            size_t readBytes = fread(buf, 1, longStrSize, out);
            if (readBytes != longStrSize)
            {
                printf("Read operation ended with %d bytes; the full size is %d\n", (int)readBytes, (int)longStrSize);
                return -1;
            }
            if (strcmp(orgBuf, buf))
            {
                printf("The strings are different after RW operation\n");
                return -1;
            }
        }
        fclose(out);
        free(orgBuf);
        free(buf);
    }
    return 0;
}
