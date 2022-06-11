/*
 * Copyright (C) 2016-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 * A App that waits for pin internal thread
 * creation and then finishes successfuly
 * else finishes with error
 */

#if defined(TARGET_WINDOWS)
#include <windows.h>
#else
#include <unistd.h>
#endif

void MySleep(int count)
{
#if defined(TARGET_WINDOWS)
    Sleep(count);
#else
    usleep(count * 1000);
#endif
}

#if defined(TARGET_WINDOWS)
#define EXPORT_SYM extern "C" __declspec(dllexport)
#else
#define EXPORT_SYM extern "C"
#endif

EXPORT_SYM bool IsInternalThreadCreated() { return false; }

typedef bool (*FUNPTR)();

int main()
{
    volatile FUNPTR threadCreated = IsInternalThreadCreated;
    int i;
    for (i = 0; i < 10; i++)
    {
        // Pin Internal thread created.
        if (threadCreated())
        {
            // Success.
            return 0;
        }

        MySleep(1000);
    }
    // Pin Internal thread was not created.
    return 1;
}
