/*
 * Copyright (C) 2015-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

volatile int iteration_main   = 0;
volatile int iteration_second = 0;
volatile bool shouldExit      = false;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// Prevent inlining of these functions
#define EMPTY_FUNC(name)    \
    extern "C" void name(); \
    __asm__(".globl " #name "\n .type " #name ", @function\n " #name ":\n ret\n");

EMPTY_FUNC(SecondThreadIterationCheckpoint1)
EMPTY_FUNC(SecondThreadIterationCheckpoint2)
EMPTY_FUNC(SecondThreadIterationCheckpoint3)
EMPTY_FUNC(MainThreadIterationCheckpoint)

void* SecondThread(void* arg)
{
    for (iteration_second = 0; iteration_second < 100;)
    {
        SecondThreadIterationCheckpoint1();
        pthread_mutex_unlock(&mutex);
        iteration_second++;
        SecondThreadIterationCheckpoint2();
        pthread_mutex_lock(&mutex);
        SecondThreadIterationCheckpoint3();

        while (iteration_second > iteration_main)
        {
            pthread_mutex_unlock(&mutex);
            sched_yield();
            pthread_mutex_lock(&mutex);
        }
    }
    pthread_mutex_unlock(&mutex);

    shouldExit = true;
    return NULL;
}

int main()
{
    pthread_t thd;

    pthread_mutex_lock(&mutex);

    pthread_create(&thd, NULL, SecondThread, NULL);

    do
    {
        MainThreadIterationCheckpoint();
        pthread_mutex_lock(&mutex);
        iteration_main++;
        while (iteration_main >= iteration_second && !shouldExit)
        {
            pthread_mutex_unlock(&mutex);
            sched_yield();
            pthread_mutex_lock(&mutex);
        }
        pthread_mutex_unlock(&mutex);
    }
    while (!shouldExit);

    printf("Application finished successfully!\n");

    return 0;
}
