/*
 * Copyright (C) 2014-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <cstdio>
#include <cstdlib>
#include <vector>
#include <assert.h>
#include "pin.H"
using std::string;

// This PIN tool shall check PIN's memory limit knob
//

#define ALLOC_CHUNK 0x10000

/* ===================================================================== */
/* Commandline Switches */
/* ===================================================================== */

//Output file where to write everything
KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "memory_limit.out", "specify output file name");

//have memory addresses within that region
KNOB< ADDRINT > KnobBytesToAllocate(KNOB_MODE_WRITEONCE, "pintool", "b", "0x10000", "Number of bytes to allocate");

/* ===================================================================== */
/* Globals */
/* ===================================================================== */

//Output file
FILE* out;

VOID OutOfMemory(size_t sz, VOID* arg)
{
    fprintf(out, "Failed to allocate dynamic memory: Out of memory!\n");
    fclose(out);
    exit(3);
}

VOID* AllocateAndCheck(long size)
{
    void* p = malloc((size_t)size);
    if (NULL == p)
    {
        fprintf(out, "Failed to allocate dynamic memory with size %lx.\n", size);
        fclose(out);
        exit(1);
    }

    return p;
}

// This function allocates number of bytes specified by the -b knobs.
// The bytes are allocated in chunks of size ALLOC_CHUNK.
VOID AppStart(VOID* v)
{
    fprintf(out, "Allocating total %lx bytes.\n", (long)KnobBytesToAllocate.Value());
    long count     = (long)KnobBytesToAllocate.Value() / ALLOC_CHUNK;
    long remainder = (long)KnobBytesToAllocate.Value() % ALLOC_CHUNK;
    for (long i = 0; i < count; i++)
    {
        void* p = AllocateAndCheck(ALLOC_CHUNK);
        fprintf(out, "Iteration %lx, returned: %p\n", i, p);
    }
    if (remainder > 0)
    {
        void* p = AllocateAndCheck(remainder);
        fprintf(out, "Last iteration, returned: %p\n", p);
    }

    fprintf(out, "Test unexpectedly passed (it should fail).\n");
    fclose(out);
}

int main(int argc, char* argv[])
{
    PIN_InitSymbols();
    PIN_Init(argc, argv);

    out = fopen(KnobOutputFile.Value().c_str(), "w");

    PIN_AddOutOfMemoryFunction(OutOfMemory, NULL);

    PIN_AddApplicationStartFunction(AppStart, 0);

    // Never returns
    PIN_StartProgram();

    return 0;
}
