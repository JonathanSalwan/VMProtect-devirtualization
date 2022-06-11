/*
 * Copyright (C) 2012-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * Utility application that creates threads via pthreads().
 */

#include <cstdlib>
#include <cstring>
#include <climits>
#include <iostream>
#include <pthread.h>
#include "types.h"

extern "C" void GlobalFunction();
typedef void (*FUNPTR)();

static bool PrintAddress = false;
static unsigned NumThreads;

static bool ParseArgs(int, char**);
static void* Worker(void*);

int main(int argc, char** argv)
{
    if (!ParseArgs(argc, argv)) return 1;

    if (PrintAddress)
    {
        PTRINT fp = reinterpret_cast< PTRINT >(GlobalFunction);
        std::cout << "0x" << std::hex << std::noshowbase << fp << "\n";
        return 0;
    }

    pthread_t* workers = new pthread_t[NumThreads];
    for (unsigned i = 0; i < NumThreads; i++)
    {
        if (pthread_create(&workers[i], 0, Worker, 0) != 0)
        {
            std::cerr << "Unable to create thread\n";
            return 1;
        }
    }

    for (unsigned i = 0; i < NumThreads; i++)
        pthread_join(workers[i], 0);

    delete[] workers;
    return 0;
}

static bool ParseArgs(int argc, char** argv)
{
    if (argc == 3 && std::strcmp(argv[1], "-threads") == 0)
    {
        char* end;
        unsigned long val = std::strtoul(argv[2], &end, 10);
        if (*(argv[2]) == '\0' || val > UINT_MAX || val == 0 || *end != '\0')
        {
            std::cerr << "Invalid parameter to -threads: " << argv[2] << std::endl;
            return false;
        }
        NumThreads = static_cast< unsigned >(val);
        return true;
    }
    else if (argc == 2 && std::strcmp(argv[1], "-print-address") == 0)
    {
        PrintAddress = true;
        return true;
    }

    std::cerr << "Usage: thread-unix -threads <number-threads>\n";
    std::cerr << "       thread-unix -print-address\n";
    return false;
}

static void* Worker(void*)
{
    // Call through a volatile pointer to prevent the compiler from in-lining.
    //
    volatile FUNPTR fp = GlobalFunction;
    fp();
    return 0;
}

extern "C" void GlobalFunction()
{
    /*
     * This is a good place to put Pin instrumentation or to set a breakpoint.
     */
}
