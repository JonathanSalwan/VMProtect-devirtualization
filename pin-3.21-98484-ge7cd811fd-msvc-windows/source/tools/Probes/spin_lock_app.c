/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

volatile int lock = 0;

static void* child(void* v)
{
    printf("Child: calling pthread_spin_lock()\n");
    sleep(3);
    int locked = pthread_spin_lock(&lock);
    printf("Child: acquired spinlock\n");
    locked = pthread_spin_unlock(&lock);
    printf("Child: released spinlock\n");
    return 0;
}

int main()
{
    pthread_t child_thread;

    pthread_spin_init(&lock, PTHREAD_PROCESS_PRIVATE);

    printf("Main: calling pthread_spin_lock()\n");
    int locked = pthread_spin_lock(&lock);
    printf("Main: acquired spinlock\n");

    int status = pthread_create(&child_thread, 0, child, 0);
    if (status != 0) printf("Main: could not create chikd thread\n");

    sleep(10);

    locked = pthread_spin_unlock(&lock);
    printf("Main: released spinlock\n");

    pthread_join(child_thread, 0);
}
