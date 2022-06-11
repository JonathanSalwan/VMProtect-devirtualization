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
#define FAST_CALL __fastcall
#define STD_CALL __stdcall
#else
#define EXPORT_SYM extern
#define FAST_CALL
#define STD_CALL
#endif

EXPORT_SYM void Bar() { printf(" Hello from Bar!\n "); }

EXPORT_SYM void Blue() { printf(" Hello from Blue!\n"); }

EXPORT_SYM int Bar9(int one, int two, int three, int four, int five, int six, int seven, int eight, int nine)
{
    int sum = 0;

    printf(" Hello from Bar10: one = %d, two = %d\n ", one, two);
    printf(" three = %d, four = %d, five = %d, six = %d\n ", three, four, five, six);
    printf(" seven = %d, eight = %d, nine = %d \n ", seven, eight, nine);

    sum = one + two + three + four + five + six + seven + eight + nine;

    printf(" sum = %d\n", sum);

    return sum;
}

EXPORT_SYM int Bar10(int one, int two, int three, int four, int five, int six, int seven, int eight, int nine, int zero)
{
    int sum = 0;

    printf(" Hello from Bar10: one = %d, two = %d\n ", one, two);
    printf(" three = %d, four = %d, five = %d, six = %d\n ", three, four, five, six);
    printf(" seven = %d, eight = %d, nine = %d, zero = %d\n ", seven, eight, nine, zero);

    sum = one + two + three + four + five + six + seven + eight + nine + zero;

    printf(" sum = %d\n", sum);

    return sum;
}

#if defined(TARGET_WINDOWS)

extern __declspec(dllexport) int FAST_CALL
    FastBar10(int one, int two, int three, int four, int five, int six, int seven, int eight, int nine, int zero)
{
    int sum;

    printf(" Hello from FastBar10: one = %d, two = %d\n ", one, two);
    printf(" three = %d, four = %d, five = %d, six = %d\n ", three, four, five, six);
    printf(" seven = %d, eight = %d, nine = %d, zero = %d\n ", seven, eight, nine, zero);

    sum = one + two + three + four + five + six + seven + eight + nine + zero;

    printf(" sum = %d\n", sum);

    return sum;
}

extern __declspec(dllexport) int STD_CALL
    StdBar10(int one, int two, int three, int four, int five, int six, int seven, int eight, int nine, int zero)
{
    int sum;

    printf(" Hello from StdBar10: one = %d, two = %d\n ", one, two);
    printf(" three = %d, four = %d, five = %d, six = %d\n ", three, four, five, six);
    printf(" seven = %d, eight = %d, nine = %d, zero = %d\n ", seven, eight, nine, zero);

    sum = one + two + three + four + five + six + seven + eight + nine + zero;

    printf(" sum = %d\n", sum);

    return sum;
}

#endif
