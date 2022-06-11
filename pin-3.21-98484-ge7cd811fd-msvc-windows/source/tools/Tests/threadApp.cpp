/*
 * Copyright (C) 2010-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <pthread.h>

void* DoNothing(void* arg) { return NULL; }

int main()
{
    pthread_t tid;
    pthread_create(&tid, 0, DoNothing, 0);
    pthread_join(tid, 0);
}
