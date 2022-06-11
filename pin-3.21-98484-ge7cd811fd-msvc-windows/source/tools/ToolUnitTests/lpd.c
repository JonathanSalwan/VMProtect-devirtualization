/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdint.h>
#include <stdio.h>

typedef struct
{
    uint64_t data[2] __attribute__((aligned(16)));
} XMM_VALUE;

XMM_VALUE in;
XMM_VALUE out;

int main()
{
    in.data[0] = 0x1234567887654321;
    in.data[1] = 0x1234567887654321;

    asm("movapd %0,%%xmm0" ::"m"(in));
    asm("movlpd %0,%%xmm0" ::"m"(in));
    asm("movapd %%xmm0,%0" : "=m"(out));
    if (memcmp(&in, &out, sizeof(in)) != 0)
    {
        printf("Error:\n");
        printf("in:%llx %llx\n", in.data[0], in.data[1]);
        printf("out: %llx %llx\n", out.data[0], out.data[1]);
        return 1;
    }
    else
    {
        printf("Passed\n");
    }
    return 0;
}
