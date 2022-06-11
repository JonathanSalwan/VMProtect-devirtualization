/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdlib.h>
#include <stdio.h>
#include <windows.h>

//=======================================================================
// Application which sets and gets last system error in Windows TEB.
// The value should not be changed between set and get
//=======================================================================

int main()
{
    int errorCode = 0x12345;
    SetLastError(errorCode);
    if (GetLastError() != errorCode)
    {
        fprintf(stderr, "Failure: Bad value returned from GetLastError\n");
        fflush(stderr);
        exit(1);
    }

    return 0;
}
