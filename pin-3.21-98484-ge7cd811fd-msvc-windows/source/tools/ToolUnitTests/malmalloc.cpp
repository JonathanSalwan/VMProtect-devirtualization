/*
 * Copyright (C) 2004-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <stdlib.h>
#include "pin.H"

char* data[100];
INT32 sizeindex[100];

INT32 sizes[] = {100, 4000, 30, 20, 6000, 24000, 0};

VOID mal(INT32 id)
{
    char* d    = data[id];
    INT32 size = sizes[sizeindex[id]];

    if (d)
    {
        for (INT32 i = 0; i < size; i++)
        {
            if (d[i] != id)
            {
                fprintf(stderr, "Bad data id %d data %d\n", id, d[i]);
                exit(1);
            }
        }
        free(d);
    }

    sizeindex[id]++;

    if (sizes[sizeindex[id]] == 0) sizeindex[id] = 0;
    size = sizes[sizeindex[id]];

    ASSERTX(size != 0);

    data[id] = (char*)malloc(size);
    d        = data[id];
    for (INT32 i = 0; i < size; i++)
    {
        d[i] = id;
    }
}

VOID Tr(TRACE trace, VOID*) { TRACE_InsertCall(trace, IPOINT_BEFORE, AFUNPTR(mal), IARG_THREAD_ID, IARG_END); }

int main(INT32 argc, CHAR** argv)
{
    PIN_Init(argc, argv);

    TRACE_AddInstrumentFunction(Tr, 0);

    // Never returns
    PIN_StartProgram();

    return 0;
}
