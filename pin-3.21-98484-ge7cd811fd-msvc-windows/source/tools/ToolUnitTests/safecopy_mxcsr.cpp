/*
 * Copyright (C) 2007-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 *  Tool that tests the PIN_SafeCopy() function does not affect the mxcsr. 
 */

#include "pin.H"
#include <os-apis.h>
#include <iostream>
#include <fstream>
#include <string.h>
#include <unistd.h>
using std::endl;
using std::string;

// The original test value was 0x9fc0. However, due to a bug in Linux kernels after 2.6.32, the DAZ bit is not restored
// properly when returning from the kernel after a signal handler. So the test value was changed to 0x9f80.
const int mxscrTestValue = 0x9f80;
extern "C" void SetMxcsr(const int* mxcsrVal);
extern "C" void GetMxcsr(int* mxcsrVal);

/* ================================================================== */
// Global variables
/* ================================================================== */
KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "safecopy.out", "specify output file name");

std::ofstream out;

VOID* probeAddr = 0; // base address of a range overwritten by a probe

/* ================================================================== */
// Utilities
/* ================================================================== */

VOID* MemAlloc(size_t size)
{
    void* pageFrameStart = NULL;
    OS_AllocateMemory(NATIVE_PID_CURRENT, OS_PAGE_PROTECTION_TYPE_READ | OS_PAGE_PROTECTION_TYPE_WRITE, size,
                      OS_MEMORY_FLAGS_PRIVATE, &pageFrameStart);
    return pageFrameStart;
}

BOOL MemProtect(VOID* addr, size_t size, BOOL enableAccess)
{
    UINT prot = enableAccess ? (OS_PAGE_PROTECTION_TYPE_READ | OS_PAGE_PROTECTION_TYPE_WRITE) : OS_PAGE_PROTECTION_TYPE_NOACCESS;
    OS_RETURN_CODE result = OS_ProtectMemory(NATIVE_PID_CURRENT, const_cast< void* >(addr), size, prot);
    return OS_RETURN_CODE_IS_SUCCESS(result);
}

VOID MemFree(VOID* addr, size_t size) { OS_FreeMemory(NATIVE_PID_CURRENT, addr, size); }

/*!
 * Test the PIN_SafeCopy() function in the following scenarios:
 * A. Successful copy of an entire memory region
 * B. Partial copy of a memory region, whose tail is inaccessible
 * C. Failure to copy an inaccessible memory region
 */
VOID SafeCopyTest()
{
    size_t pageSize = getpagesize();

    CHAR* src = (CHAR*)MemAlloc(2 * pageSize);
    ASSERTX(src != 0);
    CHAR* srcBuf = src + 1; // +1 for testing unaligned access

    CHAR* dst = (CHAR*)MemAlloc(2 * pageSize);
    ASSERTX(dst != 0);
    CHAR* dstBuf = dst + 1; // +1 for testing unaligned access

    size_t bufSize     = 2 * pageSize - 1;
    size_t halfBufSize = pageSize - 1;
    size_t copySize;

    //A.
    for (unsigned int i = 0; i < bufSize; ++i)
    {
        src[i] = i / 256;
        dst[i] = 0;
    }
    int mxcsrValueBeforeSafeCopy;
    GetMxcsr(&mxcsrValueBeforeSafeCopy);
    SetMxcsr(&mxscrTestValue);
    copySize = PIN_SafeCopy(dstBuf, srcBuf, bufSize);
    int mxcsrValueAfterSafeCopy;
    GetMxcsr(&mxcsrValueAfterSafeCopy);
    SetMxcsr(reinterpret_cast< const int* >(&mxcsrValueBeforeSafeCopy));
    ASSERT((mxscrTestValue == mxcsrValueAfterSafeCopy), "SafeCopy (A) failed due to unexpected mxcsr value after SafeCopy.\n");
    ASSERT(((copySize == bufSize) && (memcmp(dstBuf, srcBuf, bufSize) == 0)), "SafeCopy (A) failed.\n");
    out << "SafeCopy (A): Entire buffer has been copied successfully." << endl;

    //B.
    for (unsigned int i = 0; i < pageSize; ++i)
    {
        dst[i] = 0;
    }
    MemProtect(src + pageSize, pageSize, FALSE); // second half of src is inaccessible
    copySize = PIN_SafeCopy(dstBuf, srcBuf, bufSize);
    ASSERT(((copySize == halfBufSize) && (memcmp(dstBuf, srcBuf, halfBufSize) == 0)), "SafeCopy (B) failed.\n");

    // Check to see that all accessible bytes near the end of the first page are copied successfully
    for (unsigned int sz = 1; sz < 16; ++sz)
    {
        for (unsigned int i = 0; i < sz; ++i)
        {
            dstBuf[i] = 0;
        }

        GetMxcsr(&mxcsrValueBeforeSafeCopy);
        SetMxcsr(&mxscrTestValue);
        copySize = PIN_SafeCopy(dstBuf, src + pageSize - sz, pageSize);
        GetMxcsr(&mxcsrValueAfterSafeCopy);
        SetMxcsr(reinterpret_cast< const int* >(&mxcsrValueBeforeSafeCopy));
        ASSERT((mxscrTestValue == mxcsrValueAfterSafeCopy),
               "SafeCopy (B) failed due to unexpected mxcsr value after SafeCopy.\n");
        ASSERT(((copySize == sz) && (memcmp(dstBuf, src + pageSize - sz, sz) == 0)), "SafeCopy (B) failed.\n");
    }

    out << "SafeCopy (B): Accessible part of the buffer has been copied successfully." << endl;

    //C.
    MemProtect(dst, pageSize, FALSE); // dst is inaccessible
    GetMxcsr(&mxcsrValueBeforeSafeCopy);
    SetMxcsr(&mxscrTestValue);
    copySize = PIN_SafeCopy(dstBuf, srcBuf, bufSize);
    GetMxcsr(&mxcsrValueAfterSafeCopy);
    SetMxcsr(reinterpret_cast< const int* >(&mxcsrValueBeforeSafeCopy));
    ASSERT((mxscrTestValue == mxcsrValueAfterSafeCopy), "SafeCopy (C) failed due to unexpected mxcsr value after SafeCopy.\n");
    ASSERT((copySize == 0), "SafeCopy (C) failed.\n");
    out << "SafeCopy (C): Inaccessible buffer has not been copied." << endl;

    MemFree(src, 2 * pageSize);
    MemFree(dst, 2 * pageSize);
}

/*!
 * Test SafeCopy in the trace analysis routine
 */
VOID SafeCopyTestInTrace()
{
    static BOOL first = TRUE;
    if (first)
    {
        first = FALSE;
        out << "Test SafeCopy in the trace analysis routine." << endl;
        SafeCopyTest();
    }
}

/*!
 * Test SafeCopy in the trace instrumentation callback
 */
VOID Trace(TRACE trace, VOID* v)
{
    static BOOL first = TRUE;
    if (first)
    {
        first = FALSE;
        out << "Test SafeCopy in the trace instrumentation callback." << endl;
        SafeCopyTest();
        TRACE_InsertCall(trace, IPOINT_BEFORE, (AFUNPTR)SafeCopyTestInTrace, IARG_END);
    }

    // Verify that pin-inserted probes are not visible through PIN_SafeCopy
    if (probeAddr != 0)
    {
        out << "Test SafeCopy in a region overwritten by probe." << endl;
        CHAR buffer[8];
        int mxcsrValueBeforeSafeCopy, mxcsrValueAfterSafeCopy;
        GetMxcsr(&mxcsrValueBeforeSafeCopy);
        SetMxcsr(&mxscrTestValue);
        size_t copySize = PIN_SafeCopy(buffer, probeAddr, sizeof(buffer));
        GetMxcsr(&mxcsrValueAfterSafeCopy);
        SetMxcsr(reinterpret_cast< const int* >(&mxcsrValueBeforeSafeCopy));
        ASSERT((mxscrTestValue == mxcsrValueAfterSafeCopy),
               "SafeCopy failed in a region overwritten by probe due to unexpected mxcsr value after SafeCopy.\n");
        ASSERT((copySize == sizeof(buffer)), "SafeCopy failed in a region overwritten by probe.\n");
        ASSERT((memcmp(buffer, probeAddr, copySize) != 0), "Pin inserted probes are visible through SafeCopy.\n");
        probeAddr = 0;
    }
}

/*!
 * Image instrumentation callback
 */
VOID ImageLoad(IMG img, VOID* v)
{
#if defined(TARGET_WINDOWS)
    // Pin on Windows inserts a probe that intercepts APCs at KiApcUserDispatcher
    RTN rtn = RTN_FindByName(img, "KiUserApcDispatcher");
    if (RTN_Valid(rtn))
    {
        probeAddr = (VOID*)RTN_Address(rtn);
    }
#endif
}

VOID Fini(INT32 code, VOID* v) { out << "SafeCopy test completed." << endl; }

/*!
 * The main procedure of the tool.
 */
int main(int argc, char* argv[])
{
    PIN_InitSymbols();
    PIN_Init(argc, argv);

    out.open(KnobOutputFile.Value().c_str());

    // Register function to be called to instrument traces
    TRACE_AddInstrumentFunction(Trace, 0);

    // Register function to be called to instrument images
    IMG_AddInstrumentFunction(ImageLoad, 0);

    // Register function to be called when the application exits
    PIN_AddFiniFunction(Fini, 0);

    // Start the program, never returns
    PIN_StartProgram();

    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
