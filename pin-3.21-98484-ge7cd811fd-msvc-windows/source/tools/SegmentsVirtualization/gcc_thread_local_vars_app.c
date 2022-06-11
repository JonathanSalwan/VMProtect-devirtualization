/*
 * Copyright (C) 2014-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * gcc_thread_local_vars_app.c
 * This example code checks GCC's __thread variable:
 * 1. Each of the threads writes a different value to its own __thread variable
 * 2. All threads are synchronized
 * 3. Each thread reads the value of the __thread variable and returns it back
 *        to the main thread
 * 4. The main thread prints the values of each thread's variable in order
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <pthread.h>

#define THREADS_COUNT 10

__thread int tid_local[16] = {0};
int barrier                = THREADS_COUNT;
pthread_mutex_t barrier_mutex;
pthread_cond_t barrier_threshold_cv;

void Fail(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    exit(1);
}

void AddToThreadLocalVar(int i) { tid_local[5] += i; }

void* thread_main(void* ptr)
{
    AddToThreadLocalVar(*(int*)ptr);
    AddToThreadLocalVar(*(int*)ptr);
    pthread_mutex_lock(&barrier_mutex);
    if (--barrier <= 0)
    {
        pthread_cond_signal(&barrier_threshold_cv);
    }
    pthread_mutex_unlock(&barrier_mutex);

    *(int*)ptr = tid_local[5];
    return NULL;
}

int main()
{
    pthread_t threads[THREADS_COUNT] = {0};
    int results[THREADS_COUNT]       = {0};
    int res                          = -1;
    int i                            = 0;

    res = pthread_mutex_init(&barrier_mutex, NULL);
    if (0 != res)
    {
        Fail("Failed to initialize mutex (error %d)\n", res);
    }
    res = pthread_cond_init(&barrier_threshold_cv, NULL);
    if (0 != res)
    {
        Fail("Failed to initialize condition variable (error %d)\n", res);
    }
    for (i = 0; i < THREADS_COUNT; i++)
    {
        results[i] = i;
        res        = pthread_create(&threads[i], NULL, thread_main, (void*)&results[i]);
        if (0 != res)
        {
            Fail("Failed to create thread number %d (error %d)\n", i, res);
        }
    }

    pthread_mutex_lock(&barrier_mutex);
    /*
     * Waits for all the threads to write to their '__thread' variables
     */
    while (barrier > 0)
    {
        pthread_cond_wait(&barrier_threshold_cv, &barrier_mutex);
    }
    pthread_mutex_unlock(&barrier_mutex);

    /*
     * Waits for all the threads to terminate, reading their '__thread' variables before
     */
    for (i = 0; i < THREADS_COUNT; i++)
    {
        pthread_join(threads[i], NULL);
        printf("TLS%d=%d\n", i, results[i]);
    }

    pthread_mutex_destroy(&barrier_mutex);
    pthread_cond_destroy(&barrier_threshold_cv);
    return 0;
}
