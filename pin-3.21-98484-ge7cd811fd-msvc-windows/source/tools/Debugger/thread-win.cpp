/*
 * Copyright (C) 2012-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * Utility application that creates threads on Windows().
 */

#include <cstdlib>
#include <cstring>
#include <climits>
#include <iostream>
#include <windows.h>
#include "types.h"

extern "C" __declspec(dllexport) void GlobalFunction();
typedef void (*FUNPTR)();

static bool PrintAddress = false;
static unsigned NumThreads;

static bool ParseArgs(int, char**);
static DWORD WINAPI Worker(LPVOID);

int main(int argc, char** argv)
{
    if (!ParseArgs(argc, argv)) return 1;

    if (PrintAddress)
    {
        PTRINT fp = reinterpret_cast< PTRINT >(GlobalFunction);
        std::cout << "0x" << std::hex << std::noshowbase << fp << "\n";
        return 0;
    }

    HANDLE* workers = new HANDLE[NumThreads];
    for (unsigned i = 0; i < NumThreads; i++)
    {
        workers[i] = CreateThread(0, 0, Worker, 0, 0, 0);
        if (!workers[i])
        {
            std::cerr << "Unable to create thread\n";
            return 1;
        }
    }

    for (unsigned i = 0; i < NumThreads; i++)
        WaitForSingleObject(workers[i], INFINITE);

    delete workers;
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

    std::cerr << "Usage: thread-windows -threads <number-threads>\n";
    std::cerr << "       thread-windows -print-address\n";
    return false;
}

static DWORD WINAPI Worker(LPVOID)
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
