/*
 * Copyright (C) 2010-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>

/*
 * Only run this on IA32, Linux (could also run on other gcc compatible platforms).
 * The code in Pin is all generic, but generating the test is simpler if we constrain things.
 */
int deleteMov() __attribute__((noinline));
int deleteMov()
{
    int res;
    // The mov will be deleted by the tool
    __asm__ volatile("xor   %0,%0;"
                     "mov   $-1,%0"
                     : "=r"(res));
    return res;
}

int insertJump() __attribute__((noinline));
int insertJump()
{
    int res;
    // The mov will be branched over by the tool
    __asm__ volatile("xor   %0,%0;"
                     "mov   $-1,%0"
                     : "=r"(res));
    return res;
}

int insertIndirectJump() __attribute__((noinline));
int insertIndirectJump()
{
    int res;
    // The mov will be branched over by the tool
    __asm__ volatile("xor   %0,%0;"
                     "mov   $-1,%0"
                     : "=r"(res));
    return res;
}

static int values[] = {0, -1};

int modifyAddressing(int) __attribute__((noinline));
int modifyAddressing(int idx)
{
    int* base = &values[0];
    int res   = 0;
    // The addressing on this or will be modified...
    __asm__ volatile("or   (%1,%2,4),%0" : "+r"(res) : "r"(base), "r"(idx));

    return res;
}

int main(int argc, char** argv)
{
    int failed = 0;
    if (deleteMov() != 0)
    {
        fprintf(stderr, "Mov instruction was not deleted\n");
        failed++;
    }
    if (insertJump() != 0)
    {
        fprintf(stderr, "Mov instruction was not branched over\n");
        failed++;
    }
    if (insertIndirectJump() != 0)
    {
        fprintf(stderr, "Mov instruction was not indirectly branched over\n");
        failed++;
    }
    if (modifyAddressing(1) != 0)
    {
        fprintf(stderr, "Addressing was not modified\n");
        failed++;
    }

    return failed;
}
