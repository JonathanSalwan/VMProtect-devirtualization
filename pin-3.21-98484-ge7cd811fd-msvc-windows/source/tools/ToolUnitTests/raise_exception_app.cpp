/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 * This application verifies that Pin correctly emulates exceptions when the 
 * application attempts to execute an invalid or inaccessible instruction.
 */

#include <string>
#include <iostream>
#include "../Utils/sys_memory.h"

#if defined(TARGET_WINDOWS)
#include "windows.h"
#define EXPORT_CSYM extern "C" __declspec(dllexport)
#else
#error Unsupported OS
#endif
using std::cerr;
using std::endl;
using std::hex;
using std::string;

//==========================================================================
// Printing utilities
//==========================================================================
string UnitTestName("raise_exception");
string FunctionTestName;

static void StartFunctionTest(const string& functionTestName)
{
    if (FunctionTestName != "")
    {
        cerr << UnitTestName << "[" << FunctionTestName << "] Success" << endl;
    }
    FunctionTestName = functionTestName;
}

static void ExitUnitTest()
{
    if (FunctionTestName != "")
    {
        cerr << UnitTestName << "[" << FunctionTestName << "] Success" << endl;
    }
    cerr << UnitTestName << " : Completed successfully" << endl;
    exit(0);
}

static void Abort(const string& msg)
{
    cerr << UnitTestName << "[" << FunctionTestName << "] Failure: " << msg << endl;
    exit(1);
}

//==========================================================================
// Exception handling utilities
//==========================================================================

/*!
 * Exception filter for the ExecuteSafe function: copy the exception record 
 * to the specified structure.
 * @param[in] exceptPtr        pointers to the exception context and the exception 
 *                             record prepared by the system
 * @param[in] pExceptRecord    pointer to the structure that receives the 
 *                             exception record
 * @return the exception disposition
 */
static int SafeExceptionFilter(LPEXCEPTION_POINTERS exceptPtr, EXCEPTION_RECORD* pExceptRecord)
{
    *pExceptRecord = *(exceptPtr->ExceptionRecord);
    return EXCEPTION_EXECUTE_HANDLER;
}

/*!
 * Execute the specified function and return to the caller even if the function raises 
 * an exception.
 * @param[in] pExceptRecord    pointer to the structure that receives the 
 *                             exception record if the function raises an exception
 * @return TRUE, if the function raised an exception
 */
template< typename FUNC > bool ExecuteSafe(FUNC fp, EXCEPTION_RECORD* pExceptRecord)
{
    __try
    {
        fp();
        return false;
    }
    __except (SafeExceptionFilter(GetExceptionInformation(), pExceptRecord))
    {
        return true;
    }
}

template< typename FUNC, typename ARG > bool ExecuteSafeWithArg(FUNC fp, ARG arg, EXCEPTION_RECORD* pExceptRecord)
{
    __try
    {
        fp(arg);
        return false;
    }
    __except (SafeExceptionFilter(GetExceptionInformation(), pExceptRecord))
    {
        return true;
    }
}

/*!
 * The tool intercepts this function and raises an EXCEPTION_X87_OVERFLOW exception. 
 */
EXPORT_CSYM void RaiseX87OverflowException() { ; }

/*!
* The tool intercepts this function and raises the specified system exception. 
*/
EXPORT_CSYM void RaiseSystemException(unsigned sysExceptCode) { ; }

/*!
 * Check to see if the specified exception record represents an exception with the 
 * specified exception code.
 */
static bool CheckExceptionCode(EXCEPTION_RECORD* pExceptRecord, unsigned exceptCode)
{
    if (pExceptRecord->ExceptionCode != exceptCode)
    {
        cerr << "Unexpected exception code " << hex << pExceptRecord->ExceptionCode << ". Should be " << hex << exceptCode
             << endl;
        return false;
    }
    return true;
}

/*!
 * Check to see if the specified exception record represents an exception with the 
 * specified values of exception code, exception address, access type and access address.
 */
static bool CheckExceptionInfo(EXCEPTION_RECORD* pExceptRecord, unsigned exceptCode, void* exceptAddr, unsigned accessType,
                               void* accessAddr)
{
    if (!CheckExceptionCode(pExceptRecord, exceptCode))
    {
        return false;
    }

    if (pExceptRecord->ExceptionAddress != exceptAddr)
    {
        cerr << "Unexpected exception address " << hex << (ULONG_PTR)(pExceptRecord->ExceptionAddress) << ". Should be " << hex
             << (ULONG_PTR)exceptAddr << endl;
        return false;
    }

    if ((pExceptRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION) && (pExceptRecord->NumberParameters >= 2))
    {
        if (pExceptRecord->ExceptionInformation[0] != (ULONG_PTR)accessType)
        {
            cerr << "Unexpected access type " << hex << pExceptRecord->ExceptionInformation[0] << ". Should be " << hex
                 << accessType << endl;
            return false;
        }
        if (pExceptRecord->ExceptionInformation[1] != (ULONG_PTR)accessAddr)
        {
            cerr << "Unexpected address of the faulty access " << hex << pExceptRecord->ExceptionInformation[1] << ". Should be "
                 << hex << (ULONG_PTR)accessAddr << endl;
            return false;
        }
    }
    return true;
}

/*!
 * Convert a pointer to <SRC> type into a pointer to <DST> type.
 * Allows any combination of data/function types.
 */
#if defined(TARGET_IA32) || defined(TARGET_IA32E)
template< typename DST, typename SRC > DST* CastPtr(SRC* src)
{
    union CAST
    {
        DST* dstPtr;
        SRC* srcPtr;
    } cast;
    cast.srcPtr = src;
    return cast.dstPtr;
}
#else
#error Unsupported architechture
#endif

typedef void FUNC_NOARGS();

/*!
 * The main procedure of the application.
 */
int main(int argc, char* argv[])
{
    FUNC_NOARGS* fpExcept;
    EXCEPTION_RECORD exceptRecord;
    bool exceptionCaught;

    // Allocate two adjacent pages. First page has all access rights
    // and the second page is inaccessible.
    size_t pageSize = GetPageSize();
    char* pages     = (char*)MemAlloc(2 * pageSize, MEM_READ_WRITE_EXEC);
    if (pages == 0)
    {
        Abort("MemAlloc failed");
    }
    BOOL protectStatus = MemProtect(pages + pageSize, pageSize, MEM_INACESSIBLE);
    if (!protectStatus)
    {
        Abort("MemProtect failed");
    }

    // CALL NULL
    StartFunctionTest("Indirect call to NULL address");
    fpExcept        = 0;
    exceptionCaught = ExecuteSafe(fpExcept, &exceptRecord);
    if (!exceptionCaught)
    {
        Abort("Unhandled exception");
    }
    if (!CheckExceptionInfo(&exceptRecord, EXCEPTION_ACCESS_VIOLATION, 0, 0, 0))
    {
        Abort("Incorrect exception information");
    }

    // CALL inaccessible address
    StartFunctionTest("Indirect call to inaccessible address");
    fpExcept        = CastPtr< FUNC_NOARGS >(pages + pageSize);
    exceptionCaught = ExecuteSafe(fpExcept, &exceptRecord);
    if (!exceptionCaught)
    {
        Abort("Unhandled exception");
    }
    if (!CheckExceptionInfo(&exceptRecord, EXCEPTION_ACCESS_VIOLATION, pages + pageSize, 0, pages + pageSize))
    {
        Abort("Incorrect exception information");
    }

    // Partially accessible instruction
    // Note, the exception address and the address of the failed access are different.
    StartFunctionTest("Partially accessible instruction");
#if defined(TARGET_IA32) || defined(TARGET_IA32E)
    pages[pageSize - 1] = char(0xCD); // First byte of the INT <n> instruction
#endif
    fpExcept        = CastPtr< FUNC_NOARGS >(pages + pageSize - 1);
    exceptionCaught = ExecuteSafe(fpExcept, &exceptRecord);
    if (!exceptionCaught)
    {
        Abort("Unhandled exception");
    }
    if (!CheckExceptionInfo(&exceptRecord, EXCEPTION_ACCESS_VIOLATION, pages + pageSize - 1, 0, pages + pageSize))
    {
        Abort("Incorrect exception information");
    }

    // Invalid instruction
    StartFunctionTest("Invalid instruction");
#if defined(TARGET_IA32) || defined(TARGET_IA32E)
    pages[0] = char(0xC4); // invalid opcode
    pages[1] = char(0xC4);
#endif
    fpExcept        = CastPtr< FUNC_NOARGS >(pages);
    exceptionCaught = ExecuteSafe(fpExcept, &exceptRecord);
    if (!exceptionCaught)
    {
        Abort("Unhandled exception");
    }
    if (!CheckExceptionInfo(&exceptRecord, EXCEPTION_ILLEGAL_INSTRUCTION, pages, 0, 0))
    {
        Abort("Incorrect exception information");
    }

    // Invalid instruction - too many prefixes
    StartFunctionTest("Invalid instruction2");
#if defined(TARGET_IA32) || defined(TARGET_IA32E)
    for (int i = 0; i < 15 /* maximum # of bytes in instruction */; i++)
    {
        pages[i] = char(0x3e); // this is a segment prefix
    }
#endif
    fpExcept        = CastPtr< FUNC_NOARGS >(pages);
    exceptionCaught = ExecuteSafe(fpExcept, &exceptRecord);
    if (!exceptionCaught)
    {
        Abort("Unhandled exception");
    }
    if (!CheckExceptionInfo(&exceptRecord, EXCEPTION_ILLEGAL_INSTRUCTION, pages, 0, 0))
    {
        Abort("Incorrect exception information");
    }

    // Raise FP exception in the tool
    StartFunctionTest("Raise FP exception in the tool");
    exceptionCaught = ExecuteSafe(RaiseX87OverflowException, &exceptRecord);
    if (!exceptionCaught)
    {
        Abort("Unhandled exception");
    }
    if (!CheckExceptionCode(&exceptRecord, EXCEPTION_FLT_OVERFLOW))
    {
        Abort("Incorrect exception information");
    }

    // Raise a system-specific exception in the tool
    StartFunctionTest("Raise system-specific exception in the tool");
    exceptionCaught = ExecuteSafeWithArg(RaiseSystemException, EXCEPTION_INVALID_HANDLE, &exceptRecord);
    if (!exceptionCaught)
    {
        Abort("Unhandled exception");
    }
    if (!CheckExceptionCode(&exceptRecord, EXCEPTION_INVALID_HANDLE))
    {
        Abort("Incorrect exception information");
    }

    ExitUnitTest();
}
