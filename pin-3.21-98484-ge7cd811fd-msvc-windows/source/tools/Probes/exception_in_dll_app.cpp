/*
 * Copyright (C) 2011-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
  This application causes exception in indirect call instruction and catches it in caller.
  The call instruction is located in code region being replaced by Pin probe.
  Pin translation should not affect propagation of the exception to the C++ exception handler.
*/
#include <stdio.h>

bool destructed = false;

// cpp exceptions - Exercise windows exception mechanism
class MyClass
{
  public:
    ~MyClass() { destructed = true; }
};

extern "C" void ExcInDll();
static int (*pBar)() = 0;

extern "C" unsigned long getstack();
extern "C" unsigned long getebp();

int bar() { return 0; }
extern "C" void foo1()
{
    if (!pBar) throw(0);
    // May cause exception due to NULL pointer
    pBar();
}

extern "C" void foo2() { ExcInDll(); }

int main()
{
    int local = 1;

    unsigned long stackBefore    = getstack();
    unsigned long framePtrBefore = getebp();
    try
    {
        MyClass ins;
        foo1();
        local = 0;
    }
    catch (...)
    {
        // If Pin translated probed code properly, exception will reach the handler
        printf("Exception\n");
    }
    unsigned long stackAfter    = getstack();
    unsigned long framePtrAfter = getebp();

    if ((stackBefore != stackAfter) || (framePtrBefore != framePtrAfter))
    {
        printf("before try  Stack at 0x%x, ebp 0x%x\n", getstack(), getebp());
        printf("after catch Stack at 0x%x, ebp 0x%x\n", getstack(), getebp());
        return -1;
    }

    int param1           = 1;
    int param2           = param1 * 5;
    float param3         = 0.5 * 1.5;
    float expectedResult = 0;
    try
    {
        expectedResult = param3 + param2 + param1;
        foo2();
    }
    catch (...)
    {
        float afterCatchResult = param3 + param2 + param1;
        if (afterCatchResult != expectedResult)
        {
            printf("expectedResult = %f; afterCatchResult = %f\n", expectedResult, afterCatchResult);
            printf("Registers may be restored incorrectly in the catch block\n");
            return -1;
        }
        else
        {
            printf("Try-catch works correctly while exception propagation from dll\n");
        }
    }
    stackAfter    = getstack();
    framePtrAfter = getebp();

    if ((stackBefore != stackAfter) || (framePtrBefore != framePtrAfter))
    {
        printf("Incorrect stack of frame ptr after exception propagation from dll");
        printf("before try  Stack at 0x%x, ebp 0x%x\n", getstack(), getebp());
        printf("after catch Stack at 0x%x, ebp 0x%x\n", getstack(), getebp());
        return -1;
    }

    // Check that destructor was called and local var value was not changed when exception was handled
    if (!destructed || (local != 1))
    {
        return 1;
    }

    pBar = bar;

    try
    {
        foo1();
    }
    catch (...)
    {
        // No exception expected
        printf("Exception\n");
    }

    return 0;
}
