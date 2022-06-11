/*
 * Copyright (C) 2018-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/* This test tests three things:

-First reserves memory using the knob (remember that memory is reserved for app use) and tries
to access the ranges that were reserved through the app. If it is unsuccessful the test should return 1
(the test checks that at least one of the ranges we reserved are actually allocated for the app)

-Second makes sure that there are no free pages within any of the ranges provided, returns 3 if there are.

-Third tests scenarios where error messages should be returned:

In winrange.address we ask to reserve the following addresses, and expect to see the log messages as stated:

0x57010000:0x57020000
0x57000000:0x57050000
Some part in the middle of the range has been allocated, expect:
winloader: PROCESS_ACCESSOR: Failed to allocate the range: 0x57010000:0x57020000

0x68000000:0x68100000
0x68000000:0x68050000
The entire range has been previously allocated, expect:
winloader: RESERVE MEMORY: Attempt to reserve memory at  0x068000000:0x068050000 - FAILED

0x39080000:0x39090000
0x39060000:0x39090000
The last part of the address was previously allocated, should not be able to allocate it, expect:
winloader: PROCESS_ACCESSOR: Failed to allocate the range: 0x39080000:0x39090000

0x40011000:0x40021000
This address is not aligned to the 64K we expect. Will allocate after fixing alignment, expect to see:
winloader: PROCESS_ACCESSOR: Base Address misaligned, rounding down to: 0x40010000
winloader: PROCESS_ACCESSOR: End Address misaligned, rounding up to: 0x40030000

0x40010000:0x40000000
Here begin>end expect:
ERROR: RESERVE MEMORY: Illegal range  0x040010000:0x040000000
*/
#include <stdio.h>
#include <assert.h>
#include <windows.h>

int main(int argc, char* argv[])
{
    FILE* f         = fopen(argv[1], "r");
    int access_flag = atoi(argv[2]);
    if (!f)
    {
        fprintf(stderr, "cannot open file %s\n", argv[1]);
        return 2;
    }

    uintptr_t low = 0, high = 0, size = 0;
    int tid;
    char desc[64];
    char* ptr = (char*)NULL;

    SYSTEM_INFO si;
    GetSystemInfo(&si);
    int page_size    = si.dwPageSize;
    bool knobSuccess = false;
    bool noFree      = true;
    while (!feof(f))
    {
        fscanf(f, "%Ix %Ix %s %d", &low, &high, desc, &tid);

        if (high < low)
        {
            continue;
        }

        for (uintptr_t i = low; i < high; i += page_size)
        {
            PMEMORY_BASIC_INFORMATION reg = (PMEMORY_BASIC_INFORMATION)malloc(sizeof(MEMORY_BASIC_INFORMATION));
            VirtualQuery((void*)(i), reg, sizeof(MEMORY_BASIC_INFORMATION));
            ptr = (char*)(i);
            if (access_flag == 1 && reg->Protect == PAGE_EXECUTE_READWRITE)
            { //Protection should be PAGE_EXECUTE_READWRITE if the region was allocated through reserve_mem.
                *ptr        = 0;
                knobSuccess = true;
            }
            if (access_flag = 1 && reg->State == MEM_FREE)
            { //there is a free page within the range
                noFree = false;
            }
        }
    }

    fclose(f);

    if (knobSuccess == false && access_flag == 1)
    {
        return 1;
    }
    if (noFree == false && access_flag == 1)
    {
        return 3;
    }

    return 0;
}