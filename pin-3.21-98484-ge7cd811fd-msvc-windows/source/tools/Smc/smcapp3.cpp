/*
 * Copyright (C) 2015-2021 Intel Corporation.
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

    // buffer to move foo routines into and execute
    static char staticBuffer1[PI_FUNC::MAX_SIZE];
    static char staticBuffer2[PI_FUNC::MAX_SIZE];
    static char staticBuffer3[PI_FUNC::MAX_SIZE];
    // Set read-write-execute protection for the buffer
    size_t pageSize  = GetPageSize();
    char* firstPage1 = (char*)(((size_t)staticBuffer1) & ~(pageSize - 1));
    char* endPage1   = (char*)(((size_t)staticBuffer1 + sizeof(staticBuffer1) + pageSize - 1) & ~(pageSize - 1));
    char* firstPage2 = (char*)(((size_t)staticBuffer2) & ~(pageSize - 1));
    char* endPage2   = (char*)(((size_t)staticBuffer2 + sizeof(staticBuffer2) + pageSize - 1) & ~(pageSize - 1));
    char* firstPage3 = (char*)(((size_t)staticBuffer3) & ~(pageSize - 1));
    char* endPage3   = (char*)(((size_t)staticBuffer3 + sizeof(staticBuffer3) + pageSize - 1) & ~(pageSize - 1));
    if (!MemProtect(firstPage1, endPage1 - firstPage1, MEM_READ_WRITE_EXEC))
    {
        Abort("MemProtect failed");
    }
    if (!MemProtect(firstPage2, endPage2 - firstPage2, MEM_READ_WRITE_EXEC))
    {
        Abort("MemProtect failed");
    }
    if (!MemProtect(firstPage3, endPage3 - firstPage3, MEM_READ_WRITE_EXEC))
    {
        Abort("MemProtect failed");
    }
    int printed = 0;
    FOO_FUNC fooFunc;
    FILE* fp = fopen("smcapp3_a.out", "w");
    fprintf(fp, "%p\n", staticBuffer1);
    fclose(fp);
    fooFunc.Copy(staticBuffer1).Execute().AssertStatus();

    fp = fopen("smcapp3_b.out", "w");
    fprintf(fp, "%p\n", staticBuffer2);
    fclose(fp);
    fooFunc.Copy(staticBuffer2).Execute().AssertStatus();

    fooFunc.Copy(staticBuffer3).Execute().AssertStatus();

    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
