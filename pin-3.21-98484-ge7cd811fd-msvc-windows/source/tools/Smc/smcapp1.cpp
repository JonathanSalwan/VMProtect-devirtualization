/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 *  An example of SMC application. 
 */
#include "smc_util.h"
#include "../Utils/sys_memory.h"
#include <stdio.h>

/*!
 * Exit with the specified error message
 */
static void Abort(string msg)
{
    fprintf(stderr, "%s\n", msg.c_str());
    exit(1);
}

/*!
 * The main procedure of the application.
 */
int main(int argc, char* argv[])
{
    fprintf(stderr, "SMC in the image of the application\n");

#if !defined(TARGET_MAC)
    // buffer to move foo/bar routines into and execute
    static char staticBuffer[PI_FUNC::MAX_SIZE];
#else
    // Starting from macOS 10.15, statically allocated memory doesn't have execute permission and cannot be modified to have
    // execute permission. Therefore dynamically allocating this memory (which allows adding execute permission).
    // Still using staticBuffer just for convenience.
    char* staticBuffer = (char*)malloc(PI_FUNC::MAX_SIZE);
#endif

    // Set read-write-execute protection for the buffer
    size_t pageSize = GetPageSize();
    char* firstPage = (char*)(((size_t)staticBuffer) & ~(pageSize - 1));
    char* endPage   = (char*)(((size_t)staticBuffer + sizeof(staticBuffer) + pageSize - 1) & ~(pageSize - 1));
    if (!MemProtect(firstPage, endPage - firstPage, MEM_READ_WRITE_EXEC))
    {
        Abort("MemProtect failed");
    }
    int printed = 0;
    for (int i = 0; i < 3; ++i)
    {
        FOO_FUNC fooFunc;
        if (!printed)
        {
            FILE* fp = fopen("smcapp1.out", "w");
            fprintf(fp, "%p\n", staticBuffer);
            printf("fooFunc.Start() %p fooFunc.End() %p\n", staticBuffer,
                   (void*)((unsigned char*)(staticBuffer) + fooFunc.Size()));
            fclose(fp);
            printed = 1;
        }
        fooFunc.Copy(staticBuffer).Execute().AssertStatus();

        BAR_FUNC barFunc;
        barFunc.Copy(staticBuffer).Execute().AssertStatus();
    }
    fprintf(stderr, "Dynamic code generation\n");
    void* dynamicBuffer;
    dynamicBuffer = MemAlloc(PI_FUNC::MAX_SIZE, MEM_READ_WRITE_EXEC);
    if (dynamicBuffer == 0)
    {
        Abort("MemAlloc failed");
    }

    {
        FOO_FUNC fooFunc;
        fooFunc.Copy(dynamicBuffer);
        if (!MemProtect(dynamicBuffer, PI_FUNC::MAX_SIZE, MEM_READ_EXEC))
        {
            Abort("MemProtect failed");
        }
        for (int i = 0; i < 6; ++i)
        {
            fooFunc.Execute().AssertStatus();
        }
    }

    if (!MemProtect(dynamicBuffer, PI_FUNC::MAX_SIZE, MEM_READ_WRITE_EXEC))
    {
        Abort("MemProtect failed");
    }

    {
        BAR_FUNC barFunc;
        barFunc.Copy(dynamicBuffer);
        if (!MemProtect(dynamicBuffer, PI_FUNC::MAX_SIZE, MEM_READ_EXEC))
        {
            Abort("MemProtect failed");
        }
        for (int i = 0; i < 6; ++i)
        {
            barFunc.Execute().AssertStatus();
        }
    }

    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
