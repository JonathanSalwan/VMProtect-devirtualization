/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <stdlib.h>
#include "pin.H"

using std::string;
/* ===================================================================== */
/* Commandline Switches */
/* ===================================================================== */

KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "memalloc2.out", "specify memalloc2 file name");

/* ===================================================================== */
/* Globals */
/* ===================================================================== */

void* globalReserve             = NULL;
static const size_t ReserveSize = 256 * 1024 * 1024;
volatile BOOL isOutOfMemCalled  = FALSE;
FILE* out;

/* ===================================================================== */
// Specific Linux code -
// On fc5, we found that when running out of memory, the kernal memory
// was also exausted and the process was killed by SIGKILL.
// Therefore, use getrlimit64/setrlimit64 in order to make sure kernel
// memory is not exausted. This limitation is also valuable for 64-bit
// Linux systems in order to prevent memory trashing
// On macOS* the kernel is killing the process on out of memory and we
// have no way to enable the test
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
    // Not supported on FreeBSD
    return 0;
}
#endif

#include <sys/resource.h>

#define TOP_LIMIT (1024 * 1024 * 1024)

void LimitAvailableSpace()
{
    UINT64 myLimit;
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
    char* ptr;
    do
    {
        ptr = (char*)malloc(1024);
    }
    while ((ptr != NULL) && (isOutOfMemCalled == FALSE));
    if (isOutOfMemCalled == FALSE)
    {
        fprintf(out, "Got NULL while trying to allocate memory, test failure.\n");
    }
    else
    {
        fprintf(out, "OutOfMem was called while trying to allocate memory, test successful.\n");
    }
    fclose(out);
}

// This callback function should never be called.
VOID Assert(size_t size, VOID* v)
{
    fprintf(out, "test failure.\n");
    exit(-1);
}

// This callback function is not serialized by Pin.
// pin tool writer should serialize it if it is needed (for MT applications).
VOID OutOfMem(size_t size, VOID* v)
{
    //First thing - free memory
    if (globalReserve != NULL)
    {
        free(globalReserve);
        globalReserve = NULL;

        isOutOfMemCalled = TRUE;
        //Calling malloc() in here can cause an infinite recursion, try to avoid it.
        //if(size < (ReserveSize / 2))
        //{
        //    globalReserve = malloc(ReserveSize / 2);
        //}
    }
}

int main(int argc, char* argv[])
{
    PIN_Init(argc, argv);
    PIN_InitSymbols();

    globalReserve = malloc(ReserveSize);
    PIN_AddOutOfMemoryFunction(Assert, 0);
    PIN_AddOutOfMemoryFunction(OutOfMem, 0);

#if defined(TARGET_LINUX) || defined(TARGET_MAC)
    LimitAvailableSpace();
#endif

    out = fopen(KnobOutputFile.Value().c_str(), "w");

    PIN_AddFiniFunction(Fini, 0);
    PIN_StartProgram();

    return 0;
}
