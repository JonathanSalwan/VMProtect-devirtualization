/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 * This application verifies that Pin correctly emulates exceptions when the 
 * application attempts to execute <int N> instruction.
 */

#include <string>
#include <iostream>
using std::cerr;
using std::endl;
using std::string;

#if defined(TARGET_WINDOWS)
#include "windows.h"
#define EXPORT_CSYM extern "C" __declspec(dllexport)
#else
#error Unsupported OS
#endif

size_t GetPageSize()
{
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    return (size_t)(sysInfo.dwPageSize);
}

void* MemAlloc(size_t size) { return VirtualAlloc(0, size, MEM_COMMIT, PAGE_EXECUTE_READWRITE); }

volatile int doloop = 1;

EXPORT_CSYM int DoLoop() { return doloop; }

//==========================================================================
// Printing utilities
//==========================================================================
#define PRINT 0

string UnitTestName("sw_interrupt");
string FunctionTestName;

static void StartFunctionTest(const string& functionTestName)
{
#if PRINT
    if (FunctionTestName != "")
    {
        cerr << UnitTestName << "[" << FunctionTestName << "] Success" << endl;
    }
    FunctionTestName = functionTestName;
#endif
}

static void ExitUnitTest()
{
#if PRINT
    if (FunctionTestName != "")
    {
        cerr << UnitTestName << "[" << FunctionTestName << "] Success" << endl;
    }
    cerr << UnitTestName << " : Completed successfully" << endl;
#endif
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
 * @return IP register value in the given exception context
 */
#if defined(TARGET_IA32)
static ULONG_PTR GetIp(CONTEXT* pExceptContext) { return pExceptContext->Eip; }
#elif defined(TARGET_IA32E)
static ULONG_PTR GetIp(CONTEXT* pExceptContext) { return pExceptContext->Rip; }
#else
#error Unsupported architechture
#endif

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
    *pExceptRecord  = *(exceptPtr->ExceptionRecord);
    *pExceptContext = *(exceptPtr->ContextRecord);
    return EXCEPTION_EXECUTE_HANDLER;
}

/*!
 * Execute the specified function and return to the caller even if the function raises 
 * an exception.
 * @param[out] pExceptRecord   pointer to the structure that receives the 
 *                             exception record if the function raises an exception
 * @param[out] pExceptContext  pointer to the structure that receives the 
 *                             exception context if the function raises an exception
 * @return TRUE, if the function raised an exception
 */
template< typename FUNC > bool ExecuteSafe(FUNC fp, EXCEPTION_RECORD* pExceptRecord, CONTEXT* pExceptContext)
{
    __try
    {
        fp();
        return false;
    }
    __except (SafeExceptionFilter(GetExceptionInformation(), pExceptRecord, pExceptContext))
    {
#if PRINT
        cerr << "Exception handler: "
             << "Exception code " << hex << pExceptRecord->ExceptionCode << "."
             << "Exception address " << hex << (ULONG_PTR)(pExceptRecord->ExceptionAddress) << "."
             << "Context IP " << hex << GetIp(pExceptContext) << "." << endl;
#endif
        return true;
    }
}

/*!
 * Check to see if the specified exception record represents an exception with the 
 * specified exception code.
 */
static void CheckExceptionCode(EXCEPTION_RECORD* pExceptRecord, unsigned exceptCode)
{
    if (pExceptRecord->ExceptionCode != exceptCode)
    {
        Abort("Incorrect exception code");
    }
}

/*!
 * Check to see if the specified exception record represents an exception that occurred
 * in the specified instruction. The exception could be a trap reported after the 
 * instruction.
 */
static void CheckExceptionAddr(EXCEPTION_RECORD* pExceptRecord, void* insPtr, size_t insSize)
{
    ULONG_PTR exceptAddress = (ULONG_PTR)(pExceptRecord->ExceptionAddress);
    ULONG_PTR insStart      = (ULONG_PTR)insPtr;
    ULONG_PTR insEnd        = insStart + insSize;

    if ((exceptAddress < insStart) || (exceptAddress > insEnd))
    {
        Abort("Incorrect exception address");
    }
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

//==========================================================================
// Software interrupts
//==========================================================================

const unsigned char INT_OPCODE        = 0xCD;
const unsigned char INT1_OPCODE       = 0xF1;
const unsigned char INT3_OPCODE       = 0xCC;
const unsigned char INTO_OPCODE       = 0xCE;
const unsigned char RET_OPCODE        = 0xC3;
const unsigned char POPF_OPCODE       = 0x9D;
const unsigned char PUSHF_OPCODE      = 0x9C;
const unsigned char PUSH_IMM32_OPCODE = 0x68;

union EFLAGS
{
    DWORD dw;
    BYTE byte[4];
};

/*!
 * Generate a procedure that executes <int N> instruction (2-byte encoding).
 * @param[in]  procAddr  location where the procedure should be generated
 * @param[in]  intNum    interrupt number
 * @param[out] pInsPtr   pointer to the the generated interrupt instruction
 * @param[out] pInsSize  size of the interrupt instruction
 */
void GenerateIntN(unsigned char* procAddr, unsigned char intNum, unsigned char** pInsPtr, size_t* pInsSize)
{
    procAddr[0] = INT_OPCODE;
    procAddr[1] = intNum;
    procAddr[2] = RET_OPCODE;
    *pInsPtr    = procAddr;
    *pInsSize   = 2;
}

/*!
 * Generate a procedure that executes <int1> instruction (1-byte encoding).
 * @param[in] procAddr  location where the procedure should be generated
 * @param[out] pInsPtr   pointer to the the generated interrupt instruction
 * @param[out] pInsSize  size of the interrupt instruction
 */
void GenerateInt1(unsigned char* procAddr, unsigned char** pInsPtr, size_t* pInsSize)
{
    procAddr[0] = INT1_OPCODE;
    procAddr[1] = RET_OPCODE;
    *pInsPtr    = procAddr;
    *pInsSize   = 1;
}

/*!
 * Generate a procedure that executes <int3> instruction (1-byte encoding).
 * @param[in] procAddr  location where the procedure should be generated
 * @param[out] pInsPtr   pointer to the the generated interrupt instruction
 * @param[out] pInsSize  size of the interrupt instruction
 */
void GenerateInt3(unsigned char* procAddr, unsigned char** pInsPtr, size_t* pInsSize)
{
    procAddr[0] = INT3_OPCODE;
    procAddr[1] = RET_OPCODE;
    *pInsPtr    = procAddr;
    *pInsSize   = 1;
}

/*!
 * Generate a procedure that executes <intO> instruction.
 * @param[in] procAddr  location where the procedure should be generated
 * @param[out] pInsPtr   pointer to the the generated interrupt instruction
 * @param[out] pInsSize  size of the interrupt instruction
 */
void GenerateIntO(unsigned char* procAddr, unsigned char** pInsPtr, size_t* pInsSize)
{
    EFLAGS overflowFlag;
    overflowFlag.dw = 0xA06;

    procAddr[0] = PUSH_IMM32_OPCODE;
    procAddr[1] = overflowFlag.byte[0];
    procAddr[2] = overflowFlag.byte[1];
    procAddr[3] = overflowFlag.byte[2];
    procAddr[4] = overflowFlag.byte[3];

    procAddr[5] = POPF_OPCODE;

    procAddr[6] = INTO_OPCODE;
    procAddr[7] = RET_OPCODE;

    *pInsPtr  = procAddr + 6;
    *pInsSize = 1;
}

static const int INT_NUM_2_BYTE = 6;
#if defined(TARGET_IA32)
static const int INT_NUM_1_BYTE = 3;
#elif defined(TARGET_IA32E)
static const int INT_NUM_1_BYTE = 2;
#endif
static const int INT_NUM_TOTAL = INT_NUM_2_BYTE + INT_NUM_1_BYTE;

/*!
 * The main procedure of the application.
 */
int main(int argc, char* argv[])
{
    EXCEPTION_RECORD exceptRecord;
    CONTEXT exceptContext;
    bool exceptionCaught;
    unsigned char* insPtr[INT_NUM_TOTAL];
    size_t insSize[INT_NUM_TOTAL];
    FUNC_NOARGS* procAddr[INT_NUM_TOTAL];
    unsigned char* page[INT_NUM_TOTAL];
    size_t pageSize = GetPageSize();

    int i = 0;
    for (i = 0; i < INT_NUM_2_BYTE; i++)
    {
        page[i]     = (unsigned char*)MemAlloc(pageSize);
        procAddr[i] = CastPtr< FUNC_NOARGS >(page[i]);
        GenerateIntN(page[i], i, &insPtr[i], &insSize[i]);
    }

    page[i]     = (unsigned char*)MemAlloc(pageSize);
    procAddr[i] = CastPtr< FUNC_NOARGS >(page[i]);
    GenerateInt1(page[i], &insPtr[i], &insSize[i]);

    i++;
    page[i]     = (unsigned char*)MemAlloc(pageSize);
    procAddr[i] = CastPtr< FUNC_NOARGS >(page[i]);
    GenerateInt3(page[i], &insPtr[i], &insSize[i]);

#if defined(TARGET_IA32)
    i++;
    page[i]     = (unsigned char*)MemAlloc(pageSize);
    procAddr[i] = CastPtr< FUNC_NOARGS >(page[i]);
    GenerateIntO(page[i], &insPtr[i], &insSize[i]);
#endif

    while (DoLoop())
    {
        // int 0
        StartFunctionTest("int 0");
        exceptionCaught = ExecuteSafe(procAddr[0], &exceptRecord, &exceptContext);
        if (!exceptionCaught)
        {
            Abort("Unhandled exception");
        }
        CheckExceptionAddr(&exceptRecord, insPtr[0], insSize[0]);

        // int 1
        StartFunctionTest("int 1");
        exceptionCaught = ExecuteSafe(procAddr[1], &exceptRecord, &exceptContext);
        if (!exceptionCaught)
        {
            Abort("Unhandled exception");
        }
        CheckExceptionAddr(&exceptRecord, insPtr[1], insSize[1]);

        // int 2
        StartFunctionTest("int 2");
        exceptionCaught = ExecuteSafe(procAddr[2], &exceptRecord, &exceptContext);
        if (!exceptionCaught)
        {
            Abort("Unhandled exception");
        }
        CheckExceptionAddr(&exceptRecord, insPtr[2], insSize[2]);

        // int 3
        StartFunctionTest("int 3");
        exceptionCaught = ExecuteSafe(procAddr[3], &exceptRecord, &exceptContext);
        if (!exceptionCaught)
        {
            Abort("Unhandled exception");
        }
        CheckExceptionAddr(&exceptRecord, insPtr[3], insSize[3]);
        CheckExceptionCode(&exceptRecord, EXCEPTION_BREAKPOINT);

        // int 4
        StartFunctionTest("int 4");
        exceptionCaught = ExecuteSafe(procAddr[4], &exceptRecord, &exceptContext);
        if (!exceptionCaught)
        {
            Abort("Unhandled exception");
        }
        CheckExceptionAddr(&exceptRecord, insPtr[4], insSize[4]);
        CheckExceptionCode(&exceptRecord, EXCEPTION_INT_OVERFLOW);

        // int 5
        StartFunctionTest("int 5");
        exceptionCaught = ExecuteSafe(procAddr[5], &exceptRecord, &exceptContext);
        if (!exceptionCaught)
        {
            Abort("Unhandled exception");
        }
        CheckExceptionAddr(&exceptRecord, insPtr[5], insSize[5]);

        // int1
        StartFunctionTest("int1");
        exceptionCaught = ExecuteSafe(procAddr[6], &exceptRecord, &exceptContext);
        if (!exceptionCaught)
        {
            Abort("Unhandled exception");
        }
        CheckExceptionAddr(&exceptRecord, insPtr[6], insSize[6]);
        CheckExceptionCode(&exceptRecord, EXCEPTION_SINGLE_STEP);

        // int3
        StartFunctionTest("int3");
        exceptionCaught = ExecuteSafe(procAddr[7], &exceptRecord, &exceptContext);
        if (!exceptionCaught)
        {
            Abort("Unhandled exception");
        }
        CheckExceptionAddr(&exceptRecord, insPtr[7], insSize[7]);
        CheckExceptionCode(&exceptRecord, EXCEPTION_BREAKPOINT);

#if defined(TARGET_IA32)
        // intO
        StartFunctionTest("intO");
        exceptionCaught = ExecuteSafe(procAddr[8], &exceptRecord, &exceptContext);
        if (!exceptionCaught)
        {
            Abort("Unhandled exception");
        }
        CheckExceptionAddr(&exceptRecord, insPtr[8], insSize[8]);
        CheckExceptionCode(&exceptRecord, EXCEPTION_INT_OVERFLOW);
#endif
    }

    ExitUnitTest();
}
