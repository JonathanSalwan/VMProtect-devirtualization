/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// this application is for use in the set_fp_context_xmm_regs.test, in conjunction with the
// set_fp_contest_xmm_regs tool
#if defined(TARGET_WINDOWS)
#include <windows.h>
#include <string>
#include <iostream>
#include <memory.h>
#define EXPORT_CSYM extern "C" __declspec(dllexport)
#else
//Linux:
#include <signal.h>
#ifndef TARGET_MAC
#include <ucontext.h>
#endif
#include <stdio.h>
#include <setjmp.h>
#include <assert.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <memory.h>
#define EXPORT_CSYM extern "C"
#endif

#include <stdio.h>
#include "../Utils/threadlib.h"

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

/*
typedef unsigned __int8 UINT8 ;
typedef unsigned __int16 UINT16;
typedef unsigned __int32 UINT32;
typedef unsigned __int64 UINT64;
typedef __int8 INT8;
typedef __int16 INT16;
typedef __int32 INT32;
typedef __int64 INT64;
*/
#define ALIGN16 __declspec(align(16))
#define ALIGN8 __declspec(align(8))

#else
#error Expect usage of either GNU or MS compiler.
#endif

#define MAX_BYTES_PER_XMM_REG 16
#define MAX_WORDS_PER_XMM_REG (MAX_BYTES_PER_XMM_REG / 2)
#define MAX_DWORDS_PER_XMM_REG (MAX_WORDS_PER_XMM_REG / 2)
#define MAX_QWORDS_PER_XMM_REG (MAX_DWORDS_PER_XMM_REG / 2)
#define MAX_FLOATS_PER_XMM_REG (MAX_BYTES_PER_XMM_REG / sizeof(float))
#define MAX_DOUBLES_PER_XMM_REG (MAX_BYTES_PER_XMM_REG / sizeof(double))

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
#if defined(_MSC_VER)
extern "C" INT64 get_RDI();
extern "C" void set_xmm_reg0(xmm_reg_t& xmm_reg);
extern "C" void get_xmm_reg0(xmm_reg_t& xmm_reg);
extern "C" void set_xmm_reg1(xmm_reg_t& xmm_reg);
extern "C" void get_xmm_reg1(xmm_reg_t& xmm_reg);
extern "C" void set_xmm_reg2(xmm_reg_t& xmm_reg);
extern "C" void get_xmm_reg2(xmm_reg_t& xmm_reg);
extern "C" void set_xmm_reg3(xmm_reg_t& xmm_reg);
extern "C" void get_xmm_reg3(xmm_reg_t& xmm_reg);
extern "C" void set_xmm_reg4(xmm_reg_t& xmm_reg);
extern "C" void get_xmm_reg4(xmm_reg_t& xmm_reg);
extern "C" void set_xmm_reg5(xmm_reg_t& xmm_reg);
extern "C" void get_xmm_reg5(xmm_reg_t& xmm_reg);
extern "C" void set_xmm_reg6(xmm_reg_t& xmm_reg);
extern "C" void get_xmm_reg6(xmm_reg_t& xmm_reg);
extern "C" void set_xmm_reg7(xmm_reg_t& xmm_reg);
extern "C" void get_xmm_reg7(xmm_reg_t& xmm_reg);
#ifdef TARGET_IA32E
extern "C" void set_xmm_reg8(xmm_reg_t& xmm_reg);
extern "C" void get_xmm_reg8(xmm_reg_t& xmm_reg);
extern "C" void set_xmm_reg9(xmm_reg_t& xmm_reg);
extern "C" void get_xmm_reg9(xmm_reg_t& xmm_reg);
extern "C" void set_xmm_reg10(xmm_reg_t& xmm_reg);
extern "C" void get_xmm_reg10(xmm_reg_t& xmm_reg);
extern "C" void set_xmm_reg11(xmm_reg_t& xmm_reg);
extern "C" void get_xmm_reg11(xmm_reg_t& xmm_reg);
extern "C" void set_xmm_reg12(xmm_reg_t& xmm_reg);
extern "C" void get_xmm_reg12(xmm_reg_t& xmm_reg);
extern "C" void set_xmm_reg13(xmm_reg_t& xmm_reg);
extern "C" void get_xmm_reg13(xmm_reg_t& xmm_reg);
extern "C" void set_xmm_reg14(xmm_reg_t& xmm_reg);
extern "C" void get_xmm_reg14(xmm_reg_t& xmm_reg);
extern "C" void set_xmm_reg15(xmm_reg_t& xmm_reg);
extern "C" void get_xmm_reg15(xmm_reg_t& xmm_reg);
#endif
extern "C" void mmx_save(char* buf);
extern "C" void mmx_restore(char* buf);
#else
static INT64 get_RDI() { asm("movl %edi, %eax"); }
static void set_xmm_reg0(xmm_reg_t& xmm_reg) { asm volatile("movdqu %0, %%xmm0" : : "m"(xmm_reg) : "%xmm0"); }
static void get_xmm_reg0(xmm_reg_t& xmm_reg) { asm volatile("movdqu %%xmm0,%0" : "=m"(xmm_reg)); }
static void set_xmm_reg1(xmm_reg_t& xmm_reg) { asm volatile("movdqu %0, %%xmm1" : : "m"(xmm_reg) : "%xmm1"); }
static void get_xmm_reg1(xmm_reg_t& xmm_reg) { asm volatile("movdqu %%xmm1,%0" : "=m"(xmm_reg)); }
static void set_xmm_reg2(xmm_reg_t& xmm_reg) { asm volatile("movdqu %0, %%xmm2" : : "m"(xmm_reg) : "%xmm2"); }
static void get_xmm_reg2(xmm_reg_t& xmm_reg) { asm volatile("movdqu %%xmm2,%0" : "=m"(xmm_reg)); }
static void set_xmm_reg3(xmm_reg_t& xmm_reg) { asm volatile("movdqu %0, %%xmm3" : : "m"(xmm_reg) : "%xmm3"); }
static void get_xmm_reg3(xmm_reg_t& xmm_reg) { asm volatile("movdqu %%xmm3,%0" : "=m"(xmm_reg)); }
static void set_xmm_reg4(xmm_reg_t& xmm_reg) { asm volatile("movdqu %0, %%xmm4" : : "m"(xmm_reg) : "%xmm4"); }
static void get_xmm_reg4(xmm_reg_t& xmm_reg) { asm volatile("movdqu %%xmm4,%0" : "=m"(xmm_reg)); }
static void set_xmm_reg5(xmm_reg_t& xmm_reg) { asm volatile("movdqu %0, %%xmm5" : : "m"(xmm_reg) : "%xmm5"); }
static void get_xmm_reg5(xmm_reg_t& xmm_reg) { asm volatile("movdqu %%xmm5,%0" : "=m"(xmm_reg)); }
static void set_xmm_reg6(xmm_reg_t& xmm_reg) { asm volatile("movdqu %0, %%xmm6" : : "m"(xmm_reg) : "%xmm6"); }
static void get_xmm_reg6(xmm_reg_t& xmm_reg) { asm volatile("movdqu %%xmm6,%0" : "=m"(xmm_reg)); }
static void set_xmm_reg7(xmm_reg_t& xmm_reg) { asm volatile("movdqu %0, %%xmm7" : : "m"(xmm_reg) : "%xmm7"); }
static void get_xmm_reg7(xmm_reg_t& xmm_reg) { asm volatile("movdqu %%xmm7,%0" : "=m"(xmm_reg)); }
#ifdef TARGET_IA32E
static void set_xmm_reg8(xmm_reg_t& xmm_reg) { asm volatile("movdqu %0, %%xmm8" : : "m"(xmm_reg) : "%xmm8"); }
static void get_xmm_reg8(xmm_reg_t& xmm_reg) { asm volatile("movdqu %%xmm8,%0" : "=m"(xmm_reg)); }
static void set_xmm_reg9(xmm_reg_t& xmm_reg) { asm volatile("movdqu %0, %%xmm9" : : "m"(xmm_reg) : "%xmm9"); }
static void get_xmm_reg9(xmm_reg_t& xmm_reg) { asm volatile("movdqu %%xmm9,%0" : "=m"(xmm_reg)); }
static void set_xmm_reg10(xmm_reg_t& xmm_reg) { asm volatile("movdqu %0, %%xmm10" : : "m"(xmm_reg) : "%xmm10"); }
static void get_xmm_reg10(xmm_reg_t& xmm_reg) { asm volatile("movdqu %%xmm10,%0" : "=m"(xmm_reg)); }
static void set_xmm_reg11(xmm_reg_t& xmm_reg) { asm volatile("movdqu %0, %%xmm11" : : "m"(xmm_reg) : "%xmm11"); }
static void get_xmm_reg11(xmm_reg_t& xmm_reg) { asm volatile("movdqu %%xmm11,%0" : "=m"(xmm_reg)); }
static void set_xmm_reg12(xmm_reg_t& xmm_reg) { asm volatile("movdqu %0, %%xmm12" : : "m"(xmm_reg) : "%xmm12"); }
static void get_xmm_reg12(xmm_reg_t& xmm_reg) { asm volatile("movdqu %%xmm12,%0" : "=m"(xmm_reg)); }
static void set_xmm_reg13(xmm_reg_t& xmm_reg) { asm volatile("movdqu %0, %%xmm13" : : "m"(xmm_reg) : "%xmm13"); }
static void get_xmm_reg13(xmm_reg_t& xmm_reg) { asm volatile("movdqu %%xmm13,%0" : "=m"(xmm_reg)); }
static void set_xmm_reg14(xmm_reg_t& xmm_reg) { asm volatile("movdqu %0, %%xmm14" : : "m"(xmm_reg) : "%xmm14"); }
static void get_xmm_reg14(xmm_reg_t& xmm_reg) { asm volatile("movdqu %%xmm14,%0" : "=m"(xmm_reg)); }
static void set_xmm_reg15(xmm_reg_t& xmm_reg) { asm volatile("movdqu %0, %%xmm15" : : "m"(xmm_reg) : "%xmm15"); }
static void get_xmm_reg15(xmm_reg_t& xmm_reg) { asm volatile("movdqu %%xmm15,%0" : "=m"(xmm_reg)); }
#endif
#endif

static void set_xmm_reg(xmm_reg_t& xmm_reg, UINT32 reg_no)
{
    switch (reg_no)
    {
        case 0:
            set_xmm_reg0(xmm_reg);
            break;
        case 1:
            set_xmm_reg1(xmm_reg);
            break;
        case 2:
            set_xmm_reg2(xmm_reg);
            break;
        case 3:
            set_xmm_reg3(xmm_reg);
            break;
        case 4:
            set_xmm_reg4(xmm_reg);
            break;
        case 5:
            set_xmm_reg5(xmm_reg);
            break;
        case 6:
            set_xmm_reg6(xmm_reg);
            break;
        case 7:
            set_xmm_reg7(xmm_reg);
            break;
#ifdef TARGET_IA32E
        case 8:
            set_xmm_reg8(xmm_reg);
            break;
        case 9:
            set_xmm_reg9(xmm_reg);
            break;
        case 10:
            set_xmm_reg10(xmm_reg);
            break;
        case 11:
            set_xmm_reg11(xmm_reg);
            break;
        case 12:
            set_xmm_reg12(xmm_reg);
            break;
        case 13:
            set_xmm_reg13(xmm_reg);
            break;
        case 14:
            set_xmm_reg14(xmm_reg);
            break;
        case 15:
            set_xmm_reg15(xmm_reg);
            break;
#endif
    }
}
static void get_xmm_reg(xmm_reg_t& xmm_reg, UINT32 reg_no)
{
    switch (reg_no)
    {
        case 0:
            get_xmm_reg0(xmm_reg);
            break;
        case 1:
            get_xmm_reg1(xmm_reg);
            break;
        case 2:
            get_xmm_reg2(xmm_reg);
            break;
        case 3:
            get_xmm_reg3(xmm_reg);
            break;
        case 4:
            get_xmm_reg4(xmm_reg);
            break;
        case 5:
            get_xmm_reg5(xmm_reg);
            break;
        case 6:
            get_xmm_reg6(xmm_reg);
            break;
        case 7:
            get_xmm_reg7(xmm_reg);
            break;
#ifdef TARGET_IA32E
        case 8:
            get_xmm_reg8(xmm_reg);
            break;
        case 9:
            get_xmm_reg9(xmm_reg);
            break;
        case 10:
            get_xmm_reg10(xmm_reg);
            break;
        case 11:
            get_xmm_reg11(xmm_reg);
            break;
        case 12:
            get_xmm_reg12(xmm_reg);
            break;
        case 13:
            get_xmm_reg13(xmm_reg);
            break;
        case 14:
            get_xmm_reg14(xmm_reg);
            break;
        case 15:
            get_xmm_reg15(xmm_reg);
            break;
#endif
    }
}

//////////////////////////////////////////////////////

void write_xmm_reg(UINT32 reg_no, UINT32 val)
{
    xmm_reg_t xmm;
    xmm.dword[0] = val;
    xmm.dword[1] = val;
    xmm.dword[2] = val;
    xmm.dword[3] = val;

    set_xmm_reg(xmm, reg_no);
}

void read_xmm_reg(UINT32 reg_no, xmm_reg_t& xmm)
{
    xmm.dword[0] = 0;
    xmm.dword[1] = 0;
    xmm.dword[2] = 0;
    xmm.dword[3] = 0;

    get_xmm_reg(xmm, reg_no);
}

#ifdef TARGET_IA32E

#define NUM_XMM_REGS 16
#else
#define NUM_XMM_REGS 8
#endif

/*
 Retrieve the xmm registers and print their contents
 */
void DumpXmmRegs()
{
    xmm_reg_t xmm_regs[NUM_XMM_REGS];
    for (UINT32 i = 0; i < NUM_XMM_REGS; i++)
    {
        read_xmm_reg(i, xmm_regs[i]);
    }
    printf("DumpXmmRegs\n");
    fflush(stdout);
    for (UINT32 i = 0; i < NUM_XMM_REGS; i++)
    {
        printf("xmm%d: ", i);
        fflush(stdout);
        for (INT32 j = MAX_DWORDS_PER_XMM_REG - 1; j >= 0; j--)
        {
            printf("%x:", xmm_regs[i].dword[j]);
            fflush(stdout);
        }
        printf("\n");
        fflush(stdout);
    }
}

/*
 Retrieve the xmm registers and print their contents
 */
void SetXmmRegs(UINT32 val)
{
    xmm_reg_t xmm_regs[NUM_XMM_REGS];
    for (UINT32 i = 0; i < NUM_XMM_REGS; i++)
    {
        write_xmm_reg(i, val);
    }
}

/* when run with the set_fp_context_xmm_regs tool, the tool will have replac this
   function with a function that calls this original but first the tool
   replacement function sets the xmm regs in the context used to
   in the PIN_CallApplicationFunction used to call this original fnction
*/
EXPORT_CSYM void ReplacedXmmRegs()
{
    // just dump the contents of the xmm regs
    DumpXmmRegs();
}

/* when run with the set_fp_context_xmm_regs tool, the tool will call this
   function via the PIN_ExecuteAt, but first the tool will
   sets the xmm regs in the context used to in the PIN_ExecuteAt call
*/
EXPORT_CSYM void ExecutedAtFunc()
{
    // just dump the contents of the xmm regs
    DumpXmmRegs();
}

// the tool's OnException function directs execution to here after changing the values in the xmm registers
EXPORT_CSYM void DumpXmmRegsAtException()
{
    // just dump the contents of the xmm regs
    DumpXmmRegs();
    // and exit OK
    exit(0);
}

/* when run with the set_fp_context_xmm_regs tool, the tool sets the xmm regs in the context used to
   execute the thread - this is done in the thread creation callback received from Pin
*/
void* ThreadFunc(void* arg)
{
    xmm_reg_t xmm_regs[NUM_XMM_REGS];
    for (UINT32 i = 0; i < NUM_XMM_REGS; i++)
    {
        read_xmm_reg(i, xmm_regs[i]);
    }
    printf("DumpXmmRegs from Thread\n");
    fflush(stdout);
    for (UINT32 i = 0; i < NUM_XMM_REGS; i++)
    {
        printf("xmm%d: ", i);
        fflush(stdout);
        for (INT32 j = MAX_DWORDS_PER_XMM_REG - 1; j >= 0; j--)
        {
            printf("%x:", xmm_regs[i].dword[j]);
            fflush(stdout);
        }
        printf("\n");
        fflush(stdout);
    }
    return (NULL);
}

#if !defined(TARGET_WINDOWS)

// Linux

// the segv exception handler will never be reached, because the tool OnException function redirects the
// continuation to the DumpXmmRegsAtException
void handle(int sig, siginfo_t* info, void* vctxt)
{
    printf("Exception ocurred. Now in handler\n");
    fflush(stdout);
}
#endif

int main()
{
    int i;
    THREAD_HANDLE threadHandle;

    printf("xmm regs at ReplacedXmmRegs\n");
    fflush(stdout);
    ReplacedXmmRegs();

    /* create a thread that dumps the values in the xmm regs
    printf ("xmm regs at threads\n");
    fflush (stdout);
    CreateOneThread(&threadHandle, ThreadFunc, 0);
    JoinOneThread(threadHandle);
    */

#if !defined(TARGET_WINDOWS)
    // Linux
    // define a handler so that the tool gets the  context change callback with the CONTEXT_CHANGE_REASON_SIGNAL
    // and a valid ctxtTo
    struct sigaction sigact;

    sigact.sa_sigaction = handle;
    sigemptyset(&sigact.sa_mask);
    sigact.sa_flags = SA_SIGINFO;
    if (sigaction(SIGSEGV, &sigact, 0) == -1)
    {
        fprintf(stderr, "Unable to handle SIGSEGV signal\n");
        exit(-1);
    }
#endif

    printf("xmm regs before exception\n");
    SetXmmRegs(0xa5a5a5a5);
    DumpXmmRegs();
    printf("next print of xmm regs will be after the tool's OnException function has set their values\n");
    char* p = 0;
    p++;
    SetXmmRegs(0xa5a5a5a5);
    // the gpf here will cause the invocation of the tool's OnException function
    *p = 0;
}
