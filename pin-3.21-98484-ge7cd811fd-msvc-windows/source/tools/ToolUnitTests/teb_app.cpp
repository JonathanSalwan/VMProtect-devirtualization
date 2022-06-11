/*
 * Copyright (C) 2007-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <windows.h>
#include <stdio.h>

//=======================================================================
// This is the application for testing the teb.dll tool.
// It sets and gets last system error in Windows TEB. The value should
// not change between set and get, even if the tool changes it in an
// analysis routine.
//=======================================================================

int main(int argc, char** argv)
{
    SetLastError(777);
    DWORD lastError = GetLastError();

    if (lastError == 777)
    {
        printf("Success\n");
        fflush(stdout);
    }
    else
    {
        printf("Failure\n");
        fflush(stdout);
    }
    return 0;
}
