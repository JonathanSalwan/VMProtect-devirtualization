/*
 * Copyright (C) 2007-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// This little application tests calling application functions.
//
#include <stdio.h>
#include <limits.h>

#if defined(TARGET_WINDOWS)
#define EXPORT_SYM __declspec(dllexport)
#else
#define EXPORT_SYM extern
#endif

EXPORT_SYM void* Bar4(long one, long two)
{
    printf(" Hello from Bar4: one = %ld, two = %ld!\n ", one, two);

    return (void*)0xc0decafe00;
}

EXPORT_SYM void* Blue4(long one, long two)
{
    printf(" Hello from Blue4: one = %ld, two = %ld!\n ", one, two);

    return (void*)0xdeadbeef00;
}

EXPORT_SYM short Bar13(short s1, short s2, unsigned short us1, long l1, long l2, unsigned long ul1, short s3, short s4,
                       unsigned short us2, long l3, long l4, unsigned long ul2)
{
    printf(" Hello from Bar13: \n");
    printf(" s1 = %hd, s2 = %hd, us1 = %hu\n ", s1, s2, us1);
    printf(" s3 = %hd, s4 = %hd, us2 = %hu\n ", s3, s4, us2);
    printf(" l1 = %ld, l2 = %ld, ul1 = %lu\n ", l1, l2, ul1);
    printf(" l3 = %ld, l4 = %ld, ul2 = %lu\n ", l3, l4, ul2);

    return 1;
}

EXPORT_SYM short Green13(short s1, short s2, unsigned short us1, long l1, long l2, unsigned long ul1, short s3, short s4,
                         unsigned short us2, long l3, long l4, unsigned long ul2)
{
    printf(" Green13: ");

    if (s1 == SHRT_MIN && s2 == SHRT_MAX && us1 == USHRT_MAX && l1 == LONG_MIN && l2 == LONG_MAX && ul1 == ULONG_MAX &&
        s3 == SHRT_MIN && s4 == SHRT_MAX && us2 == USHRT_MAX && l3 == LONG_MIN && l4 == LONG_MAX && ul2 == ULONG_MAX)
    {
        printf(" Test passed\n");
        return 1;
    }

    else
    {
        printf(" Test failed\n");

        if (s1 != SHRT_MIN) printf(" s1 != SHRT_MIN\n");

        if (s2 != SHRT_MAX) printf(" s2 != SHRT_MIN\n");

        if (us1 != USHRT_MAX) printf("  us1 != USHRT_MAX\n");

        if (l1 != LONG_MIN) printf(" l1 != LONG_MIN\n");

        if (l2 != LONG_MAX) printf(" l2 != LONG_MAX\n");

        if (ul1 != ULONG_MAX) printf(" ul1 == ULONG_MAX\n");

        if (s3 != SHRT_MIN) printf(" s3 != SHRT_MIN\n");

        if (s4 != SHRT_MAX) printf(" s4 != SHRT_MAX\n");

        if (us2 != USHRT_MAX) printf(" us2 != USHRT_MAX\n");

        if (l3 != LONG_MIN) printf(" l3 != LONG_MIN\n");

        if (l4 != LONG_MAX) printf(" l4 != LONG_MAX\n");

        if (ul2 != ULONG_MAX) printf(" ul2 != LONG_MAX\n");
    }

    return 1;
}
