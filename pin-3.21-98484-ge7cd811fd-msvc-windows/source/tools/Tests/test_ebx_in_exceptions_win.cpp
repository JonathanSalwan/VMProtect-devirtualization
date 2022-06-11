/*
 * Copyright (C) 2012-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * This test causes and handles access violations/bus errors in 
   instructions that use the ebx register implicitly.
   The relevant instructions are cmpxchg8b and xlat (which uses ebx as
   a base register)
 */

#include <windows.h>
#include <stdio.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <cstdlib>
using std::fprintf;
using std::string;
/*!
 * @return IP register value in the given exception context
 */
#if defined(TARGET_IA32)
static ULONG_PTR GetIp(CONTEXT* pExceptContext) { return pExceptContext->Eip; }
static VOID SetIp(LPEXCEPTION_POINTERS exceptPtr, ULONG_PTR addr) { exceptPtr->ContextRecord->Eip = addr; }
#elif defined(TARGET_IA32E)
static ULONG_PTR GetIp(CONTEXT* pExceptContext) { return pExceptContext->Rip; }
static VOID SetIp(LPEXCEPTION_POINTERS exceptPtr, ULONG_PTR addr) { exceptPtr->ContextRecord->Rip = addr; }
#else
#error Unsupported architechture
#endif

extern "C" unsigned int GetInstructionLenAndDisasm(unsigned char* ip, string* str);
extern "C" int TestAccessViolations();
int numExceptions = 0;

/*!
 * Exception filter for the ExecuteSafe function: copy the exception record 
 * to the specified structure.
 * @param[in] exceptPtr        pointers to the exception context and the exception 
 *                             record prepared by the system
 * @param[out] pExceptRecord   pointer to the structure that receives the 
 *                             exception record
 * @param[out] pExceptContext  pointer to the structure that receives the 
 *                             exception context
 * @return the exception disposition
 */
static int SafeExceptionFilter(LPEXCEPTION_POINTERS exceptPtr, EXCEPTION_RECORD* pExceptRecord, CONTEXT* pExceptContext)
{
    numExceptions++;
    *pExceptRecord  = *(exceptPtr->ExceptionRecord);
    *pExceptContext = *(exceptPtr->ContextRecord);
    fprintf(stderr,
            "SafeExceptionFilter: Exception code %x Exception address %p Context IP %p \npExceptContext->Eax %p "
            "exceptPtr->ContextRecord->Eax %p\npExceptContext->Ebx %p exceptPtr->ContextRecord->Ebx %p\npExceptContext->Ecx %p "
            "exceptPtr->ContextRecord->Ecx %p\npExceptContext->Edx %p exceptPtr->ContextRecord->Edx %p\n",
            pExceptRecord->ExceptionCode, (ULONG_PTR)(pExceptRecord->ExceptionAddress), GetIp(pExceptContext),
            pExceptContext->Eax, exceptPtr->ContextRecord->Eax, pExceptContext->Ebx, exceptPtr->ContextRecord->Ebx,
            pExceptContext->Ecx, exceptPtr->ContextRecord->Ecx, pExceptContext->Edx, exceptPtr->ContextRecord->Edx);
    // continue execution at the instruction following the exception causing instruction
    ULONG_PTR ipToContinueAt = GetIp(pExceptContext);
    string str;
    UINT32 instructionLen = GetInstructionLenAndDisasm((UINT8*)ipToContinueAt, &str);
    if (0 == instructionLen)
    {
        fprintf(stderr, "***Error\n");
        exit(1);
    }
    fprintf(stderr, "segv at: %s\n", str.c_str());
    ipToContinueAt = (ULONG_PTR)((UINT8*)ipToContinueAt + instructionLen);
    fprintf(stderr, " setting resume ip to %p\n", ipToContinueAt);
    instructionLen = GetInstructionLenAndDisasm((unsigned char*)ipToContinueAt, &str);
    if (0 == instructionLen)
    {
        fprintf(stderr, "***Error 0 length instruction at ip %p\n", ipToContinueAt);
        exit(1);
    }
    fprintf(stderr, "  resume instruction is %s\n", str.c_str());
    SetIp(exceptPtr, ipToContinueAt);
    return EXCEPTION_CONTINUE_EXECUTION; // EXCEPTION_EXECUTE_HANDLER;
}

extern "C" void InitXed();
int main(int argc, char* argv[])
{
    InitXed();
    int retVal;
    EXCEPTION_RECORD exceptRecord;
    CONTEXT exceptContext;
    __try
    {
        retVal = TestAccessViolations();
    }
    __except (SafeExceptionFilter(GetExceptionInformation(), &exceptRecord, &exceptContext))
    {
        fprintf(stderr, "Exception handler: Exception code %x Exception address %p Context IP %p\n", exceptRecord.ExceptionCode,
                (ULONG_PTR)(exceptRecord.ExceptionAddress), GetIp(&exceptContext));
    }
    if (!retVal || numExceptions != 3)
    {
        fprintf(stderr, "***Error\n");
        exit(1);
    }
}
