/*
 * Copyright (C) 2007-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <Windows.h>
#include <iostream>

using std::cout;
using std::endl;
using std::string;

// This application must be compiled with /INCREMENTAL:NO

// Don't change Function location!
int Return10() { return 10; }
void EndReturn10() {}

int Return20() { return 20; }
void EndReturn20() {}
/////////////////////////////////

typedef int (*FUNCTION_PROTO)();

size_t PtrDiff(const VOID* ptr1, const VOID* ptr2)
{
    return (reinterpret_cast< const char* >(ptr1) - reinterpret_cast< const char* >(ptr2));
}

int main()
{
    size_t functionSize1  = PtrDiff((const VOID*)EndReturn10, (const VOID*)Return10);
    size_t functionSize2  = PtrDiff((const VOID*)EndReturn20, (const VOID*)Return20);
    size_t allocationSize = max(functionSize1, functionSize2);
    LPVOID address        = VirtualAlloc(NULL, allocationSize, MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (address == NULL)
    {
        cout << "Failed to reserve memory" << endl;
        exit(0);
    }

    // First
    {
        LPVOID address1 = VirtualAlloc(address, allocationSize, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
        if (address1 == NULL || address1 != address)
        {
            cout << "Failed to commit memory at first" << endl;
            exit(0);
        }
        memcpy(address1, (const VOID*)Return10, functionSize1);
        FUNCTION_PROTO pReturn10 = (FUNCTION_PROTO)address1;
        cout << "First function return value = " << pReturn10() << endl;
        VirtualFree(address1, allocationSize, MEM_DECOMMIT);
    }

    // Second
    {
        LPVOID address2 = VirtualAlloc(address, allocationSize, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
        if (address2 == NULL || address2 != address)
        {
            cout << "Failed to commit memory at second" << endl;
            exit(0);
        }
        memcpy(address2, (const VOID*)Return20, functionSize2);
        FUNCTION_PROTO pReturn20 = (FUNCTION_PROTO)address2;
        cout << "Second function return value = " << pReturn20() << endl;
        VirtualFree(address2, allocationSize, MEM_DECOMMIT);
    }

    VirtualFree(address, 0x0, MEM_RELEASE);
    return 0;
}
