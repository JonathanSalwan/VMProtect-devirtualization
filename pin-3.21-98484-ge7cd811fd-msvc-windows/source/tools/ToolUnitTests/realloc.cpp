/*
 * Copyright (C) 2007-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * This test verifies that the functionality of realloc() function.  It
 * also verifies that the memory allocated by realloc() can be freed by
 * Pin's free() function.
 */

#include <iostream>
#include <vector>
#include <stdlib.h>
#include "pin.H"
using std::dec;
using std::hex;

extern "C" void* memalign(size_t, size_t);
extern "C" void* realloc(void*, size_t);

static BOOL TestRealloc(void*, size_t);

int main(int argc, char* argv[])
{
    PIN_Init(argc, argv);

    // Test realloc to larger and smaller sizes

    void* ptr1 = malloc(20);
    if (ptr1 == NULL)
    {
        std::cerr << "Malloc failed\n";
        exit(-1);
    }

    std::cerr << "Malloc : " << hex << ptr1 << dec << std::endl;

    if (TestRealloc(ptr1, 20))
    {
        std::cerr << "Realloc of simple malloc failed\n";
        return -1;
    }

    // Test realloc of small chunk aligned at small value
    void* ptr2 = memalign(32, 400);
    if (ptr2 == NULL)
    {
        std::cerr << "Memalign failed\n";
        exit(-1);
    }

    std::cerr << "Memalign : " << hex << ptr2 << dec << std::endl;

    if (TestRealloc(ptr2, 400))
    {
        std::cerr << "Realloc of small chunk aligned at small boundary failed\n";
        return -1;
    }

    // Test realloc of small chunk aligned at large value
    void* ptr3 = memalign(2048, 400);
    if (ptr3 == NULL)
    {
        std::cerr << "Memalign failed\n";
        exit(-1);
    }

    std::cerr << "Memalign : " << hex << ptr3 << dec << std::endl;

    if (TestRealloc(ptr3, 400))
    {
        std::cerr << "Realloc of small chunk aligned at large boundary failed\n";
        return -1;
    }

    // Test realloc of large chunk aligned at large value
    void* ptr4 = memalign(2048, 400000);
    if (ptr4 == NULL)
    {
        std::cerr << "Memalign failed\n";
        exit(-1);
    }

    std::cerr << "Memalign : " << hex << ptr4 << dec << std::endl;

    if (TestRealloc(ptr4, 400000))
    {
        std::cerr << "Realloc of large chunk aligned at large boundary failed\n";
        return -1;
    }

    // Never returns
    PIN_StartProgram();
}

BOOL TestRealloc(void* ptr, size_t sz)
{
    // Reallocate ptr to a larger size
    void* tmp = realloc(ptr, sz + 21000);
    if (tmp == NULL)
    {
        std::cerr << "Realloc to larger size failed\n";
        return 1;
    }
    ptr = tmp;
    std::cerr << "Realloc to larger size : " << hex << ptr << dec << std::endl;

    // Reallocate ptr to a half the original size
    tmp = realloc(ptr, sz / 2);
    if (tmp == NULL)
    {
        std::cerr << "Realloc to smaller size failed\n";
        return 1;
    }
    ptr = tmp;
    std::cerr << "Realloc to smaller size : " << hex << ptr << dec << std::endl;
    free(ptr);

    return 0;
}
