/*
 * Copyright (C) 2005-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// This test looks for fails in updating the hash table
// Even with an intentional bug, it rarely fails
// Increase TRIES to make it run longer and more likely to find the bug

#define TRIES 1000

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include "../Utils/threadlib.h"
#if defined(__GNUC__)
#include <stdlib.h>
#endif
int a[100000];
int n = 10;

typedef int (*FUNPTR)();

#define FUN(n) \
    int fun##n() { return n; }
#define FUN10(n) \
    FUN(n##0)    \
    FUN(n##1)    \
    FUN(n##2)    \
    FUN(n##3)    \
    FUN(n##4)    \
    FUN(n##5)    \
    FUN(n##6)    \
    FUN(n##7)    \
    FUN(n##8)    \
    FUN(n##9)

#define FUN100(n) \
    FUN10(n##0)   \
    FUN10(n##1)   \
    FUN10(n##2)   \
    FUN10(n##3)   \
    FUN10(n##4)   \
    FUN10(n##5)   \
    FUN10(n##6)   \
    FUN10(n##7)   \
    FUN10(n##8)   \
    FUN10(n##9)

FUN100(1)
FUN100(2)
FUN100(3)
FUN100(4)
FUN100(5)
FUN100(6)
FUN100(7)
FUN100(8)
FUN100(9)
FUN100(10)
FUN100(11)
FUN100(12)
FUN100(13)

#define FUNINIT10(n) fun##n##0, fun##n##1, fun##n##2, fun##n##3, fun##n##4, fun##n##5, fun##n##6, fun##n##7, fun##n##8, fun##n##9
#define FUNINIT100(n)                                                                                                      \
    FUNINIT10(n##0), FUNINIT10(n##1), FUNINIT10(n##2), FUNINIT10(n##3), FUNINIT10(n##4), FUNINIT10(n##5), FUNINIT10(n##6), \
        FUNINIT10(n##7), FUNINIT10(n##8), FUNINIT10(n##9)

FUNPTR funs[] = {FUNINIT100(1), FUNINIT100(2), FUNINIT100(3),  FUNINIT100(4),  FUNINIT100(5),  FUNINIT100(6), FUNINIT100(7),
                 FUNINIT100(8), FUNINIT100(9), FUNINIT100(10), FUNINIT100(11), FUNINIT100(12), FUNINIT100(13)};

void* hello(void* arg)
{
    int i;
    int j;

    for (j = 0; j < TRIES; j++)
    {
        for (i = 0; i < 1300; i++)
        {
            int res    = funs[i]();
            int expect = i + 100;

            if (res != expect)
            {
                printf("Expected %d, got %d\n", expect, res);
                exit(1);
            }
        }
    }

    return 0;
}

THREAD_HANDLE threads[MAXTHREADS];

int main(int argc, char* argv[])
{
    int numthreads = 0;
    int i;

    numthreads = 2;
    assert(numthreads < MAXTHREADS);

    for (i = 0; i < numthreads; i++)
    {
        printf("Creating thread\n");
        fflush(stdout);
        CreateOneThread(&threads[i], hello, 0);
        fflush(stdout);
    }

    for (i = 0; i < numthreads; i++)
    {
        BOOL success;
        success = JoinOneThread(threads[i]);
        if (!success)
        {
            fprintf(stdout, "JoinOneThread failed\n");
        }
        else
        {
            fprintf(stdout, "Joined %d\n", i);
        }
        fflush(stdout);
    }

    printf("All threads joined\n");
    fflush(stdout);

    return 0;
}
