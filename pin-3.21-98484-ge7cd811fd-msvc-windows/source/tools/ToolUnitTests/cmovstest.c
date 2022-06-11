/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

//
// Code which includes examples of all the different cmov conditions.
//
#include <stdio.h>

static int executed = 0;
static int total    = 0;

#define S1(s) #s
#define STRINGIZE(s) S1(s)

// In the tool we look for fnop and toggle counting, so that it only
// counts the cmov operations in this code, not elsewhere in the runtime.
static void toggleCollection() { __asm__ __volatile__("fnop"); }

// Iterate over each CMOVcc instruction (there are more mnemonics than this,
// but they're all synonyms for one of these)
#define FOREACH_CMOV(expand)                                                                                                \
    expand(cmovb) expand(cmovbe) expand(cmovl) expand(cmovle) expand(cmovnb) expand(cmovnbe) expand(cmovnl) expand(cmovnle) \
        expand(cmovno) expand(cmovnp) expand(cmovns) expand(cmovnz) expand(cmovo) expand(cmovp) expand(cmovs) expand(cmovz)

#define DeclareCounter(movOp)        \
    static int movOp##_total    = 0; \
    static int movOp##_executed = 0;

FOREACH_CMOV(DeclareCounter)

#define CMOVFunction(res, comparison, movOp)                                         \
    {                                                                                \
        int one = 1;                                                                 \
        res     = 0;                                                                 \
        if (comparison)                                                              \
            __asm__ __volatile__(STRINGIZE(movOp) " %1, %0" : "+r"(res) : "r"(one)); \
        else                                                                         \
            __asm__ __volatile__(STRINGIZE(movOp) " %1, %0" : "+r"(res) : "r"(one)); \
                                                                                     \
        movOp##_total++;                                                             \
        movOp##_executed += res;                                                     \
    }

#define DefineTest(comparison, movOp)                           \
    toggleCollection();                                         \
    CMOVFunction(result, comparison, movOp) toggleCollection(); \
    total++;                                                    \
    if (result)                                                 \
    {                                                           \
        executed++;                                             \
    }

#define DefineEqual(movOp) DefineTest(a == b, movOp)

#define PrintStats(movOp) printf("%-7s: %4d %4d\n", STRINGIZE(movOp), movOp##_total, movOp##_executed);

int main(int argc, char** argv)
{
    static int aValues[] = {-2, -1, 0, 1, 2};
    static int bValues[] = {-2, -1, 0, 1, 2};
    int aIndex;
    int bIndex;

    for (aIndex = 0; aIndex < sizeof(aValues) / sizeof(aValues[0]); aIndex++)
    {
        int a = aValues[aIndex];

        for (bIndex = 0; bIndex < sizeof(bValues) / sizeof(bValues[0]); bIndex++)
        {
            int b = bValues[bIndex];
            int result;

            FOREACH_CMOV(DefineEqual);
        }
    }

    printf("Total : %d Executed : %d\n", total, executed);

    FOREACH_CMOV(PrintStats);

    return 0;
}
