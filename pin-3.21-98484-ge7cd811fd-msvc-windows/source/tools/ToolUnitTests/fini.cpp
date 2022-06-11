/*
 * Copyright (C) 2004-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include "pin.H"

void Fini(INT32 code, VOID* v)
{
    FILE* f = fopen("fini.out", "w");
    fprintf(f, "Fini\n");
    fclose(f);
}

int main(INT32 argc, CHAR** argv)
{
    PIN_Init(argc, argv);

    PIN_AddFiniFunction(Fini, 0);

    // Never returns
    PIN_StartProgram();
    return 0;
}
