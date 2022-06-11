/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 *  An example of SMC application. 
 */
#include "smc_util.h"
#include "../Utils/sys_memory.h"

#if defined(TARGET_MAC)
//# include <mach/mach.h>
#define CODE_SECTION_MAC(name) __attribute__((section("__TEXT, " name)))
#else
// No need at the moment for other OS's.
#define CODE_SECTION_MAC(name)
#endif

/*!
 * Exit with the specified error message
 */
static void Abort(string msg)
{
    cerr << msg << endl;
    exit(1);
}

/*!
 * The main procedure of the application.
 */
int main(int argc, char* argv[])
{
    cerr << "SMC in the image of the application" << endl;

    // buffer to move foo/bar routines into and execute
    // Starting from macOS 10.15, statically allocated memory doesn't have execute permission and cannot be modified to have
    // execute permission. Therefore adding compiler annotation to mark this buffer as a text section which will grant him
    // execute permission.
    static char staticBuffer[PI_FUNC::MAX_SIZE] CODE_SECTION_MAC("executable_buf");
    // Set read-write-execute protection for the buffer
    size_t pageSize = GetPageSize();
    char* firstPage = (char*)(((size_t)staticBuffer) & ~(pageSize - 1));
    char* endPage   = (char*)(((size_t)staticBuffer + sizeof(staticBuffer) + pageSize - 1) & ~(pageSize - 1));
    if (!MemProtect(firstPage, endPage - firstPage, MEM_READ_WRITE_EXEC))
    {
        Abort("MemProtect failed");
    }

    for (int i = 0; i < 3; ++i)
    {
        FOO_FUNC fooFunc;
        fooFunc.Copy(staticBuffer).Execute().AssertStatus();
        cerr << fooFunc.Name() << ": " << fooFunc.ErrorMessage() << endl;

        BAR_FUNC barFunc;
        barFunc.Copy(staticBuffer).Execute().AssertStatus();
        cerr << barFunc.Name() << ": " << barFunc.ErrorMessage() << endl;
    }

    cerr << "Dynamic code generation" << endl;
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
        for (int i = 0; i < 3; ++i)
        {
            fooFunc.Execute().AssertStatus();
            cerr << fooFunc.Name() << ": " << fooFunc.ErrorMessage() << endl;
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
        for (int i = 0; i < 3; ++i)
        {
            barFunc.Execute().AssertStatus();
            cerr << barFunc.Name() << ": " << barFunc.ErrorMessage() << endl;
        }
    }

    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
