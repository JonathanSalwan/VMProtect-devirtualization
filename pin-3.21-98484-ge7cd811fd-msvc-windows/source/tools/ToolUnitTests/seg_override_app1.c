/*
 * Copyright (C) 2007-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <windows.h>
#include <stdio.h>

#if defined(__cplusplus)
extern "C"
#endif
    int source[4] = {1, 1, 1, 1};
#if defined(__cplusplus)
extern "C"
#endif
    int dest[4] = {0, 0, 0, 0};

#if defined(__cplusplus)
extern "C"
#endif
    void
    TestSegOverride();

// This function is called when the application exits
VOID VerifyResults()
{
    BOOL hadFailure = FALSE;
    if (dest[0] != 1)
    {
        printf("Unexpected value of dest[0] %d\n", dest[0]);
        hadFailure = TRUE;
    }
    if (dest[1] != 1)
    {
        printf("Unexpected value of dest[1] %d\n", dest[1]);
        hadFailure = TRUE;
    }
    if (dest[2] != 0)
    {
        printf("Unexpected value of dest[2] %d\n", dest[2]);
        hadFailure = TRUE;
    }
    if (dest[3] != 0)
    {
        printf("Unexpected value of dest[3] %d\n", dest[3]);
        hadFailure = TRUE;
    }
    printf("Finished: hadFailure  %d\n", hadFailure);
    fflush(stdout);
}

int main(int argc, char* argv[])
{
    TestSegOverride();
    VerifyResults();

    return 0;
}
