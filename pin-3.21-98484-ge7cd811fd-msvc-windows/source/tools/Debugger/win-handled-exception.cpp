/*
 * Copyright (C) 2011-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <windows.h>
#include <stdio.h>

static BOOL SafeDiv(INT32, INT32, INT32*);
static int Filter(unsigned int, struct _EXCEPTION_POINTERS*);

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
    __except (Filter(GetExceptionCode(), GetExceptionInformation()))
    {
        return FALSE;
    }
    return TRUE;
}

static int Filter(unsigned int code, struct _EXCEPTION_POINTERS* ep)
{
    if (code == EXCEPTION_INT_DIVIDE_BY_ZERO)
    {
        return EXCEPTION_EXECUTE_HANDLER;
    }
    else
    {
        return EXCEPTION_CONTINUE_SEARCH;
    }
}
