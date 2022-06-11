/*
 * Copyright (C) 2015-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
  This DLL causes exception in destructor of global object.
  It should happen after application that loaded this DLL started process exit flow.
*/
#include <stdio.h>
#include <Windows.h>

int* nullPtr = 0;

// Define global object with destructor that throws hardware exception.
class MyClass
{
  public:
    ~MyClass()
    {
        __try
        {
            // Causes exception.
            *nullPtr = 5;
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            printf("Caught exception %X\n", GetExceptionCode());
        }
    }
} myObj;

extern "C" __declspec(dllexport) int Something() { return 0; }
