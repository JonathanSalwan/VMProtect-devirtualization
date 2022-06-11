/*
 * Copyright (C) 2012-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#ifdef TARGET_WINDOWS
#include <windows.h>
#endif
#include <stdio.h>
#include "probe_stdcall_fastcall.h"

DWORD GetEspBeforeCalled(void);

#define EXPORT_SYM __declspec(dllexport)

int dummyVar;

EXPORT_SYM DummyFunc1() { dummyVar = 1; }

char funcChar1 = 0, funcChar2 = 0, funcChar3 = 0;
char funcInt1 = 0, funcInt2 = 0;
EXPORT_SYM int DefaultCallFunctionToBeReplacedByPin(char c1, int num1, char c2, int num2, char c3)

{
    printf("DefaultCallFunctionToBeReplacedByPin c1 %c num1 %d c2 %c num2 %d c3 %c\n", c1, num1, c2, num2, c3);
    funcChar1 = c1;
    funcChar2 = c2;
    funcChar3 = c3;
    funcInt1  = num1;
    funcInt2  = num2;
    return (RETVAL1);
}

EXPORT_SYM int DefaultCallFunction2ToBeReplacedByPin(char c1, int num1, char c2, int num2, char c3)

{
    printf("DefaultCallFunction2ToBeReplacedByPin c1 %c num1 %d c2 %c num2 %d c3 %c\n", c1, num1, c2, num2, c3);
    funcChar1 = c1;
    funcChar2 = c2;
    funcChar3 = c3;
    funcInt1  = num1;
    funcInt2  = num2;
    return (RETVAL2);
}

EXPORT_SYM int DefaultCallFunction3ToBeReplacedByPin(char c1, int num1, char c2, int num2, char c3)

{
    printf("DefaultCallFunction3ToBeReplacedByPin c1 %c num1 %d c2 %c num2 %d c3 %c\n", c1, num1, c2, num2, c3);
    funcChar1 = c1;
    funcChar2 = c2;
    funcChar3 = c3;
    funcInt1  = num1;
    funcInt2  = num2;
    return (RETVAL3);
}

EXPORT_SYM int DefaultCallFunction4ToBeReplacedByPin(char c1, int num1, char c2, int num2, char c3)

{
    printf("DefaultCallFunction4ToBeReplacedByPin c1 %c num1 %d c2 %c num2 %d c3 %c\n", c1, num1, c2, num2, c3);
    funcChar1 = c1;
    funcChar2 = c2;
    funcChar3 = c3;
    funcInt1  = num1;
    funcInt2  = num2;
    return (RETVAL4);
}

EXPORT_SYM int DefaultCallFunction5ToBeReplacedByPin(char c1, int num1, char c2, int num2, char c3)

{
    printf("DefaultCallFunction5ToBeReplacedByPin c1 %c num1 %d c2 %c num2 %d c3 %c\n", c1, num1, c2, num2, c3);
    funcChar1 = c1;
    funcChar2 = c2;
    funcChar3 = c3;
    funcInt1  = num1;
    funcInt2  = num2;
    return (RETVAL5);
}

EXPORT_SYM Dummyfunc() { dummyVar = 2; }
main()
{
    BOOL success = TRUE;
    DWORD esp, espAfter;
    int retVal1, retVal2, retVal3, retVal4, retVal5;
    funcChar1 = 0;
    funcChar2 = 0;
    funcInt1  = 0;
    funcInt2  = 0;
    funcChar3 = 0;
    esp       = GetEspBeforeCalled();
    printf("esp before DefaultCallFunctionToBeReplacedByPin %x\n", esp);
    retVal1  = DefaultCallFunctionToBeReplacedByPin(CHAR_VAL1, INT_VAL1, CHAR_VAL2, INT_VAL2, CHAR_VAL3);
    espAfter = GetEspBeforeCalled();
    if (esp != espAfter)
    {
        printf("APP ERROR: DefaultCallFunctionToBeReplacedByPin returned with bad stack\n");
        success = FALSE;
    }
    printf("esp after  DefaultCallFunctionToBeReplacedByPin %x\n", espAfter);

    if (retVal1 != RETVAL1)
    {
        printf("APP ERROR: DefaultCallFunctionToBeReplacedByPin returned unexpected value %d\n", retVal1);
        success = FALSE;
    }
    if (funcChar1 != CHAR_VAL1)
    {
        printf("APP ERROR: DefaultCallFunctionToBeReplacedByPin was not passed in c1 as expected, actual funcChar1 %c\n",
               funcChar1);
        success = FALSE;
    }
    if (funcChar2 != CHAR_VAL2)
    {
        printf("APP ERROR: DefaultCallFunctionToBeReplacedByPin was not passed in c2 as expected, actual funcChar2 %c\n",
               funcChar2);
        success = FALSE;
    }
    if (funcChar3 != CHAR_VAL3)
    {
        printf("APP ERROR: DefaultCallFunction5ToBeReplacedByPin was not passed in c3 as expected, actual funcChar3 %c\n",
               funcChar3);
        success = FALSE;
    }
    if (funcInt1 != INT_VAL1)
    {
        printf("APP ERROR: DefaultCallFunctionToBeReplacedByPin was not passed in num1 as expected, actual funcInt1 %d\n",
               funcInt1);
        success = FALSE;
    }
    if (funcInt2 != INT_VAL2)
    {
        printf("APP ERROR: DefaultCallFunctionToBeReplacedByPin was not passed in num2 as expected, actual funcInt2 %d\n",
               funcInt2);
        success = FALSE;
    }

    funcChar1 = 0;
    funcChar2 = 0;
    funcInt1  = 0;
    funcInt2  = 0;
    funcChar3 = 0;
    esp       = GetEspBeforeCalled();
    printf("esp before DefaultCallFunction2ToBeReplacedByPin %x\n", esp);
    retVal2  = DefaultCallFunction2ToBeReplacedByPin(CHAR_VAL1, INT_VAL1, CHAR_VAL2, INT_VAL2, CHAR_VAL3);
    espAfter = GetEspBeforeCalled();
    if (esp != espAfter)
    {
        printf("APP ERROR: DefaultCallFunction2ToBeReplacedByPin returned with bad stack\n");
        success = FALSE;
    }
    printf("esp after  DefaultCallFunction2ToBeReplacedByPin %x\n", espAfter);

    if (retVal2 != RETVAL2)
    {
        printf("APP ERROR: DefaultCallFunction2ToBeReplacedByPin returned unexpected value %d\n", retVal2);
        success = FALSE;
    }

    if (funcChar1 != CHAR_VAL1)
    {
        printf("APPX ERROR: DefaultCallFunction2ToBeReplacedByPin was not passed in c1 as expected, actual funcChar1 %c\n",
               funcChar1);
        success = FALSE;
    }
    if (funcChar2 != CHAR_VAL2)
    {
        printf("APP ERROR: DefaultCallFunction2ToBeReplacedByPin was not passed in c2 as expected, actual funcChar2 %c\n",
               funcChar2);
        success = FALSE;
    }
    if (funcChar3 != CHAR_VAL3)
    {
        printf("APP ERROR: DefaultCallFunction5ToBeReplacedByPin was not passed in c3 as expected, actual funcChar3 %c\n",
               funcChar3);
        success = FALSE;
    }
    if (funcInt1 != INT_VAL1)
    {
        printf("APP ERROR: DefaultCallFunction2ToBeReplacedByPin was not passed in num1 as expected, actual funcInt1 %d\n",
               funcInt1);
        success = FALSE;
    }
    if (funcInt2 != INT_VAL2)
    {
        printf("APP ERROR: DefaultCallFunction2ToBeReplacedByPin was not passed in num2 as expected, actual funcInt2 %d\n",
               funcInt2);
        success = FALSE;
    }

    funcChar1 = 0;
    funcChar2 = 0;
    funcInt1  = 0;
    funcInt2  = 0;
    funcChar3 = 0;
    esp       = GetEspBeforeCalled();
    printf("esp before DefaultCallFunction3ToBeReplacedByPin %x\n", esp);
    retVal3  = DefaultCallFunction3ToBeReplacedByPin(CHAR_VAL1, INT_VAL1, CHAR_VAL2, INT_VAL2, CHAR_VAL3);
    espAfter = GetEspBeforeCalled();
    if (esp != espAfter)
    {
        printf("APP ERROR: DefaultCallFunction3ToBeReplacedByPin returned with bad stack\n");
        success = FALSE;
    }
    printf("esp after  DefaultCallFunction3ToBeReplacedByPin %x\n", espAfter);

    if (retVal3 != RETVAL3)
    {
        printf("APP ERROR: DefaultCallFunction3ToBeReplacedByPin returned unexpected value %d\n", retVal3);
        success = FALSE;
    }
    if (funcChar1 != CHAR_VAL1)
    {
        printf("APP ERROR: DefaultCallFunction3ToBeReplacedByPin was not passed in c1 as expected, actual funcChar1 %c\n",
               funcChar1);
        success = FALSE;
    }
    if (funcChar2 != CHAR_VAL2)
    {
        printf("APP ERROR: DefaultCallFunction3ToBeReplacedByPin was not passed in c2 as expected, actual funcChar2 %c\n",
               funcChar2);
        success = FALSE;
    }
    if (funcChar3 != CHAR_VAL3)
    {
        printf("APP ERROR: DefaultCallFunction5ToBeReplacedByPin was not passed in c3 as expected, actual funcChar3 %c\n",
               funcChar3);
        success = FALSE;
    }
    if (funcInt1 != INT_VAL1)
    {
        printf("APP ERROR: DefaultCallFunction3ToBeReplacedByPin was not passed in num1 as expected, actual funcInt1 %d\n",
               funcInt1);
        success = FALSE;
    }
    if (funcInt2 != INT_VAL2)
    {
        printf("APP ERROR: DefaultCallFunction3ToBeReplacedByPin was not passed in num2 as expected, actual funcInt2 %d\n",
               funcInt2);
        success = FALSE;
    }

    if (success)
    {
        printf("SUCCESS\n");
    }

    funcChar1 = 0;
    funcChar2 = 0;
    funcInt1  = 0;
    funcInt2  = 0;
    funcChar3 = 0;
    esp       = GetEspBeforeCalled();
    printf("esp before DefaultCallFunction4ToBeReplacedByPin %x\n", esp);
    retVal4  = DefaultCallFunction4ToBeReplacedByPin(CHAR_VAL1, INT_VAL1, CHAR_VAL2, INT_VAL2, CHAR_VAL3);
    espAfter = GetEspBeforeCalled();
    if (esp != espAfter)
    {
        printf("APP ERROR: DefaultCallFunction4ToBeReplacedByPin returned with bad stack\n");
        success = FALSE;
    }
    printf("esp after  DefaultCallFunction4ToBeReplacedByPin %x\n", espAfter);

    if (retVal4 != RETVAL4)
    {
        printf("APP ERROR: DefaultCallFunction4ToBeReplacedByPin returned unexpected value %d\n", retVal4);
        success = FALSE;
    }
    if (funcChar1 != CHAR_VAL1)
    {
        printf("APP ERROR: DefaultCallFunction4ToBeReplacedByPin was not passed in c1 as expected, actual funcChar1 %c\n",
               funcChar1);
        success = FALSE;
    }
    if (funcChar2 != CHAR_VAL2)
    {
        printf("APP ERROR: DefaultCallFunction4ToBeReplacedByPin was not passed in c2 as expected, actual funcChar2 %c\n",
               funcChar2);
        success = FALSE;
    }
    if (funcChar3 != CHAR_VAL3)
    {
        printf("APP ERROR: DefaultCallFunction5ToBeReplacedByPin was not passed in c3 as expected, actual funcChar3 %c\n",
               funcChar3);
        success = FALSE;
    }
    if (funcInt1 != INT_VAL1)
    {
        printf("APP ERROR: DefaultCallFunction4ToBeReplacedByPin was not passed in num1 as expected, actual funcInt1 %d\n",
               funcInt1);
        success = FALSE;
    }
    if (funcInt2 != INT_VAL2)
    {
        printf("APP ERROR: DefaultCallFunction4ToBeReplacedByPin was not passed in num2 as expected, actual funcInt2 %d\n",
               funcInt2);
        success = FALSE;
    }

    if (success)
    {
        printf("SUCCESS\n");
    }

    funcChar1 = 0;
    funcChar2 = 0;
    funcInt1  = 0;
    funcInt2  = 0;
    funcChar3 = 0;
    esp       = GetEspBeforeCalled();
    printf("esp before DefaultCallFunction5ToBeReplacedByPin %x\n", esp);
    retVal5  = DefaultCallFunction5ToBeReplacedByPin(CHAR_VAL1, INT_VAL1, CHAR_VAL2, INT_VAL2, CHAR_VAL3);
    espAfter = GetEspBeforeCalled();
    if (esp != espAfter)
    {
        printf("APP ERROR: DefaultCallFunction5ToBeReplacedByPin returned with bad stack\n");
        success = FALSE;
    }
    printf("esp after  DefaultCallFunction5ToBeReplacedByPin %x\n", espAfter);

    if (retVal5 != RETVAL5)
    {
        printf("APP ERROR: DefaultCallFunction5ToBeReplacedByPin returned unexpected value %d\n", retVal5);
        success = FALSE;
    }

    if (funcChar1 != CHAR_VAL1)
    {
        printf("APP ERROR: DefaultCallFunction5ToBeReplacedByPin was not passed in c1 as expected, actual funcChar1 %c\n",
               funcChar1);
        success = FALSE;
    }
    if (funcChar2 != CHAR_VAL2)
    {
        printf("APP ERROR: DefaultCallFunction5ToBeReplacedByPin was not passed in c2 as expected, actual funcChar2 %c\n",
               funcChar2);
        success = FALSE;
    }
    if (funcChar3 != CHAR_VAL3)
    {
        printf("APP ERROR: DefaultCallFunction5ToBeReplacedByPin was not passed in c3 as expected, actual funcChar3 %c\n",
               funcChar3);
        success = FALSE;
    }
    if (funcInt1 != INT_VAL1)
    {
        printf("APP ERROR: DefaultCallFunction5ToBeReplacedByPin was not passed in num1 as expected, actual funcInt1 %d\n",
               funcInt1);
        success = FALSE;
    }
    if (funcInt2 != INT_VAL2)
    {
        printf("APP ERROR: DefaultCallFunction5ToBeReplacedByPin was not passed in num2 as expected, actual funcInt2 %d\n",
               funcInt2);
        success = FALSE;
    }

    if (success)
    {
        printf("SUCCESS\n");
    }
}