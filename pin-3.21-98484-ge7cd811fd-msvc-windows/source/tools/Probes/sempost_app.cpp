/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <semaphore.h>

int main()
{
    sem_t sem;
    sem_init(&sem, 0, 10);
    int res = sem_post(&sem);
    if (res != 0)
    {
        printf("sem_post result is not 0\n");
        sem_destroy(&sem);
        return -1;
    }
    sem_destroy(&sem);

    return 0;
}
