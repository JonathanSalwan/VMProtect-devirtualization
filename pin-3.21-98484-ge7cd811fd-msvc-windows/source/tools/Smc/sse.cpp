/*
 * Copyright (C) 2005-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <iostream>
#include <iomanip>
using std::cout;
using std::endl;
#define N 1024
int main(int argc, char** argv);

#if defined(__GNUC__)

#include <stdint.h>
typedef uint8_t UINT8; //LINUX HOSTS
typedef uint16_t UINT16;
typedef uint32_t UINT32;
typedef uint64_t UINT64;
typedef int8_t INT8;
typedef int16_t INT16;
typedef int32_t INT32;
typedef int64_t INT64;

#define ALIGN16 __attribute__((aligned(16)))
#define ALIGN8 __attribute__((aligned(8)))

#elif defined(_MSC_VER)

typedef unsigned __int8 UINT8;
typedef unsigned __int16 UINT16;
typedef unsigned __int32 UINT32;
typedef unsigned __int64 UINT64;
typedef __int8 INT8;
typedef __int16 INT16;
typedef __int32 INT32;
typedef __int64 INT64;

#define ALIGN16 __declspec(align(16))
#define ALIGN8 __declspec(align(8))

#else
#error Expect usage of either GNU or MS compiler.
#endif

#define MAX_XMM_REGS 16
#define MAX_BYTES_PER_XMM_REG 16
#define MAX_WORDS_PER_XMM_REG (MAX_BYTES_PER_XMM_REG / 2)
#define MAX_DWORDS_PER_XMM_REG (MAX_WORDS_PER_XMM_REG / 2)
#define MAX_QWORDS_PER_XMM_REG (MAX_DWORDS_PER_XMM_REG / 2)
#define MAX_FLOATS_PER_XMM_REG (MAX_BYTES_PER_XMM_REG / sizeof(float))
#define MAX_DOUBLES_PER_XMM_REG (MAX_BYTES_PER_XMM_REG / sizeof(double))

#define MAX_MMX_REGS 8
#define MAX_BYTES_PER_MMX_REG 8
#define MAX_WORDS_PER_MMX_REG (MAX_BYTES_PER_MMX_REG / 2)
#define MAX_DWORDS_PER_MMX_REG (MAX_WORDS_PER_MMX_REG / 2)
#define MAX_QWORDS_PER_MMX_REG (MAX_DWORDS_PER_MMX_REG / 2)
#define MAX_FLOATS_PER_MMX_REG (MAX_BYTES_PER_MMX_REG / sizeof(float))
#define MAX_DOUBLES_PER_MMX_REG (MAX_BYTES_PER_MMX_REG / sizeof(double))

union ALIGN16 xmm_reg_t
{
    UINT8 byte[MAX_BYTES_PER_XMM_REG];
    UINT16 word[MAX_WORDS_PER_XMM_REG];
    UINT32 dword[MAX_DWORDS_PER_XMM_REG];
    UINT64 qword[MAX_QWORDS_PER_XMM_REG];

    INT8 s_byte[MAX_BYTES_PER_XMM_REG];
    INT16 s_word[MAX_WORDS_PER_XMM_REG];
    INT32 s_dword[MAX_DWORDS_PER_XMM_REG];
    INT64 s_qword[MAX_QWORDS_PER_XMM_REG];

    float flt[MAX_FLOATS_PER_XMM_REG];
    double dbl[MAX_DOUBLES_PER_XMM_REG];
};

union ALIGN8 mmx_reg_t
{
    UINT8 byte[MAX_BYTES_PER_MMX_REG];
    UINT16 word[MAX_WORDS_PER_MMX_REG];
    UINT32 dword[MAX_DWORDS_PER_MMX_REG];
    UINT64 qword[MAX_QWORDS_PER_MMX_REG];

    INT8 s_byte[MAX_BYTES_PER_MMX_REG];
    INT16 s_word[MAX_WORDS_PER_MMX_REG];
    INT32 s_dword[MAX_DWORDS_PER_MMX_REG];
    INT64 s_qword[MAX_QWORDS_PER_MMX_REG];

    float flt[MAX_FLOATS_PER_MMX_REG];
    double dbl[MAX_DOUBLES_PER_MMX_REG];
};

#if defined(__GNUC__)
static void set_xmm_reg0(xmm_reg_t& xmm_reg) { asm volatile("movdqu %0, %%xmm0" : : "m"(xmm_reg) : "%xmm0"); }
#elif defined(_MSC_VER)
extern "C" void set_xmm_reg0(xmm_reg_t& xmm_reg);
#endif

#if defined(__GNUC__)
static void get_xmm_reg0(xmm_reg_t& xmm_reg) { asm volatile("movdqu %%xmm0,%0" : "=m"(xmm_reg)); }
#elif defined(_MSC_VER)
extern "C" void get_xmm_reg0(xmm_reg_t& xmm_reg);
#endif

#if defined(__GNUC__)
static void set_mmx_reg0(mmx_reg_t& mmx_reg) { asm volatile("movq %0, %%mm0" : : "m"(mmx_reg) : "%mm0"); }
#elif defined(_MSC_VER)
extern "C" void set_mmx_reg0(mmx_reg_t& mmx_reg);
#endif

#if defined(__GNUC__)
static void get_mmx_reg0(mmx_reg_t& mmx_reg) { asm volatile("movq %%mm0,%0" : "=m"(mmx_reg)); }
#elif defined(_MSC_VER)
extern "C" void get_mmx_reg0(mmx_reg_t& mmx_reg);
#endif

UINT32 init_sse(UINT32 z)
{
    xmm_reg_t xmm;
    xmm.dword[0] = z;
    set_xmm_reg0(xmm); // from memory to register -- we modify the output using the tool
    get_xmm_reg0(xmm); // from register to memory
    return xmm.dword[0];
}

UINT32 init_mmx(UINT32 z)
{
    mmx_reg_t mmx;
    mmx.dword[0] = z;
    set_mmx_reg0(mmx); // from mem to register -- we modify the output of this one
    get_mmx_reg0(mmx); // from register to memory
    return mmx.dword[0];
}

/* This part replaces the library version of atoi which causes problems in Intel(R) 64 on windows */

bool isdigit(char c) { return ((c >= '0') && (c <= '9')); }

int digitconv(char c) { return (c - '0'); }

bool iswhitespace(char c) { return ((c == ' ') || (c == '\t') || (c == '\n')); }

int str2int(char* str, int base = 10)
{
    const int maxdigits = 9;
    if (NULL == str)
    {
        return 0;
    }
    int it = 0;
    //Preceding whitespaces
    while (iswhitespace(str[it]))
        ++it;
    //Negative number?
    bool neg = false;
    if (str[it] == '-')
    {
        neg = true;
        ++it;
    }
    else if (str[it] == '+')
    {
        neg = false;
        ++it;
    }

    int num = 0;
    for (int i = 0; (i < maxdigits) && (isdigit(str[it])); ++i, ++it)
    /* Note that since the null char is not a digit the program will behave correctly. */
    {
        num *= base;
        num += digitconv(str[it]);
    }
    return num * (neg ? -1 : 1);
}
/* End of atoi replacement */

int main(int argc, char** argv)
{
    UINT32 x = init_sse(str2int(argv[1]));
    cout << x << endl;
    UINT32 y = init_mmx(x);
    cout << y << endl;
    return 0;
}
