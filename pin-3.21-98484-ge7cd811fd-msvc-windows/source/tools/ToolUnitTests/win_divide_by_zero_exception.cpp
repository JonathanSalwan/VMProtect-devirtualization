/*
 * Copyright (C) 2006-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <Windows.h>
#include <stdio.h>
#define EXPORT_SYM __declspec(dllexport)

int dummy;
/* These Pin* functions are hooks that will be hooked by functions in the 
   exception_monitor.dll tool, in order to verify that PIN has not lost control
   when excecption handling occurs
*/
extern "C" EXPORT_SYM void PinVerifyInTry() { dummy = 1; }

extern "C" EXPORT_SYM void PinVerifyInCatch() { dummy = 2; }

extern "C" EXPORT_SYM void PinVerifyAfterCatch() { dummy = 3; }

extern "C" EXPORT_SYM void PinVerifyInDestructor() { dummy = 4; }

// divide by zero exception - Exercise windows exception mechanism
int DivideByZero()
{
    static int zero = 0;
    __try
    {
        PinVerifyInTry();
        fprintf(stderr, "Going to divide by zero\n");
        int i = 1 / zero;
    }
    __except (GetExceptionCode() == EXCEPTION_INT_DIVIDE_BY_ZERO ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
    {
        PinVerifyInCatch();
        fprintf(stderr, "Catching divide by zero\n");
        fflush(stderr);
        return FALSE;
    }
    PinVerifyAfterCatch();
    return TRUE;
}

/*------------------------  dispatcher ----------------------*/

int main()
{
    DivideByZero();
    return 0;
}