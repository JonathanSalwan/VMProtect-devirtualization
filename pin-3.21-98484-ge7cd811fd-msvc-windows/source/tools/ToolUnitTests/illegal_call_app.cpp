/*
 * Copyright (C) 2012-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <windows.h>
#include <stdio.h>

typedef int (*FUNCTION_POINTER_TYPE)();

unsigned int exceptCode = 0;

static int MyExceptionFilter(LPEXCEPTION_POINTERS exceptPtr)
{
    exceptCode = exceptPtr->ExceptionRecord->ExceptionCode;
    return EXCEPTION_EXECUTE_HANDLER;
}

char data[64 * 1024] = {0x0f, 0x0b}; // this is an illegal instruction

int main(int argc, char* argv[])
{
    BOOL gotIllegalInstructionException    = FALSE;
    BOOL gotFirstAccessViolationException  = FALSE;
    BOOL gotSecondAccessViolationException = FALSE;
    DWORD oldProtect;
    VirtualProtect(data, 15, PAGE_EXECUTE_READ, &oldProtect);
    fprintf(stderr, "VirtualProtect returned %x\n", oldProtect);

    __try
    {
        FUNCTION_POINTER_TYPE fPtr;
        fPtr = reinterpret_cast< FUNCTION_POINTER_TYPE >((char*)data);
        fprintf(stderr, "fPtr to data is at %p\n", fPtr);
        fPtr();
    }
    __except (MyExceptionFilter(GetExceptionInformation()))
    {
        fprintf(stderr, "Executed call to data Got exception code %x\n", exceptCode);
        gotIllegalInstructionException = (exceptCode == EXCEPTION_ILLEGAL_INSTRUCTION);
    }

    FUNCTION_POINTER_TYPE fPtr;
    __try
    {
        fPtr = reinterpret_cast< FUNCTION_POINTER_TYPE >(-8);
        fPtr();
    }
    __except (MyExceptionFilter(GetExceptionInformation()))
    {
        fprintf(stderr, "Executed call to %p, Got exception code %x\n", fPtr, exceptCode);
        gotFirstAccessViolationException = (exceptCode == EXCEPTION_ACCESS_VIOLATION);
    }

    __try
    {
        fPtr = reinterpret_cast< FUNCTION_POINTER_TYPE >(-1);
        fPtr();
    }
    __except (MyExceptionFilter(GetExceptionInformation()))
    {
        fprintf(stderr, "Executed call to %p, Got exception code %x\n", fPtr, exceptCode);
        gotSecondAccessViolationException = (exceptCode == EXCEPTION_ACCESS_VIOLATION);
    }

    BOOL haveError = FALSE;
    if (!gotIllegalInstructionException)
    {
        haveError = TRUE;
        fprintf(stderr, "***Error: did not get expected EXCEPTION_ILLEGAL_INSTRUCTION\n");
    }
    if (!gotFirstAccessViolationException)
    {
        haveError = TRUE;
        fprintf(stderr, "***Error: did not get first expected EXCEPTION_ACCESS_VIOLATION\n");
    }
    if (!gotSecondAccessViolationException)
    {
        haveError = TRUE;
        fprintf(stderr, "***Error: did not get second expected EXCEPTION_ACCESS_VIOLATION\n");
    }
    if (haveError)
    {
        exit(1);
    }
    return (0);
}
