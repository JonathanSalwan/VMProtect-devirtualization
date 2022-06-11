/*
 * Copyright (C) 2007-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <windows.h>
#include <stdio.h>
#include "probe_stdcall_fastcall.h"

DWORD GetEspBeforeCalled(void);

#define EXPORT_SYM __declspec(dllexport)

int dummyVar;

EXPORT_SYM DummyFunc1() { dummyVar = 1; }

char funcChar1 = 0, funcChar2 = 0;
char funcInt1 = 0, funcInt2 = 0;
EXPORT_SYM void __fastcall FastCallFunctionToBeReplacedByPin(char c1, int num1, char c2, int num2)

{
    funcChar1 = c1;
    funcChar2 = c2;
    funcInt1  = num1;
    funcInt2  = num2;
}

EXPORT_SYM Dummyfunc() { dummyVar = 2; }
main()
{
    BOOL success = TRUE;
    DWORD esp;
    esp = GetEspBeforeCalled();
    FastCallFunctionToBeReplacedByPin(CHAR_VAL1, INT_VAL1, CHAR_VAL2, INT_VAL2);
    if (esp != GetEspBeforeCalled())
    {
        printf("APP ERROR: FastCallFunctionToBeReplacedByPin returned with bad stack\n");
        success = FALSE;
    }

    if (funcChar1 != CHAR_VAL1 + CHAR_ADD_VAL1)
    {
        printf("APP ERROR: FastCallFunctionToBeReplacedByPin was not passed in c1 as expected, actual funcChar1 %c\n", funcChar1);
        success = FALSE;
    }
    if (funcChar2 != CHAR_VAL2 + CHAR_ADD_VAL2)
    {
        printf("APP ERROR: FastCallFunctionToBeReplacedByPin was not passed in c1 as expected, actual funcChar2 %c\n", funcChar2);
        success = FALSE;
    }
    if (funcInt1 != INT_VAL1 + INT_ADD_VAL1)
    {
        printf("APP ERROR: FastCallFunctionToBeReplacedByPin was not passed in num1 as expected, actual funcInt1 %d\n", funcInt1);
        success = FALSE;
    }
    if (funcInt2 != INT_VAL2 + INT_ADD_VAL2)
    {
        printf("APP ERROR: FastCallFunctionToBeReplacedByPin was not passed in num2 as expected, actual funcInt2 %d\n", funcInt2);
        success = FALSE;
    }

    if (success)
    {
        printf("SUCCESS\n");
    }
}