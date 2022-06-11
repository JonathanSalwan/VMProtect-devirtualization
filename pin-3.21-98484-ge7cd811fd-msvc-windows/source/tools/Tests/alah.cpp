/*
 * Copyright (C) 2005-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <stdlib.h>
#include "pin.H"
#include <iostream>
unsigned int n   = 0xdeadbeef;
unsigned int res = 0;

VOID ahal()
{
#if defined(PIN_GNU_COMPATIBLE)
    asm("mov %0,%%eax; mov %%ah,%%al; mov %%eax,%1" : "=m"(n) : "m"(res) : "eax");
#elif defined(PIN_MS_COMPATIBLE)
    __asm {
       mov eax, [n]
       mov al, ah
       mov [res], eax
    }
#endif
}

VOID Check(INT32 code, VOID* v)
{
    if (res != 0xdeadbebe)
    {
        printf("n %x res %x\n", n, res);
        exit(1);
    }
}

VOID Instruction(INS ins, VOID* v)
{
    static bool first = true;
    if (first)
    {
        INS_InsertCall(ins, IPOINT_AFTER, (AFUNPTR)ahal, IARG_END);
        INS_InsertCall(ins, IPOINT_AFTER, (AFUNPTR)Check, IARG_END);
    }
    first = false;
}

int main(int argc, char* argv[])
{
    PIN_Init(argc, argv);

    INS_AddInstrumentFunction(Instruction, 0);

    // Never returns
    PIN_StartProgram();

    return 0;
}
