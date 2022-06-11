/*
 * Copyright (C) 2014-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <cstdio>
#include <cstdlib>
#include <string.h>
#ifndef TARGET_MAC
#include <malloc.h>
#endif
#if defined(TARGET_LINUX) || defined(PIN_CRT)
#include <unistd.h>
#endif
#include "pin.H"
using std::string;

#if defined(PIN_CRT)
#define HAVE_POSIX_MEMALIGN
#define HAVE_MEMALIGN
#define HAVE_VALLOC
#elif defined(TARGET_WINDOWS)
#define HAVE_ALIGNED_MALLOC
#elif defined(TARGET_MAC)
#define HAVE_POSIX_MEMALIGN
#define HAVE_VALLOC
#elif defined(TARGET_LINUX)
#define HAVE_POSIX_MEMALIGN
#define HAVE_MEMALIGN
#define HAVE_VALLOC
#endif

/* ===================================================================== */
/* Commandline Switches */
/* ===================================================================== */

KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "memalign.out", "specify output file name");

/* ===================================================================== */
/* Globals */
/* ===================================================================== */

FILE* out;

#define NUMBER_OF_ALIGNMENTS_TO_CHECK 12
#define SIZE_TO_ALLOCATE 117

VOID AlignCheck(const char* msg, VOID* p, int alignment, int size)
{
    if (p == NULL)
    {
        fprintf(out, "Failed to allocate aligned memory from %s. Alignment=%d, size=%d.\n", msg, alignment, size);
        abort();
    }
    ADDRINT mask = alignment - 1;
    ADDRINT addr = (ADDRINT)p;
    if ((addr & mask) != 0)
    {
        fprintf(out, "Memory allocated from %s is not alligned. Returned memory %p, Alignment=%d, size=%d.\n", msg, p, alignment,
                size);
    }
    memset(p, 0x12345678, size);
}

VOID Fini(INT32 code, VOID* v)
{
    int alignment = sizeof(VOID*);
    for (int i = 1; i < NUMBER_OF_ALIGNMENTS_TO_CHECK; i++)
    {
        void* p = NULL;
#ifdef HAVE_ALIGNED_MALLOC
        fprintf(out, "Calling _aligned_malloc\n");
        p = _aligned_malloc(SIZE_TO_ALLOCATE, alignment);
        AlignCheck("_aligned_malloc", p, alignment, SIZE_TO_ALLOCATE);
        fprintf(out, "Free _aligned_malloc. p=%p\n", p);
        fflush(out);
        _aligned_free(p);
#endif // HAVE_ALIGNED_MALLOC
#ifdef HAVE_POSIX_MEMALIGN
        fprintf(out, "Calling posix_memalign. Alignment=%d\n", alignment);
        if (posix_memalign(&p, alignment, SIZE_TO_ALLOCATE)) p = NULL;
        AlignCheck("posix_memalign", p, alignment, SIZE_TO_ALLOCATE);
        fprintf(out, "Free posix_memalign. p=%p\n", p);
        fflush(out);
        free(p);
#endif // HAVE_POSIX_MEMALIGN
#ifdef HAVE_MEMALIGN
        fprintf(out, "Calling memalign. Alignment=%d\n", alignment);
        p = memalign(alignment, SIZE_TO_ALLOCATE);
        AlignCheck("memalign", p, alignment, SIZE_TO_ALLOCATE);
        fprintf(out, "Free memalign. p=%p\n", p);
        fflush(out);
        free(p);
#endif // HAVE_MEMALIGN

        alignment *= 2;
    }
#ifdef HAVE_VALLOC
    fprintf(out, "Calling valloc\n");
    void* p = valloc(SIZE_TO_ALLOCATE);
    AlignCheck("valloc", p,
#if defined(TARGET_MAC) || defined(PIN_CRT)
               getpagesize(),
#else
               sysconf(_SC_PAGESIZE),
#endif
               SIZE_TO_ALLOCATE);
    fprintf(out, "Free valloc. p=%p\n", p);
    fflush(out);
    free(p);
#endif // HAVE_VALLOC
    fclose(out);
}

int main(int argc, char* argv[])
{
    PIN_Init(argc, argv);

    out = fopen(KnobOutputFile.Value().c_str(), "w");

    PIN_AddFiniFunction(Fini, 0);
    // Never returns
    PIN_StartProgram();

    return 0;
}
