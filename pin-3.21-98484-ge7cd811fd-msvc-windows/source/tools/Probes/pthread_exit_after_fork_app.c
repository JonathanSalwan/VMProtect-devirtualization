/*
 * Copyright (C) 2015-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <assert.h>
#include <errno.h>
#include <sys/wait.h>

void* functionC(void* p0);

pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;

int main()
{
    int res;
    pthread_t thread1;
    pid_t pid = fork();
    if (pid < 0)
    {
        perror("fork failed\n");
        return 1;
    }
    else if (pid != 0)
    {
        // parent
        int stat;
        pid_t waitpid_ret;
        while (0 > (waitpid_ret = waitpid(pid, &stat, 0)) && errno == EINTR)
            ;
        if (0 > waitpid_ret)
        {
            perror("waitpid failed\n");
            return 2;
        }
        if (WIFEXITED(stat))
        {
            // process exited
            return WEXITSTATUS(stat);
        }
        // process terminated by a signal
        return WTERMSIG(stat) + 128;
    }

    int param;
    if ((res = pthread_create(&thread1, NULL, &functionC, &param)))
    {
        printf("Thread creation failed: %d\n", res);
        return 1;
    }

    pthread_join(thread1, NULL);
    /*   pthread_join( thread2, NULL);*/
    return 0;
}

void* functionC(void* p0)
{
    pthread_exit(p0);
    printf("ERROR: This code comes after pthread_exit. It should not be executed!\n");
    assert(0);
}
