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

EXPORT_SYM void Bar1(int one) { printf(" Hello from Bar1: one = %d!\n ", one); }

EXPORT_SYM int Bar2(int one, int two)
{
    printf(" Hello from Bar2: one = %d, two = %d!\n ", one, two);

    return one + two;
}

EXPORT_SYM long Bar3(long one, long two)
{
    printf(" Hello from Bar3: one = %ld, two = %ld!\n ", one, two);

    return one + two;
}

EXPORT_SYM void Blue1(int one) { printf(" Hello from Blue1: one = %d!\n ", one); }

EXPORT_SYM int Blue2(int one, int two)
{
    printf(" Hello from Blue2: one = %d, two = %d!\n ", one, two);

    return one + two;
}

EXPORT_SYM long Blue3(long one, long two)
{
    printf(" Hello from Blue3: one = %ld, two = %ld!\n ", one, two);

    return one + two;
}

EXPORT_SYM int Blue6(int one, int two)
{
    printf("Hello from Blue6: one = %d, two = %d!\n ", one, two);

    return one + two;
}

EXPORT_SYM int Bar6(int one, int two)
{
    int sum = 0;

    printf(" Hello from Bar6: one = %d, two = %d!\n ", one, two);

    sum = Blue6(one, two);

    return sum;
}

EXPORT_SYM int Blue7(int one, int two);

EXPORT_SYM int Bar7(int one, int two, int stop)
{
    int sum = 0;

    if (stop == 0)
    {
        printf(" Bar7: once\n");
        sum = Blue7(one, two);
    }
    else
    {
        printf(" Bar7: twice\n");
        sum = one + two;
    }

    return sum;
}

EXPORT_SYM int Blue7(int one, int two)
{
    int sum = 0;

    printf(" Blue7: once\n");

    sum = Bar7(one, two, 1);

    printf(" Blue7: twice\n");

    return sum;
}

EXPORT_SYM int Bar8(int one, int two, int stop)
{
    int sum = 0;

    if (stop == 0)
    {
        printf(" Bar8: once\n");
        sum = Bar8(one, two, 1);
    }
    else if (stop == 1)
    {
        printf(" Bar8: twice\n");
        sum = Bar8(one, two, 2);
    }
    else
    {
        printf(" Bar8: thrice\n");
        sum = one + two;
    }
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

EXPORT_SYM int Blue10(int one, int two, int three, int four, int five, int six, int seven, int eight, int nine, int zero)
{
    int sum = 0;

    printf(" Hello from Blue10: one = %d, two = %d\n ", one, two);
    printf(" three = %d, four = %d, five = %d, six = %d\n ", three, four, five, six);
    printf(" seven = %d, eight = %d, nine = %d, zero = %d\n ", seven, eight, nine, zero);

    sum = one + two + three + four + five + six + seven + eight + nine + zero;

    printf(" sum = %d\n", sum);

    return sum;
}

#if defined(TARGET_WINDOWS) && defined(TARGET_IA32)

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

extern __declspec(dllexport) int FAST_CALL
    FastBlue10(int one, int two, int three, int four, int five, int six, int seven, int eight, int nine, int zero)
{
    int sum = 0;

    printf(" Hello from FastBlue10: one = %d, two = %d\n ", one, two);
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

extern __declspec(dllexport) int STD_CALL
    StdBlue10(int one, int two, int three, int four, int five, int six, int seven, int eight, int nine, int zero)
{
    int sum = 0;

    printf(" Hello from StdBlue10: one = %d, two = %d\n ", one, two);
    printf(" three = %d, four = %d, five = %d, six = %d\n ", three, four, five, six);
    printf(" seven = %d, eight = %d, nine = %d, zero = %d\n ", seven, eight, nine, zero);

    sum = one + two + three + four + five + six + seven + eight + nine + zero;

    printf(" sum = %d\n", sum);

    return sum;
}
#endif

EXPORT_SYM char Bar12(int i1, int i2, unsigned int ui1, signed char c1, signed char c2, unsigned char uc1, int i3, int i4,
                      unsigned int ui2, signed char c3, signed char c4, unsigned char uc2)
{
    printf(" Hello from Bar12: \n");
    printf(" i1 = %d, i2 = %d, ui1 = %d\n ", i1, i2, ui1);
    printf(" c1 = %c, c2 = %c, uc1 = %c\n ", c1, c2, uc1);
    printf(" i3 = %d, i4 = %d, ui2 = %d\n ", i3, i4, ui2);
    printf(" c1 = %c, c2 = %c, uc1 = %c\n ", c3, c4, uc2);

    return '1';
}

EXPORT_SYM char Green12(int i1, int i2, unsigned int ui1, signed char c1, signed char c2, unsigned char uc1, int i3, int i4,
                        unsigned int ui2, signed char c3, signed char c4, unsigned char uc2)
{
    printf(" Green12: ");

    if (i1 == INT_MIN && i2 == INT_MAX && ui1 == UINT_MAX && c1 == SCHAR_MIN && c2 == SCHAR_MAX && uc1 == UCHAR_MAX &&
        i3 == INT_MIN && i4 == INT_MAX && ui2 == UINT_MAX && c3 == SCHAR_MIN && c4 == SCHAR_MAX && uc2 == UCHAR_MAX)
    {
        printf(" Test passed\n");
        return '1';
    }

    else
    {
        printf(" Test failed\n");

        if (i1 != INT_MIN) printf(" i1 != INT_MIN\n");

        if (i2 != INT_MAX) printf(" i1 != INT_MIN\n");

        if (ui1 != UINT_MAX) printf("  ui1 != UINT_MAX\n");

        if (c1 != SCHAR_MIN) printf(" c1 != SCHAR_MIN\n");

        if (c2 != SCHAR_MAX) printf(" c2 != SCHAR_MAX\n");

        if (uc1 != UCHAR_MAX) printf(" uc1 == UCHAR_MAX\n");

        if (i3 != INT_MIN) printf(" i3 != INT_MIN\n");

        if (i4 != INT_MAX) printf(" i4 != INT_MAX\n");

        if (ui2 != UINT_MAX) printf(" ui2 != UINT_MAX\n");

        if (c3 != SCHAR_MIN) printf(" c3 != SCHAR_MIN\n");

        if (c4 != SCHAR_MAX) printf(" c4 != SCHAR_MAX\n");

        if (uc2 != UCHAR_MAX) printf(" uc2 != UCHAR_MAX\n");
    }

    return '1';
}
