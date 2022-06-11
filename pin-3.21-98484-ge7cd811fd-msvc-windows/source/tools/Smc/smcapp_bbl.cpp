/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 *  An example of SMC inside a basic block. 
 *  To run this application under pin, specify -smc_strict option. 
 */
#include "smc_util.h"
#include "../Utils/sys_memory.h"

#if !defined(TARGET_IA32) || !defined(TARGET_WINDOWS)
#error This must be a 32-bit Windows program.
#endif

#define CODE_SECTION(name) __pragma(code_seg(name))
#define SECTION_END __pragma(code_seg())

/*!
 * A function whose single basic block modifies its own code. 
 * The function copies "bar" string into the specified buffer. If in-BBL SMC
 * is not handled, the output string is "foo".
 * To simplify calculation of the size of this function, it is placed in a special 
 * code section along with the immediately following foo2barEnd() function.
 */
CODE_SECTION("foo2bar_code")
static void foo2bar(char* result)
{
    __asm {
        // Modify immediate operands in smc* instructions
        mov byte ptr [offset smc1 + 2], 'b'; 
        mov byte ptr [offset smc2 + 2], 'a';
        mov byte ptr [offset smc3 + 2], 'r';

        mov eax, dword ptr [result];

        smc1: mov byte ptr [eax], 'f'; // *result++ = 'f'
        inc eax;

        smc2: mov byte ptr [eax], 'o'; // *result++ = 'o'
        inc eax;

        smc3: mov byte ptr [eax], 'o'; // *result++ = 'o'
        inc eax;

        mov byte ptr [eax], 0; // *result = 0
    }
}
SECTION_END

CODE_SECTION("foo2bar_code")
static void foo2barEnd(char* str) {}
SECTION_END

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
    // Set read-write-execute protection for the code of the foo2bar() function
    size_t pageSize   = GetPageSize();
    const char* start = CastPtr< char >(foo2bar);
    const char* end   = CastPtr< char >(foo2barEnd);
    char* firstPage   = (char*)(((size_t)start) & ~(pageSize - 1));
    char* endPage     = (char*)(((size_t)end + pageSize - 1) & ~(pageSize - 1));
    if (!MemProtect(firstPage, endPage - firstPage, MEM_READ_WRITE_EXEC))
    {
        Abort("MemProtect failed");
    }

    // Execute the function and verify result
    char result[16];
    foo2bar(result);
    if (strcmp(result, "bar") != 0)
    {
        Abort("foo2bar: Unexpected result");
    }
    cerr << "foo2bar: Success" << endl;
    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
