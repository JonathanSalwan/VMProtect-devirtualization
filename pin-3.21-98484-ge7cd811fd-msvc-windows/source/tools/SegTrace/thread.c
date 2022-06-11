/*
 * Copyright (C) 2006-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <pthread.h>

static void* Print(void*);

int main()
{
    pthread_t tid;

    pthread_create(&tid, 0, Print, 0);
    pthread_join(tid, 0);
    return 0;
}

static void* Print(void* arg)
{
    printf("Hello world\n");
    return 0;
}
