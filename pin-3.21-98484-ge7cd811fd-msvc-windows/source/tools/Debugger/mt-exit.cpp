/*
 * Copyright (C) 2010-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <iostream>
#include <unistd.h>
#include <pthread.h>

static void* Blocker(void*);
static void Breakpoint();

int main()
{
    pthread_t tid;
    if (pthread_create(&tid, 0, Blocker, 0) != 0)
    {
        std::cerr << "Unable to create thread\n";
        return 1;
    }

    // Wait for the child process to block in its sleep() call,
    // and then trigger the breakpoint.
    //
    sleep(5);
    Breakpoint();

    pthread_join(tid, 0);
    return 0;
}

static void* Blocker(void*)
{
    sleep(1000);
    return 0;
}

static void Breakpoint()
{
    // GDB places a breakpoint here and then terminates
    // the application.
}
