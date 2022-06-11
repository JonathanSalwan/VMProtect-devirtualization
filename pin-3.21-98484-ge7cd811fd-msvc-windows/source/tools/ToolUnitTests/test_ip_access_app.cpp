/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
This tests the ability to handle the [REG_INST_PTR] memory operand (instruction pointer is base register
and no offset or index register). Also the ability to get the register value of the REG_INST_PTR
*/

#include <cassert>
#include <cstdio>
#include <cstring>
#include <windows.h>
#include <malloc.h>
#include <signal.h>
#include <cstdlib>

#if defined(__cplusplus)
extern "C"
#endif
    void
    TestIpRead();
#if defined(__cplusplus)
extern "C"
#endif
    void
    TestIpWrite();
#if defined(__cplusplus)
extern "C"
#endif
    void
    Dummy();

typedef void (*MY_FUNC_PTR)(void);
typedef union
{
    MY_FUNC_PTR codePtr;
    char* dataPtr;
} MY_FUNC_PTR_CAST;

const size_t MAX_FUNC_SIZE = 8192;
/*!
 * Return size of the specified (foo or bar) routine
 */
size_t FuncSize(MY_FUNC_PTR func, MY_FUNC_PTR funcEnd)
{
    MY_FUNC_PTR_CAST cast;

    cast.codePtr      = func;
    const char* start = cast.dataPtr;

    cast.codePtr    = funcEnd;
    const char* end = cast.dataPtr;

    assert(end > start);
    assert(end - start <= MAX_FUNC_SIZE);
    return end - start;
}

/*!
 * Copy the TestIpWrite routine into a data buffer - because it contains a 
   mov [ip], 0x90
   and this write will cause an access violation if executed in the code segment
 */
void CopyAndExecuteTestIpWrite()
{
    static void* codeBuffer = NULL;
    size_t size             = FuncSize(TestIpWrite, Dummy);

    //get the page size
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    unsigned int iPageSize = si.dwPageSize;
    //This needs to be page aligned in order to be able to change the execution rights
    codeBuffer = _aligned_malloc(size, iPageSize);

    MY_FUNC_PTR_CAST cast;
    cast.codePtr = TestIpWrite;

    const void* funcAddr = cast.dataPtr;
    memcpy(codeBuffer, funcAddr, size);

    cast.dataPtr         = static_cast< char* >(codeBuffer);
    MY_FUNC_PTR funcCopy = cast.codePtr;

    //Change protections in order to execute and change on staticBuffer
    DWORD orig_protection = 0;
    if (VirtualProtect(codeBuffer, iPageSize, PAGE_EXECUTE_READWRITE, &orig_protection))
    {
        printf("Execution protection enabled\n");
    }
    else
    {
        printf("errno: %d:%s\n", errno, strerror(errno));
        printf("Could not set protections\n");
        exit(-1);
    }

    funcCopy();
}

int main(int argc, char* argv[])
{
    TestIpRead();
    CopyAndExecuteTestIpWrite();
    return 0;
}
