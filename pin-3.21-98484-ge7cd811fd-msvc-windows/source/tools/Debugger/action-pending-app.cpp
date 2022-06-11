/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * This test application is only interesting when run with the tool
 * "action-pending-tool.cpp".  When run under the debugger, it tests the
 * PIN_IsActionPending() API.
 */

#include <iostream>
#include <pthread.h>
#include <unistd.h>

static void* Child(void*);
static void Parent();
extern "C" void Breakpoint();
extern "C" void ToolWait();
extern "C" void ToolSignal();

int main()
{
    pthread_t tid;
    int ret = pthread_create(&tid, 0, Child, 0);
    if (ret != 0)
    {
        std::cerr << "Unable to create thread" << std::endl;
        return 1;
    }

    Parent();

    pthread_join(tid, 0);
    return 0;
}

static void* Child(void*)
{
    ToolWait();
    return 0;
}

static void Parent()
{
    sleep(2); /* wait for Child to call ToolWait() */
    Breakpoint();
    ToolSignal();
}

extern "C" void Breakpoint() { /* debugger sets a breakpoint here */ }

extern "C" void ToolWait() { /* Pin tool adds analysis routine here to wait for ToolSignal() */ }

extern "C" void ToolSignal() { /* Pin tool adds analysis routine here to release ToolWait() */ }
