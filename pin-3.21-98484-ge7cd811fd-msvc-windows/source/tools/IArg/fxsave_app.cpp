/*
 * Copyright (C) 2015-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <cstring>
#include <cstdio>

#ifdef TARGET_WINDOWS
#define ASMNAME(name)
#define ALIGN64 __declspec(align(64))
#else
#define ASMNAME(name) asm(name)
#define ALIGN64 __attribute__((aligned(64)))
#endif

/////////////////////
// EXTERNAL FUNCTIONS
/////////////////////

extern "C" void Do_Fxsave() ASMNAME("Do_Fxsave");
extern "C" void Do_Fxrstor() ASMNAME("Do_Fxrstor");

/////////////////////
// GLOBAL VARIABLES
/////////////////////

extern "C"
{
    unsigned char ALIGN64 xsaveArea[1000] ASMNAME("xsaveArea");
}

int main(int argc, const char* argv[])
{
    memset(xsaveArea, 0, sizeof(xsaveArea));

    Do_Fxsave();

    Do_Fxrstor();

    return 0;
}
