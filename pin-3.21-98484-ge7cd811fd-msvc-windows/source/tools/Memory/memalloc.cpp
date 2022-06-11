/*
 * Copyright (C) 2004-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <stdlib.h>
#include "pin.H"
using std::string;
/* ===================================================================== */
/* Commandline Switches */
/* ===================================================================== */

KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "memalloc.out", "specify memalloc file name");

/* ===================================================================== */
/* Globals */
/* ===================================================================== */

FILE* out;

/* ===================================================================== */
// Specific Linux code -
// On fc5, we found that when running out of memory, the kernel memory
// was also exhausted and the process was killed by SIGKILL.
// Therefore, use getrlimit64/setrlimit64 in order to make sure kernel
// memory is not exhausted. This limitation is also valuable for 64-bit
// Linux systems in order to prevent memory trashing
/* ===================================================================== */

#define GETRLIMIT getrlimit
#define SETRLIMIT setrlimit
typedef struct rlimit rlimit_t;

#if defined(TARGET_LINUX) || defined(TARGET_MAC)

#if defined(TARGET_LINUX)

UINT64 GetTotalSwap()
{
    long long total = 0;
    int res         = 0;
    FILE* f         = fopen("/proc/meminfo", "r");
    if (NULL == f)
    {
        return 0;
    }
    do
    {
        char buf[128];
        if (NULL == fgets(buf, sizeof(buf), f))
        {
            break;
        }
        res = sscanf(buf, "SwapTotal: %lld kB\n", &total);
    }
    while (res != 1);

    fclose(f);
    return (UINT64)(1024LL * total);
}

#else
UINT64 GetTotalSwap()
{
    // Not supported on macOS*
    return 0;
}
#endif

#include <sys/resource.h>

#define TOP_LIMIT (1024 * 1024 * 1024)

void LimitAvailableSpace()
{
    size_t myLimit;
    rlimit_t rlim;

    // Get total swap in bytes (originally it is in mem units).
    UINT64 totalswap = GetTotalSwap();

    // Make the limit at most 40% of total swap area or 400Mb
    if (totalswap == 0)
    {
        myLimit = (TOP_LIMIT / 10) * 4;
    }
    else
    {
        if (totalswap > (UINT64)(TOP_LIMIT))
        {
            myLimit = (TOP_LIMIT / 10) * 4;
        }
        else
        {
            myLimit = (totalswap / 10) * 4;
        }
    }

    if (GETRLIMIT(RLIMIT_AS, &rlim) < 0)
    {
        fprintf(out, "failed to getrlimit: continue...\n");
        return;
    }

    if ((size_t)rlim.rlim_cur > myLimit)
    {
        rlim.rlim_cur = myLimit;
    }

    if (SETRLIMIT(RLIMIT_AS, &rlim) < 0)
    {
        fprintf(out, "failed to setrlimit: continue...\n");
        return;
    }
}
#endif

VOID Fini(INT32 code, VOID* v)
{
    void* reserve = malloc(1024 * 1024); //Reserving enough memory to allow Pin to allocate memory while
                                         //shutting down.
    char* ptr;
    do
    {
        ptr = (char*)malloc(1024);
    }
    while (ptr != NULL);
    free(reserve);
    fprintf(out, "Got NULL while trying to allocate memory, test successful.\n");

    static const size_t alignmentForCompilerOptimization = 16;

    for (size_t sz = alignmentForCompilerOptimization; sz < 0x100000; sz *= 2)
    {
        void* ptr    = malloc(sz);
        ADDRINT addr = (ADDRINT)ptr;
        ASSERTX(0 == addr % alignmentForCompilerOptimization);
        free(ptr);
    }

    fclose(out);
}

int main(int argc, char* argv[])
{
    PIN_Init(argc, argv);
    PIN_InitSymbols();

#if defined(TARGET_LINUX)
    LimitAvailableSpace();
#endif

    out = fopen(KnobOutputFile.Value().c_str(), "w");

    PIN_AddFiniFunction(Fini, 0);
    // Never returns
    PIN_StartProgram();

    return 0;
}
