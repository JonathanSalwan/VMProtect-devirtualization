/*
 * Copyright (C) 2010-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * This application is meant to be run with the tool "pc-change-async-tool.cpp".
 * The tool intercepts various debugger events.
 */

#include <iostream>
#include <pthread.h>
#include <unistd.h>

extern "C" int One();
extern "C" int GetValue(int (*)());
extern "C" void Breakpoint();

static void* Child(void*);
static void DoTest();

int main()
{
    // Kill the process if the test hangs.
    //
    alarm(60);

    pthread_t tid;
    if (pthread_create(&tid, 0, Child, 0) != 0)
    {
        std::cerr << "Unable to create thread\n";
        return 1;
    }

    DoTest();

    pthread_join(tid, 0);
    return 0;
}

static void* Child(void*)
{
    Breakpoint();
    return 0;
}

static void DoTest()
{
    int val = GetValue(One);
    std::cout << "Value is " << std::dec << val << std::endl;
}
