/*
 * Copyright (C) 2007-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include "pin.H"
#include <string>
#include <iostream>
#include <assert.h>

//=======================================================================
// This is a tool that instruments Windows system calls invoked in different
// calling standards : FAST and ALT.
//=======================================================================

using std::cout;
using std::dec;
using std::endl;
using std::hex;

typedef void My_SetNtAllocateNumber_T(UINT32 num);
My_SetNtAllocateNumber_T* pfnMy_SetNtAllocateNumber = 0;
ADDRINT pfnMy_AllocateVirtualMemory                 = 0;
BOOL inMy_AllocateVirtualMemory                     = FALSE;

//=======================================================================
// Print syscall number and arguments
VOID BeforeSyscall(ADDRINT ip, ADDRINT num, ADDRINT arg0, ADDRINT arg1, ADDRINT arg2, ADDRINT arg3, ADDRINT arg4, ADDRINT arg5)
{
    if (inMy_AllocateVirtualMemory)
    {
        inMy_AllocateVirtualMemory = FALSE;
        cout << "NtAllocateVirtualMemory: ";
        cout << " size = " << dec << *((size_t*)arg3);
        cout << " allocation type = " << hex << (unsigned long)arg4;
        cout << " protect = " << hex << (unsigned long)arg5 << endl;

        if (pfnMy_SetNtAllocateNumber)
        {
            pfnMy_SetNtAllocateNumber((UINT32)num);
            cout << "Set NtAllocateVirtualMemory system call number" << endl;
            pfnMy_SetNtAllocateNumber = 0; // call My_SetNtAllocateNumber only once
        }
    }
}

//=======================================================================
// Mark start of the My_AllocateVirtualMemory function
VOID BeforeMyAlloc() { inMy_AllocateVirtualMemory = TRUE; }

//=======================================================================
// This function is called for every instruction and instruments syscalls
VOID Instruction(INS ins, VOID* v)
{
    if (INS_IsSyscall(ins))
    {
        // Arguments and syscall number is only available before
        INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(BeforeSyscall), IARG_INST_PTR, IARG_SYSCALL_NUMBER, IARG_SYSARG_VALUE, 0,
                       IARG_SYSARG_VALUE, 1, IARG_SYSARG_VALUE, 2, IARG_SYSARG_VALUE, 3, IARG_SYSARG_VALUE, 4, IARG_SYSARG_VALUE,
                       5, IARG_END);
    }
    if (INS_Address(ins) == pfnMy_AllocateVirtualMemory)
    {
        INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(BeforeMyAlloc), IARG_END);
    }
}

//=======================================================================
// This function is called for every image. It retrieves addresses of
// My_AllocateVirtualMemory and My_SetNtAllocateNumber functions in the
// application.

VOID ImageLoad(IMG img, VOID* v)
{
    RTN allocRtn = RTN_FindByName(img, "My_AllocateVirtualMemory");
    if (RTN_Valid(allocRtn))
    {
        pfnMy_AllocateVirtualMemory = RTN_Address(allocRtn);
    }

    RTN setNumberRtn = RTN_FindByName(img, "My_SetNtAllocateNumber");
    if (RTN_Valid(setNumberRtn))
    {
        pfnMy_SetNtAllocateNumber = (My_SetNtAllocateNumber_T*)RTN_Address(setNumberRtn);
    }
}

//=======================================================================
int main(int argc, CHAR* argv[])
{
    PIN_InitSymbols();
    PIN_Init(argc, argv);
    IMG_AddInstrumentFunction(ImageLoad, 0);
    INS_AddInstrumentFunction(Instruction, 0);
    PIN_StartProgram();

    return 0;
}
