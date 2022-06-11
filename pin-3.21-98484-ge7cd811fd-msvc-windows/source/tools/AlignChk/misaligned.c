/*
 * Copyright (C) 2007-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdlib.h>

#if !defined(__GNUC__)
extern void movdqa_test(char* p);
#endif

int main(int argc, char** argv)
{
    char a[1000];

    /* get in to the buffer and then align it by 16 */
    static const size_t align16_mask = (16 - 1);
    /* Integer type size_t defined to have size of pointer */
    char* b = (char*)(((size_t)a + align16_mask) & ~align16_mask);

    /* generate one aligned move and one unaligned move. The alignchk tool
     * should catch the latter one. */

#if defined(__GNUC__)
    char* c = b + 1;
    asm volatile("movdqa %%xmm0, %0" : "=m"(*b) : : "%xmm0");
    asm volatile("movdqa %%xmm0, %0" : "=m"(*c) : : "%xmm0");
#else
    movdqa_test(b);
#endif
}
