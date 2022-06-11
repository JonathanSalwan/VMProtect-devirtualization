/*
 * Copyright (C) 2007-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <windows.h>
#include <stdio.h>
#include <assert.h>

//=======================================================================
// This is the application for testing the syscall_std tool.
// It uses two different system call standards on ia32: INT2E and sysenter.
// On intel64 it always uses syscall instruction to invoke system call.
//=======================================================================

typedef BOOL WINAPI IsWow64Process_T(HANDLE, PBOOL);
typedef LONG NTSTATUS, *PNTSTATUS;
typedef NTSTATUS __stdcall NtAllocateVirtualMemory_T(HANDLE processHandle, PVOID* baseAddress, ULONG_PTR zeroBits,
                                                     PSIZE_T regionSize, ULONG allocationType, ULONG protect);

NtAllocateVirtualMemory_T* pfnNtAllocateVirtualMemory = 0;
UINT32 ntAllocateNumber                               = 0;

//=======================================================================
// Return TRUE for native 32-bit OS
BOOL IsNativeOs32()
{
    BOOL isNativeOs32 = FALSE;
    if (sizeof(void*) == 4)
    {
        static IsWow64Process_T* fnIsWow64Process =
            (IsWow64Process_T*)GetProcAddress(GetModuleHandle("kernel32"), "IsWow64Process");

        BOOL isWow64 = FALSE;
        if (fnIsWow64Process != 0)
        {
            fnIsWow64Process(GetCurrentProcess(), &isWow64);
        }
        isNativeOs32 = !isWow64;
    }

    return isNativeOs32;
}

BOOL isNativeOs32 = IsNativeOs32();

//=======================================================================
// Allocate memory region using FAST or ALT system call standard
// If syscallNum is non-zero and we are running on ia32 - use ALT system call
// Otherwise - use FAST system call
extern "C" __declspec(dllexport) PVOID
    My_AllocateVirtualMemory(HANDLE processHandle, SIZE_T size, ULONG protect, UINT32 syscallNum)
{
    PVOID baseAddress    = 0;
    PVOID* pBaseAddress  = &baseAddress;
    SIZE_T regionSize    = size;
    SIZE_T* pRegionSize  = &regionSize;
    ULONG allocationType = MEM_RESERVE | MEM_COMMIT;

    BOOL done = FALSE;
    if (isNativeOs32 && (syscallNum != 0))
    {
#ifndef _WIN64
        _asm
        {
                push    protect;
                push    allocationType;
                push    pRegionSize;
                push    0;
                push    pBaseAddress;
                push    processHandle;
                mov     edx, esp
                mov     eax, syscallNum;
          // change ESP arbitrarily to check that it is not used to retrieve
          // ALT syscall arguments
                sub     esp, 0x10; 
                int     0x2e;
                add     esp, 0x10;

                add     esp, 0x18;
        }
        done = TRUE;
#endif
    }
    if (!done)
    {
        pfnNtAllocateVirtualMemory(processHandle, pBaseAddress, 0, pRegionSize, allocationType, protect);
    }
    return baseAddress;
}

//=======================================================================
// Set syscall number for ALT syscall to NtAllocateVirtualMemory.
// To be called by a tool
extern "C" __declspec(dllexport) void My_SetNtAllocateNumber(UINT32 num) { ntAllocateNumber = num; }

//=======================================================================

int main()
{
    pfnNtAllocateVirtualMemory =
        (NtAllocateVirtualMemory_T*)GetProcAddress(GetModuleHandle("ntdll.dll"), "NtAllocateVirtualMemory");
    assert(pfnNtAllocateVirtualMemory != 0);

    printf("Before FAST call to NtAllocateVirtualMemory\n");
    fflush(stdout);
    My_AllocateVirtualMemory(GetCurrentProcess(), 777, PAGE_READWRITE, 0);
    printf("After FAST call to NtAllocateVirtualMemory\n");
    fflush(stdout);

    printf("Before ALT call to NtAllocateVirtualMemory\n");
    fflush(stdout);
    My_AllocateVirtualMemory(GetCurrentProcess(), 777, PAGE_READWRITE, ntAllocateNumber);
    printf("After ALT call to NtAllocateVirtualMemory\n");
    fflush(stdout);

    return 0;
}
