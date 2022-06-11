/*
 * Copyright (C) 2020-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 * This test made to tests these cases:
 * •   Instruction in the middle of a trace is not executable
 *       o   Instruction in the beginning of the page - test_num = 1
 *       o   Instruction is in the middle of a cross page
 *            •   First executable, second not executable - test_num = 2
 * • First instruction of a trace is not executable
 *       o   Instruction in the beginning of the page - test_num = 3
 *       o   Instruction in the middle of the page - test_num = 4
 *       o   Instruction is in the middle of a cross page
 *           •   First executable, second not executable - test_num = 5
 *           •   First page not executable, second executable - test_num = 6
 */

#define _GNU_SOURCE
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <assert.h>

#if defined(TARGET_WINDOWS)
#include <windows.h>
#include <malloc.h>
#else
#include <sys/mman.h>
#include <unistd.h>
#define EXPORT_SYM extern "C"
#endif
#if !defined(TARGET_WINDOWS)
#if defined(TARGET_MAC)
#include <sys/ucontext.h>
#else
#include <ucontext.h>
#endif
#endif

//this test should make Segmentation fault after iteration 2 out of 4

// this global variable save the address of the instruction that make the exception
char* crashAddr;

#if !defined(TARGET_WINDOWS)
/*!
 * an exception handler for the segmentation fault. and check if the crash address is correct.
 */
void handler(int nSignum, siginfo_t* si, void* vcontext)
{
    ucontext_t* context = (ucontext_t*)vcontext;

    // Print the IP that is stored in RIP (64bit) or EIP (32bit).
    // and check if we got the signal on the correct IP
#if defined(TARGET_MAC)
#ifdef TARGET_IA32
    printf("Address from where crash happen is %x \n", context->uc_mcontext->__ss.__eip);
    assert(crashAddr == (char*)(context->uc_mcontext->__ss.__eip));

#else
    printf("Address from where crash happen is %x \n", context->uc_mcontext->__ss.__rip);
    assert(crashAddr == (char*)(context->uc_mcontext->__ss.__rip));
#endif

#else
#ifdef TARGET_IA32
    printf("Address from where crash happen is %x \n", context->uc_mcontext.gregs[REG_EIP]);
    assert(crashAddr == (char*)(context->uc_mcontext.gregs[REG_EIP]));
#else
    printf("Address from where crash happen is %x \n", context->uc_mcontext.gregs[REG_RIP]);
    assert(crashAddr == (char*)(context->uc_mcontext.gregs[REG_RIP]));
#endif
#endif
    printf("Segmentation fault\n");

    exit(0);
}

#endif

/*!
 * Allocate 1/2 pages and initializing memory space, then change protections.
 * @param[in] content char pointer to the code that it is intended to fill the requested page
 * @param[in] iContentSize size of the code buffer.
 * @param[in] test_num test number to decide which test to run.
 * @return  a pointer to the base of the requested +rwx page(Size iSize).
 */
void* AllocAndFillExecutbalePage(const char* content, int iContentSize, int test_num)
{
    void *vptrAlloc = NULL, *vptrAlloc2 = NULL;
    //get the page size
#if defined(TARGET_WINDOWS)
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    unsigned int iPageSize = si.dwPageSize;
#else
    unsigned int iPageSize = sysconf(_SC_PAGE_SIZE);
#endif

    unsigned int size;
    if (test_num == 3 || test_num == 4)
    {
        size = iPageSize;
    }
    else
    {
        size = iPageSize * 2;
    }
    printf("Requesting a page of %d \n", size);

    //This needs to be page aligned in order to be able to change the execution rights
    //Ask for two pages
#if defined(TARGET_WINDOWS)
    vptrAlloc = _aligned_malloc(size, iPageSize);
#else
    vptrAlloc = mmap(NULL,                               /*addr*/
                     size,                               /*length*/
                     PROT_READ | PROT_WRITE | PROT_EXEC, /*prot*/
                     MAP_PRIVATE | MAP_ANONYMOUS,        /*flags*/
                     -1 /*fd*/, 0                        /*offset*/
    );
#endif

    vptrAlloc2 = vptrAlloc;
    if (vptrAlloc != NULL)
    {
        printf("Succesfully requested the writable page\n");
    }
    else
    {
        printf("Failed to request a writable page\n");
        exit(-1);
    }

    //Initializing memory space
    memset(vptrAlloc, 0x0, size);
    int i         = 0;
    char* cWriter = vptrAlloc;
    if (test_num == 1)
    {
        cWriter += iPageSize - 4;
    }
    if (test_num == 2)
    {
        cWriter += iPageSize - 6;
    }
    if (test_num == 4)
    {
        cWriter += iPageSize - iPageSize / 2;
    }
    if (test_num == 5 || test_num == 6)
    {
        cWriter += iPageSize - 2;
    }

    vptrAlloc = cWriter;
    memcpy((void*)cWriter, (void*)content, iContentSize);
    cWriter += iContentSize;

    //Change protections
#if defined(TARGET_WINDOWS)
    DWORD orig_protection = 0;
    if (test_num == 1 || test_num == 2 || test_num == 5)
    {
        //1st page is executable
        if (VirtualProtect(vptrAlloc2, iPageSize, PAGE_EXECUTE_READ, &orig_protection))
        {
            printf("1st page: Execution protection enabled\n");
        }
        else
        {
            printf("errno: %d:%s\n", errno, strerror(errno));
            printf("Could not set protections\n");
            exit(-1);
        }
        // 2nd page has read protection.
        orig_protection = 0;
        if (VirtualProtect((void*)((char*)vptrAlloc2 + iPageSize), iPageSize, PAGE_READONLY, &orig_protection))
        {
            printf("2nd page: Execution protection disabled\n");
        }
        else
        {
            printf("errno: %d:%s\n", errno, strerror(errno));
            printf("Could not set protections\n");
            exit(-1);
        }
    }
    else
    {
        //tests: 3 4 6
        //1st page has read protection
        if (VirtualProtect(vptrAlloc2, iPageSize, PAGE_READONLY, &orig_protection))
        {
            printf("1st page: Execution protection disabled\n");
        }
        else
        {
            printf("errno: %d:%s\n", errno, strerror(errno));
            printf("Could not set protections \n");
            exit(-1);
        }

        orig_protection = 0;
        if (test_num == 6)
        {
            if (VirtualProtect((void*)((char*)vptrAlloc2 + iPageSize), iPageSize, PAGE_READONLY, &orig_protection))
            {
                printf("2nd page: Execution protection disabled disabled\n");
            }
            else
            {
                printf("errno: %d:%s\n", errno, strerror(errno));
                printf("Could not set protections\n");
                exit(-1);
            }
        }
    }
#else
    if (test_num == 1 || test_num == 2 || test_num == 5)
    {
        //1st page is executable
        if (mprotect(vptrAlloc2, iPageSize, PROT_EXEC | PROT_READ) == 0)
        {
            printf("1st page: Execution protection disabled enabled\n");
        }
        else
        {
            printf("errno: %d:%s\n", errno, strerror(errno));
            printf("Could not set protections\n");
        }

        // 2nd page has read protection.
        if (mprotect(vptrAlloc2 + iPageSize, iPageSize, PROT_READ) == 0)
        {
            printf("2nd page: Execution protection disabled disabled\n");
        }
        else
        {
            printf("errno: %d:%s\n", errno, strerror(errno));
            printf("Could not set protections\n");
            exit(-1);
        }
    }
    else
    {
        //tests: 3 4 6
        //1st page has read protection.
        if (mprotect(vptrAlloc2, iPageSize, PROT_READ) == 0)
        {
            printf("1st page: Execution protection disabled disabled\n");
        }
        else
        {
            printf("errno: %d:%s\n", errno, strerror(errno));
            printf("Could not set protections\n");
            exit(-1);
        }

        if (test_num == 6)
        {
            //2nd page is executable
            if (mprotect(vptrAlloc2 + iPageSize, iPageSize, PROT_EXEC | PROT_READ) == 0)
            {
                printf("2nd page: Execution protection disabled disabled\n");
            }
            else
            {
                printf("errno: %d:%s\n", errno, strerror(errno));
                printf("Could not set protections\n");
                exit(-1);
            }
        }
    }
#endif
    return vptrAlloc;
}

void test(int test_num)
{
    //this is what the char Code[] represent:
    //nopl   0x0(%rax)    0x0F 0x1F 0x40 0x00
    //nopl   0x0(%rax)    0x0F 0x1F 0x40 0x00
    //retq                0xc3
    volatile char Code[] = {0x0F, 0x1F, 0x40, 0x00, 0x0F, 0x1F, 0x40, 0x00, 0xc3};
    void* vPtrRequest;

    //This test made to tests these cases:
    //•   Instruction in the middle of a trace is not executable
    //      o   Instruction in the beginning of the page - test_num = 1
    //      o   Instruction is in the middle of a cross page
    //           •   First executable, second not executable - test_num = 2
    //• First instruction of a trace is not executable
    //      o   Instruction in the beginning of the page - test_num = 3
    //      o   Instruction in the middle of the page - test_num = 4
    //      o   Instruction is in the middle of a cross page
    //           •   First executable, second not executable - test_num = 5
    //           •   First page not executable, second executable - test_num = 6

    vPtrRequest = AllocAndFillExecutbalePage((char*)Code, 9, test_num);

    unsigned int i = 0;
    char* jump     = (char*)vPtrRequest;

    if (test_num == 1 || test_num == 2)
    {
        crashAddr = jump + 4;
    }
    else
    {
        crashAddr = jump;
    }

    ((void (*)(void))jump)();
}

#if defined(TARGET_WINDOWS)
static int MyFilter(unsigned long code, struct _EXCEPTION_POINTERS* ep)
{
    PEXCEPTION_RECORD pExceptRecord = ep->ExceptionRecord;
    printf("Exception %08X\n", code);
    printf("Exception address %08X\n", (ULONG_PTR)(pExceptRecord->ExceptionAddress));
    assert(crashAddr == (char*)(pExceptRecord->ExceptionAddress));
    return EXCEPTION_EXECUTE_HANDLER;
}
#endif

void runTest(int argc, char* argv[])
{
    if (atoi(argv[1]) <= 6)
    {
        test(atoi(argv[1]));
    }
    else
    {
        exit(-1);
    }
}

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        printf("Invalid number of arguments<%d>, expecting only one\n", argc);
        exit(-1);
    }
    //catch exception on Windows.
#if defined(TARGET_WINDOWS)
    __try
    {
        runTest(argc, argv);
    }
    __except (MyFilter(GetExceptionCode(), GetExceptionInformation()))
    {
        printf("Segmentation fault\n");
        return 0;
    }

#else
    //set an exception handler
    struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_flags     = SA_SIGINFO;
    action.sa_sigaction = handler;
#if defined(TARGET_MAC)
    sigaction(SIGBUS, &action, NULL);
#else
    sigaction(SIGSEGV, &action, NULL);
#endif
    runTest(argc, argv);
#endif

    printf("App ended\n");

    return 1;
}
