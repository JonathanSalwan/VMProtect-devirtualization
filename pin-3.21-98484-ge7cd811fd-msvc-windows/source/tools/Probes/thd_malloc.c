/*
 * Copyright (C) 2007-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>

static void* child(void*);
static void* parent(void*);

int main()
{
    pthread_t tid;
    if (pthread_create(&tid, 0, child, 0) != 0) return 1;
    parent(0);

    pthread_join(tid, 0);
    printf(" test complete\n");
    return 0;
}

// Copy the malloc return address to this variable to avoid warning unused-result
static void* alloc_buffer = NULL;

static void* child(void* dummy)
{
    alloc_buffer = malloc(1);
    return 0;
}

static void* parent(void* dummy)
{
    alloc_buffer = malloc(1);
    return 0;
}
