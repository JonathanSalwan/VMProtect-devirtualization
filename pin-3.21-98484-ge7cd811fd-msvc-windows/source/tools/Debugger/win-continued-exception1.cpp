/*
 * Copyright (C) 2011-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <windows.h>
#include <stdio.h>

static BOOL SafeDiv(INT32, INT32, INT32*);
static int MyFilter(unsigned long);

int main()
{
    INT32 res;
    if (!SafeDiv(10, 0, &res))
    {
        printf("Divide by zero!\n");
        fflush(stdout);
    }
    return 0;
}

static BOOL SafeDiv(INT32 dividend, INT32 divisor, INT32* pResult)
{
    __try
    {
        *pResult = dividend / divisor;
    }
    __except (MyFilter(GetExceptionCode()))
    {
        return FALSE;
    }
    return TRUE;
}

static int MyFilter(unsigned long code) { return EXCEPTION_CONTINUE_SEARCH; }
