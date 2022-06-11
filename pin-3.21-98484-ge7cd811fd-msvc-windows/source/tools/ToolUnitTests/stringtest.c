/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Simple string test
 */

/* Windows 32 bit implementation */
#if (defined(_WIN32))
#if (!defined(_M_X64))
#define WIN_CODE(...) __VA_ARGS__
#define WIN32_CODE(...) __VA_ARGS__
#define WIN64_CODE(...)
#define LINUX_CODE(...)
#define LINUX32_CODE(...)
#define LINUX64_CODE(...)
#else
/* Windows 64 bit implementation */
#define WIN_CODE(...) __VA_ARGS__
#define WIN32_CODE(...)
#define WIN64_CODE(...) __VA_ARGS__
#define LINUX_CODE(...)
#define LINUX32_CODE(...)
#define LINUX64_CODE(...)
#endif
#else
/* Not windows, presumably a GNU compiler and (likely) Linux */
#if (!defined(_M_X64))
#define WIN_CODE(...)
#define WIN32_CODE(...)
#define WIN64_CODE(...)
#define LINUX_CODE(...) __VA_ARGS__
#define LINUX32_CODE(...) __VA_ARGS__
#define LINUX64_CODE(...)
#else
/* Linux 64 bit implementations */
#define WIN_CODE(...)
#define WIN32_CODE(...)
#define WIN64_CODE(...)
#define LINUX_CODE(...) __VA_ARGS__
#define LINUX32_CODE(...)
#define LINUX64_CODE(...) __VA_ARGS__
#endif
#endif

static void movsb(char* dest, const char* src, int len, int decrement)
{
    if (decrement)
    {
        WIN_CODE(__asm std)
        LINUX_CODE(__asm__ volatile("std");)
    }

    WIN32_CODE(__asm mov esi, src __asm mov edi, dest __asm mov ecx, len __asm rep movsb)

    WIN64_CODE(__asm mov rsi, src __asm mov rdi, dest __asm mov rcx, len __asm rep movsb)

    LINUX_CODE(__asm__ volatile("rep movsb" ::"S"(src), "D"(dest), "c"(len) : "memory");)

    if (decrement)
    {
        WIN_CODE(__asm cld)
        LINUX_CODE(__asm__ volatile("cld");)
    }
}

static void movsd(void* dest, const void* src, int len, int decrement)
{
    if (decrement)
    {
        WIN_CODE(__asm std)
        LINUX_CODE(__asm__ volatile("std");)
    }

    WIN32_CODE(__asm mov esi, src __asm mov edi, dest __asm mov ecx, len __asm rep movsd)

    WIN64_CODE(__asm mov rsi, src __asm mov rdi, dest __asm mov rcx, len __asm rep movsd)

    LINUX_CODE(__asm__ volatile("rep movsd" ::"S"(src), "D"(dest), "c"(len) : "memory");)

    if (decrement)
    {
        WIN_CODE(__asm cld)
        LINUX_CODE(__asm__ volatile("cld");)
    }
}

/* Use repne scasb to calculate length of a string. 
 */
static int length(const char* src, int maxlen)
{
    const char* s = src;

    WIN32_CODE(__asm mov edi, src __asm mov al, 0 __asm mov ecx, maxlen __asm repne scasb __asm mov src, edi)

    WIN64_CODE(__asm mov rdi, src __asm mov al, 0 __asm mov rcx, maxlen __asm repne scasb __asm mov src, rdi)

    LINUX_CODE(__asm__ volatile("xor %%al,%%al; repne scasb" : "=D"(src) : "0"(src), "c"(maxlen) : "%eax");)

    return src - s;
}

static int cmps(const char* s1, const char* s2, int maxlen, int decrement)
{
    int res = 1;

    if (decrement)
    {
        WIN_CODE(__asm std)
        LINUX_CODE(__asm__ volatile("std");)
    }

    LINUX_CODE(__asm__ volatile("repz cmpsb\n"
                                "cmovz %2,%0\n"
                                "jge   1f\n"
                                "neg   %0\n"
                                "1:"
                                : "=r"(res)
                                : "0"(res), "r"(0), "S"(s1), "D"(s2), "c"(maxlen));)

    if (decrement)
    {
        WIN_CODE(__asm cld)
        LINUX_CODE(__asm__ volatile("cld");)
    }

    return res;
}

/* Use rep stosb to fill a chunk of store. */
static void fill(char* target, char value, int count)
{
    WIN32_CODE(__asm mov edi, target __asm mov al, value __asm mov ecx, count __asm rep stosb)

    WIN64_CODE(__asm mov rdi, target __asm mov al, value __asm mov rcx, count __asm rep stosb)

    LINUX_CODE(__asm__ volatile("mov %1,%%al; rep stosb"
                                : "=D"(target)
                                : "m"(value), "c"(count), "0"(target)
                                : "%eax", "memory");)
}

int copyAndTest(int* dest, int* src, int len, int df)
{
    int i;
    int failed = 0;

    memset(dest, 0xff, (len + 2) * sizeof(int));
    for (i = 0; i < len; i++)
        src[i] = i;

    dest = dest + 1;

    if (df)
        movsd(dest + len - 1, src + len - 1, len, df);
    else
        movsd(dest, src, len, df);

    // Compare the results
    for (i = 0; i < len; i++)
    {
        if (src[i] != dest[i])
        {
            failed++;
            printf("***Failed : copy %p to %p for %d %swards, at dest[%d] see %d not %d\n", src, dest, len, df ? "back" : "for",
                   i, dest[i], src[i]);
        }
    }
    if (dest[-1] != -1)
    {
        failed++;
        printf("***Failed : %swards, overwrote below base (%d)\n", df ? "back" : "for", dest[-1]);
    }
    if (dest[len] != -1)
    {
        failed++;
        printf("***Failed : %swards, overwrote above top (%d)\n", df ? "back" : "for", dest[len]);
    }

    return failed;
}

int testMovsd()
{
    enum
    {
        length = 121
    };

    int* s1 = (int*)malloc(length * sizeof(int));
    int* d1 = (int*)malloc((length + 2) * sizeof(int));

    int df;
    int failures = 0;

    for (df = 0; df < 2; df++)
    {
        failures += copyAndTest(d1, s1, length, df);
    }

    printf("%d failures in testMovsd\n", failures);
    return failures;
}

int doCmpsTests()
{
    const char* s1 = "abcdefg";
    const char* s2 = "abcdefh";
    const char* s3 = &s2[1];
    int df         = 0;
    int results[4];
    int failures = 0;

    printf("s1 : '%s' @%p\n"
           "s2 : '%s' @%p\n",
           s1, s1, s2, s2);

    for (df = 0; df < 2; df++)
    {
        int d1 = df ? 6 : 0;
        int d2 = df ? 5 : 0;

        printf("DF = %d\n", df);

        results[0] = cmps(s1 + d1, s1 + d1, 7, df);
        results[1] = cmps(s1 + d1, s2 + d1, 7, df);
        results[2] = cmps(s2 + d1, s1 + d1, 7, df);
        results[3] = cmps(s3 + d2, s2 + d2, 6, df);

        printf("cmps (%s,%s) = %d, should be 0\n", s1, s1, results[0]);
        printf("cmps (%s,%s) = %d, should be -1\n", s1, s2, results[1]);
        printf("cmps (%s,%s) = %d, should be 1\n", s2, s1, results[2]);
        printf("cmps (%s,%s) = %d, should be 1\n", s3, s2, results[3]);
        failures += results[0] != 0;
        failures += results[1] != -1;
        failures += results[2] != 1;
        failures += results[3] != 1;
    }
    return failures;
}

int doTest()
{
    const char* src = "Hello World";
    char dest[50];
    char d1[101];
    int len = strlen(src) + 1;
    int i;
    int failures = 0;

    movsb(&dest[0], src, len, 0);
    printf("Result after copy  '%s'\n", dest);
    if (strcmp(dest, src) != 0)
    {
        printf("*** Copy failed ***\n");
        failures++;
    }

    memset(dest, 0, sizeof(dest));

    movsb(&dest[len], &src[len], len + 1, 1);
    printf("Result after reversed copy  '%s'\n", dest);
    if (strcmp(dest, src) != 0)
    {
        printf("*** Copy failed ***\n");
        failures++;
    }

    printf("src = '%s'\n", src);
    i = length(src, 1024);
    printf("Length gives %d\n", i);

    movsb(&dest[0], src, len, 0);
    printf("dest = '%s'\n", dest);

    movsb(&dest[0], &dest[6], len - 6, 0);
    printf("Result after overlapping copy  '%s'\n", dest);

    memset(d1, -1, 100);
    d1[100] = 99;
    fill(d1, (char)1, 100);
    printf("Filled\n");
    for (i = 0; i < 100; i++)
        if (d1[i] != 1)
        {
            printf("Fill failed d1[%d] == 0x%02x\n", i, d1[i]);
            failures++;
        }
    if (d1[100] != 99)
    {
        printf("Fill failed d1[101] == 0x%02x\n", d1[100]);
        failures++;
    }
    return failures;
}

int main(int argc, char** argv)
{
    int failures = 0;
    printf("Simple tests\n");
    failures += doTest();

    printf("movsd tests\n");
    failures += testMovsd();

    printf("\n\ncmps tests\n");
    failures += doCmpsTests(0);

    return failures;
}
