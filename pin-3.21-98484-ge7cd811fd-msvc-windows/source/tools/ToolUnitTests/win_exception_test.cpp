/*
 * Copyright (C) 2010-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <iostream>
#include <sstream>
#include <cstdlib>
#include <cstring>
#include <stdio.h>
#include <stdlib.h>
#include "windows.h"

static int enteredFilter = 0;
static int MyExceptionFilter(LPEXCEPTION_POINTERS exceptPtr)
{
    enteredFilter = 1;
    return EXCEPTION_EXECUTE_HANDLER;
}

int main(int argc, char* argv[])
{
    __try
    {
        __asm
        {
            xor ebx, ebx
            mov eax, dword ptr [ebx] // test verifies that Pin does NOT optimize away this instruction
              // which would mean the exception is not generated
            xor eax, eax            // makes previous load of eax dead - make sure
                         // pin does not optimize away that load which would
                         // cause the exception not to occur.
        }
    }
    __except (MyExceptionFilter(GetExceptionInformation()))
    {
    }
    if (!enteredFilter)
    {
        printf("FAILURE: MyExceptionFilter not entered\n");
        fflush(stdout);
        return (-1);
    }
    return 0;
}
