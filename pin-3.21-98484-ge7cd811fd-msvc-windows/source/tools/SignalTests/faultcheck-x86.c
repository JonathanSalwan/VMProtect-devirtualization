/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#define _GNU_SOURCE
#ifdef TARGET_MAC
#include <sys/ucontext.h>
#include <mach/mach.h>
#include <stdbool.h>
#else
#include <ucontext.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/utsname.h>
#include "raise-exception-addrs.h"
#include "faultcheck-target.h"

#define PAGESIZE 4096

/* When an executable is position independent, it may load into any location.
 * This causes a problem when diffing the outputs of two runs of this program.
 * We can solve this by only comparing the page offsets, which should remain
 * constant between runs. If, for debugging purposes, full addresses are
 * needed, define NORMALIZE_ADDRESSES as 0.
 */
#define NORMALIZE_ADDRESSES 1

uintptr_t normalize_addr(uintptr_t addr)
{
    if (NORMALIZE_ADDRESSES)
    {
        addr = addr & (PAGESIZE - 1);
    }
    return addr;
}

extern void DoUnmappedWrite(void*);
extern void DoUnmappedRead(void*);
extern void DoInaccessibleRead(void*);
extern void DoInaccessibleWrite(void*);
extern void DoUD2();
extern void DoPrivilegedInstruction();
extern void DoIntegerDivideByZero();
extern void DoIntegerOverflowTrap();
extern void DoX87DivideByZero();
extern void DoX87Overflow();
extern void DoX87Underflow();
extern void DoX87Precision();
extern void DoX87InvalidOperation();
extern void DoX87DenormalizedOperand();
extern void DoX87StackUnderflow();
extern void DoX87StackOverflow();
extern void DoX87MultipleExceptions();
extern void DoSIMDDivideByZero();
extern void DoSIMDOverflow();
extern void DoSIMDUnderflow();
extern void DoSIMDPrecision();
extern void DoSIMDInvalidOperation();
extern void DoSIMDDenormalizedOperand();
extern void DoSIMDMultipleExceptions();
extern void DoBreakpointTrap();
extern void DoSingleStepTrap();
extern void DoBadRegister();
extern void ClearAC();
extern void DoIllegalSetOfSegReg1();
extern void DoIllegalSetOfSegReg2();
extern void DoIllegalSetOfSegReg3();
extern void DoIllegalGetOfSegReg1();
extern void DoIllegalGetOfSegReg2();

extern char PinLab_UnmappedRead;
extern char PinLab_UnmappedWrite;
extern char PinLab_InaccessibleRead;
extern char PinLab_InaccessibleWrite;
extern char PinLab_MisalignedRead;
extern char PinLab_MisalignedWrite;
extern char PinLab_IllegalInstruction;
extern char PinLab_PrivilegedInstruction;
extern char PinLab_IntegerDivideByZero;
extern char PinLab_IntegerOverflowTrap;
extern char PinLab_X87DivideByZero;
extern char PinLab_X87Overflow;
extern char PinLab_X87Underflow;
extern char PinLab_X87Precision;
extern char PinLab_X87InvalidOperation;
extern char PinLab_X87DenormalizedOperand;
extern char PinLab_X87StackUnderflow;
extern char PinLab_X87StackOverflow;
extern char PinLab_X87MultipleExceptions;
extern char PinLab_SIMDDivideByZero;
extern char PinLab_SIMDOverflow;
extern char PinLab_SIMDUnderflow;
extern char PinLab_SIMDPrecision;
extern char PinLab_SIMDInvalidOperation;
extern char PinLab_SIMDDenormalizedOperand;
extern char PinLab_SIMDMultipleExceptions;
extern char PinLab_BreakpointTrap;

#if defined(TARGET_IA32) && !defined(TARGET_MAC)
extern void DoBoundTrap();
extern char PinLab_BoundTrap;
#endif

#define BYTES_PER_TRAP_FUNC 3 /* number bytes in each "IntTrap" function */
#define NUM_TRAP_FUNCS 256 /* number of "IntTrap" functions */

static char NoPermBuffer[2 * PAGESIZE];
static void* PageNoPerm = 0;
static char MisalignedBuffer[16];
static void* MisalignedAddress = 0;
static void* UnmappedAddress   = (void*)8;
#if !defined(TARGET_MAC)
static unsigned char IntTrapBuffer[3 * PAGESIZE];
#endif
static unsigned char* IntTrapCode;

static int IsBadKernel;
static int PrintSiAddr;
static int PrintMxcsr;
static int PrintX87Status;

/*
 * Do one-time initializations for the tests.  Returns 1 on success, 0 on failure.
 */
int Initialize()
{
    RAISE_EXCEPTION_ADDRS pinAddrs;
    struct utsname uinfo;
    unsigned long addr;
    int trapNo;
    int i;

    /*
     * Create a page without read or write permission.
     */
    if (getpagesize() != PAGESIZE)
    {
        fprintf(stderr, "Wrong page size\n");
        return 0;
    }
    addr       = (unsigned long)&NoPermBuffer[0];
    addr       = (addr + PAGESIZE) & ~(PAGESIZE - 1);
    PageNoPerm = (void*)addr;
    if (mprotect(PageNoPerm, PAGESIZE, 0) != 0)
    {
        fprintf(stderr, "mprotect failed\n");
        return 0;
    }

    /*
     * Get a misaligned pointer to a 8-byte buffer.
     */
    addr              = (unsigned long)&MisalignedBuffer[0];
    MisalignedAddress = (void*)(((addr + 8) & ~7) + 1);

    /*
     * Create code for all possible 'INT x' instructions.  We construct the code
     * so that it falls at the end of a page and the next page is not accessible.
     * This helps test SMC.  See Mantis #1795.
     */
#if defined(TARGET_MAC)
    // Starting from macOS 10.15, statically allocated memory doesn't have execute permission and cannot be modified to have
    // execute permission. Therefore dynamically allocating this memory (which allows adding execute permission).
    addr = (unsigned long)malloc(3 * PAGESIZE);
#else
    addr = (unsigned long)&IntTrapBuffer[0];
#endif
    addr = (addr + PAGESIZE) & ~(PAGESIZE - 1);

    if (mprotect((void*)addr, PAGESIZE, (PROT_READ | PROT_WRITE | PROT_EXEC)) != 0)
    {
        fprintf(stderr, "mprotect failed\n");
        return 0;
    }
    if (mprotect((void*)(addr + PAGESIZE), PAGESIZE, 0) != 0)
    {
        fprintf(stderr, "mprotect failed\n");
        return 0;
    }
    IntTrapCode = (void*)(addr + PAGESIZE - BYTES_PER_TRAP_FUNC * NUM_TRAP_FUNCS);

    for (i = 0, trapNo = 0; trapNo < NUM_TRAP_FUNCS; trapNo++)
    {
        IntTrapCode[i++] = 0xcd; /* int <trapNo> */
        IntTrapCode[i++] = trapNo;
        IntTrapCode[i++] = 0xc3; /* ret */
    }
    assert(i == BYTES_PER_TRAP_FUNC * NUM_TRAP_FUNCS);

    /*
     * Tell the Pin tool about the labels for some of the instructions that raise
     * exceptions.  The tool may instrument these instructions.
     */
    memset(&pinAddrs, 0, sizeof(pinAddrs));
    pinAddrs._unmappedRead          = &PinLab_UnmappedRead;
    pinAddrs._unmappedReadAddr      = UnmappedAddress;
    pinAddrs._unmappedWrite         = &PinLab_UnmappedWrite;
    pinAddrs._unmappedWriteAddr     = UnmappedAddress;
    pinAddrs._inaccessibleRead      = &PinLab_InaccessibleRead;
    pinAddrs._inaccessibleReadAddr  = PageNoPerm;
    pinAddrs._inaccessibleWrite     = &PinLab_InaccessibleWrite;
    pinAddrs._inaccessibleWriteAddr = PageNoPerm;
    pinAddrs._misalignedRead        = &PinLab_MisalignedRead;
    pinAddrs._misalignedWrite       = &PinLab_MisalignedWrite;
    pinAddrs._illegalInstruction    = &PinLab_IllegalInstruction;
    pinAddrs._privilegedInstruction = &PinLab_PrivilegedInstruction;
    pinAddrs._integerDivideByZero   = &PinLab_IntegerDivideByZero;
    pinAddrs._integerOverflowTrap   = &PinLab_IntegerOverflowTrap;
#if defined(TARGET_IA32) && !defined(TARGET_MAC)
    pinAddrs._boundTrap = &PinLab_BoundTrap;
#endif
    pinAddrs._x87DivideByZero         = &PinLab_X87DivideByZero;
    pinAddrs._x87Overflow             = &PinLab_X87Overflow;
    pinAddrs._x87Underflow            = &PinLab_X87Underflow;
    pinAddrs._x87Precision            = &PinLab_X87Precision;
    pinAddrs._x87InvalidOperation     = &PinLab_X87InvalidOperation;
    pinAddrs._x87DenormalizedOperand  = &PinLab_X87DenormalizedOperand;
    pinAddrs._x87StackUnderflow       = &PinLab_X87StackUnderflow;
    pinAddrs._x87StackOverflow        = &PinLab_X87StackOverflow;
    pinAddrs._x87MultipleExceptions   = &PinLab_X87MultipleExceptions;
    pinAddrs._simdDivideByZero        = &PinLab_SIMDDivideByZero;
    pinAddrs._simdOverflow            = &PinLab_SIMDOverflow;
    pinAddrs._simdUnderflow           = &PinLab_SIMDUnderflow;
    pinAddrs._simdPrecision           = &PinLab_SIMDPrecision;
    pinAddrs._simdInvalidOperation    = &PinLab_SIMDInvalidOperation;
    pinAddrs._simdDenormalizedOperand = &PinLab_SIMDDenormalizedOperand;
    pinAddrs._simdMultipleExceptions  = &PinLab_SIMDMultipleExceptions;
    pinAddrs._breakpointTrap          = &PinLab_BreakpointTrap;
    SetLabelsForPinTool(&pinAddrs);

    /*
     * Some RHEL3 kernels are known to be broken in a way that causes this
     * test to fail.  Kernels "2.4.21-17" (Taroon Update 3) are known broken,
     * while kernels "2.4.21-37" (Taroon Update 6) are known to be OK.  If we
     * discover other broken kernel versions, the test below should be updated.
     *
     * Broken kernels only appear to be broken when running an IA32 application
     * on an Intel64 kernel.  In the broken case, the kernel appears to leave
     * the "si_code" field uninitialized in the siginfo_t structure.  This confuses
     * Pin (resulting in a Pin crash) for some of the tests below.  Other tests
     * fail simply because Pin emulates the correct si_code value.  However the test
     * fails because, we compare the Pin emulated si_code value against the native
     * si_code value.
     */
    IsBadKernel = 0;
#if defined(TARGET_IA32)
    if (uname(&uinfo) == 0)
    {
        if ((strncmp(uinfo.machine, "x86_64", sizeof("x86_64") - 1) == 0) &&
            (strncmp(uinfo.release, "2.4.21-17", sizeof("2.4.21-17") - 1) == 0))
        {
            IsBadKernel = 1;
        }
    }
#endif

    return 1;
}

TSTATUS DoTest(unsigned int tnum)
{
    unsigned int trapNo;
    void (*fn)();

    PrintSiAddr    = 0;
    PrintMxcsr     = 0;
    PrintX87Status = 0;

    switch (tnum)
    {
        case 0:
            printf("  Read from unmapped address (EXCEPTCODE_ACCESS_INVALID_ADDRESS)\n");
            PrintSiAddr = 1;
            DoUnmappedRead(UnmappedAddress);
            return TSTATUS_NOFAULT;
        case 1:
            printf("  Write to unmapped address (EXCEPTCODE_ACCESS_INVALID_ADDRESS)\n");
            PrintSiAddr = 1;
            DoUnmappedWrite(UnmappedAddress);
            return TSTATUS_NOFAULT;
        case 2:
            printf("  Jump to unmapped address (EXCEPTCODE_ACCESS_INVALID_ADDRESS)\n");
#if !defined(TARGET_BSD)
            if (!IsBadKernel)
            {
                PrintSiAddr = 1;
                fn          = UnmappedAddress;
                fn();
                return TSTATUS_NOFAULT;
            }
            else
            {
                return TSTATUS_SKIP;
            }
#else
            /* BSD does not support jumping to invalid code yet.  See Mantis #1940 */
            return TSTATUS_SKIP;
#endif
        case 3:
            printf("  Read from inaccessible address (EXCEPTCODE_ACCESS_DENIED)\n");
            PrintSiAddr = 1;
            DoInaccessibleRead(PageNoPerm);
            return TSTATUS_NOFAULT;
        case 4:
            printf("  Write to inaccessible address (EXCEPTCODE_ACCESS_DENIED)\n");
            PrintSiAddr = 1;
            DoInaccessibleWrite(PageNoPerm);
            return TSTATUS_NOFAULT;
        case 5:
            printf("  Jump to inaccessible address (EXCEPTCODE_ACCESS_DENIED)\n");
#if !defined(TARGET_BSD)
            if (!IsBadKernel)
            {
                PrintSiAddr = 1;
                fn          = PageNoPerm;
                fn();
                return TSTATUS_NOFAULT;
            }
            else
            {
                return TSTATUS_SKIP;
            }
#else
            /* BSD does not support jumping to invalid code yet.  See Mantis #1940 */
            return TSTATUS_SKIP;
#endif
        case 6:
            printf("  Read from misaligned address (EXCEPTCODE_ACCESS_MISALIGNED)\n");
#if !defined(TARGET_BSD) && !defined(TARGET_MAC)
            DoMisalignedRead(MisalignedAddress);
            return TSTATUS_NOFAULT;
#else
            /* This causes problems on BSD.  See Mantis #1940 */
            return TSTATUS_SKIP;
#endif
        case 7:
            printf("  Write to misaligned address (EXCEPTCODE_ACCESS_MISALIGNED)\n");
#if !defined(TARGET_BSD) && !defined(TARGET_MAC)
            DoMisalignedWrite(MisalignedAddress);
            return TSTATUS_NOFAULT;
#else
            /* This causes problems on BSD.  See Mantis #1940 */
            return TSTATUS_SKIP;
#endif
        case 8:
            printf("  Execute illegal instruction (EXCEPTCODE_ILLEGAL_INS)\n");
            PrintSiAddr = 1;
            DoUD2();
            return TSTATUS_NOFAULT;
        case 9:
            printf("  Execute privileged instruction (EXCEPTCODE_PRIVILEGED_INS)\n");
            DoPrivilegedInstruction();
            return TSTATUS_NOFAULT;
        case 10:
            printf("  Integer divide by zero (EXCEPTCODE_INT_DIVIDE_BY_ZERO)\n");
            PrintSiAddr = 1;
            DoIntegerDivideByZero();
            return TSTATUS_NOFAULT;
        case 11:
            printf("  Integer overflow trap (EXCEPTCODE_INT_OVERFLOW_TRAP)\n");
            DoIntegerOverflowTrap();
            return TSTATUS_NOFAULT;
        case 12:
            printf("  Array index bound trap (EXCEPTCODE_INT_BOUNDS_EXCEEDED)\n");
#if defined(TARGET_IA32) && !defined(TARGET_MAC)
            /*
         * The BOUND instruction only exists on IA32.
         * GCC/GAS on macOS* does not support assemblying the bound instruction
         */
            DoBoundTrap();
            return TSTATUS_NOFAULT;
#else
            return TSTATUS_SKIP;
#endif
        case 13:
            printf("  X87 divide by zero (EXCEPTCODE_X87_DIVIDE_BY_ZERO)\n");
            PrintSiAddr    = 1;
            PrintX87Status = 1;
            DoX87DivideByZero();
            return TSTATUS_NOFAULT;
        case 14:
            printf("  X87 overflow (EXCEPTCODE_X87_OVERFLOW)\n");
            PrintSiAddr    = 1;
            PrintX87Status = 1;
            DoX87Overflow();
            return TSTATUS_NOFAULT;
        case 15:
            printf("  X87 underflow (EXCEPTCODE_X87_UNDERFLOW)\n");
            PrintSiAddr    = 1;
            PrintX87Status = 1;
            DoX87Underflow();
            return TSTATUS_NOFAULT;
        case 16:
            printf("  X87 precision (EXCEPTCODE_X87_INEXACT_RESULT)\n");
            PrintSiAddr    = 1;
            PrintX87Status = 1;
            DoX87Precision();
            return TSTATUS_NOFAULT;
        case 17:
            printf("  X87 invalid operation (EXCEPTCODE_X87_INVALID_OPERATION)\n");
            PrintSiAddr    = 1;
            PrintX87Status = 1;
            DoX87InvalidOperation();
            return 1;
        case 18:
            printf("  X87 denormalized operand (EXCEPTCODE_X87_DENORMAL_OPERAND)\n");
            PrintSiAddr    = 1;
            PrintX87Status = 1;
            DoX87DenormalizedOperand();
            return TSTATUS_NOFAULT;
        case 19:
            printf("  X87 stack underflow (EXCEPTCODE_X87_STACK_ERROR)\n");
            PrintSiAddr    = 1;
            PrintX87Status = 1;
            DoX87StackUnderflow();
            return TSTATUS_NOFAULT;
        case 20:
            printf("  X87 stack overflow (EXCEPTCODE_X87_STACK_ERROR)\n");
            PrintSiAddr    = 1;
            PrintX87Status = 1;
            DoX87StackOverflow();
            return 1;
        case 21:
            printf("  X87 multiple exceptions\n");
            PrintSiAddr    = 1;
            PrintX87Status = 1;
            DoX87MultipleExceptions();
            return 1;
        case 22:
#if !defined(TARGET_MAC)
            printf("  SIMD divide by zero (EXCEPTCODE_SIMD_DIVIDE_BY_ZERO)\n");
            PrintSiAddr = 1;
            PrintMxcsr  = 1;
            DoSIMDDivideByZero();
            return TSTATUS_NOFAULT;
#else
            /* This causes problems on macOS* */
            return TSTATUS_SKIP;
#endif
        case 23:
            printf("  SIMD overflow (EXCEPTCODE_SIMD_OVERFLOW)\n");
            PrintSiAddr = 1;
            PrintMxcsr  = 1;
            DoSIMDOverflow();
            return TSTATUS_NOFAULT;
        case 24:
            printf("  SIMD underflow (EXCEPTCODE_SIMD_UNDERFLOW)\n");
            PrintSiAddr = 1;
            PrintMxcsr  = 1;
            DoSIMDUnderflow();
            return TSTATUS_NOFAULT;
        case 25:
            printf("  SIMD precision (EXCEPTCODE_SIMD_INEXACT_RESULT)\n");
            PrintSiAddr = 1;
            PrintMxcsr  = 1;
            DoSIMDPrecision();
            return TSTATUS_NOFAULT;
        case 26:
#if !defined(TARGET_MAC)
            printf("  SIMD invalid operation (EXCEPTCODE_SIMD_INVALID_OPERATION)\n");
            PrintSiAddr = 1;
            PrintMxcsr  = 1;
            DoSIMDInvalidOperation();
            return TSTATUS_NOFAULT;
#else
            /* This causes problems on macOS* */
            return TSTATUS_SKIP;
#endif
        case 27:
#if !defined(TARGET_MAC)
            printf("  SIMD denormalized operand (EXCEPTCODE_SIMD_DENORMAL_OPERAND)\n");
            PrintSiAddr = 1;
            PrintMxcsr  = 1;
            DoSIMDDenormalizedOperand();
            return TSTATUS_NOFAULT;
#else
            /* This causes problems on macOS* */
            return TSTATUS_SKIP;
#endif
        case 28:
            printf("  SIMD multiple exceptions\n");
            PrintSiAddr = 1;
            PrintMxcsr  = 1;
            DoSIMDMultipleExceptions();
            return TSTATUS_NOFAULT;
        case 29:
            printf("  breakpoint trap (EXCEPTCODE_DBG_BREAKPOINT_TRAP)\n");
            DoBreakpointTrap();
            return TSTATUS_NOFAULT;
        case 30:
            printf("  single-step trap (EXCEPTCODE_DBG_SINGLE_STEP_TRAP)\n");
#if 0
        /*
         * Pin doesn't handle this well now, so it is disabled.
         */
        PrintSiAddr = 1;
        DoSingleStepTrap();
        return TSTATUS_NOFAULT;
#else
            return TSTATUS_SKIP;
#endif
        case 31:
            printf("  bad register encoding\n");
#if !defined(TARGET_BSD)
            PrintSiAddr = 1;
            DoBadRegister();
            return TSTATUS_NOFAULT;
#else
            /* This causes problems on BSD.  See Mantis #1940 */
            return TSTATUS_SKIP;
#endif
        case 32:
            printf("  put illegal value into segment register gs/fs\n");
#if !defined(TARGET_BSD) && !defined(TARGET_MAC)
            PrintSiAddr = 1;
            DoIllegalSetOfSegReg1();
            return TSTATUS_NOFAULT;
#else
            /* This causes problems on BSD.  See Mantis #1940 */
            return TSTATUS_SKIP;
#endif
        case 33:
            printf("  illegal mem access in lfs/lgs \n");
#if !defined(TARGET_BSD)
            PrintSiAddr = 0;
            DoIllegalSetOfSegReg2();
            return TSTATUS_NOFAULT;
#else
            /* This causes problems on BSD.  See Mantis #1940 */
            return TSTATUS_SKIP;
#endif
        case 34:
            printf("  illegal mem access in RW to segment register\n");
#if !defined(TARGET_BSD)
            PrintSiAddr = 1;
            DoIllegalGetOfSegReg1();
            return TSTATUS_NOFAULT;
#else
            /* This causes problems on BSD.  See Mantis #1940 */
            return TSTATUS_SKIP;
#endif
        default:
            trapNo = tnum - 34;
            if (trapNo >= NUM_TRAP_FUNCS) break;

            printf("  INT %u (at %p)\n", trapNo, normalize_addr((intptr_t)&IntTrapCode[trapNo * BYTES_PER_TRAP_FUNC]));

#if defined(TARGET_MAC)
            /*
         * macOS* kernel delivers all sorts of signals to these traps
         */
            if (trapNo <= 5 || trapNo == 127) return TSTATUS_SKIP;

            /*
         * These are considered as system calls on macOS*
         */
            if (0x80 <= trapNo && trapNo <= 0x83) return TSTATUS_SKIP;
#else
            /*
         * Skip 'int 3' because some Linux kernels don't handle it well.  See Mantis #666.
         * Skip 'int 5' because some Linux kernels don't handle it well.  See Mantis #2678.
         * Skip 'int 0x80' because it is a system call trap on Linux and BSD.
         */
            if (trapNo == 3 || trapNo == 5 || trapNo == 0x80) return TSTATUS_SKIP;

#endif

            PrintSiAddr = 1;
            fn          = (void (*)(void)) & IntTrapCode[trapNo * BYTES_PER_TRAP_FUNC];
            fn();
            return TSTATUS_NOFAULT;
    }

    return TSTATUS_DONE;
}

void PrintSignalContext(int sig, const siginfo_t* info, void* vctxt)
{
    ucontext_t* ctxt = vctxt;
    unsigned long rip;
    long int trapno;
    long int mxcsr = 0;
    long int x87sw = 0;

    /*
     * Some of the tests set the AC bit.  Clear it to prevent alignment-check faults
     * while doing the print below.
     */
    ClearAC();

#if defined(TARGET_BSD)
    rip    = (unsigned long)ctxt->uc_mcontext.mc_rip;
    trapno = (long int)ctxt->uc_mcontext.mc_trapno;
#elif defined(TARGET_LINUX) && defined(TARGET_IA32E)
    rip    = (unsigned long)ctxt->uc_mcontext.gregs[REG_RIP];
    trapno = (long int)ctxt->uc_mcontext.gregs[REG_TRAPNO];
    if (ctxt->uc_mcontext.fpregs)
    {
        mxcsr = (long int)ctxt->uc_mcontext.fpregs->mxcsr;
        x87sw = (long int)ctxt->uc_mcontext.fpregs->swd;
    }
#elif defined(TARGET_LINUX) && defined(TARGET_IA32)
    rip    = (unsigned long)ctxt->uc_mcontext.gregs[REG_EIP];
    trapno = (long int)ctxt->uc_mcontext.gregs[REG_TRAPNO];
    if (ctxt->uc_mcontext.fpregs)
    {
        x87sw = (long int)ctxt->uc_mcontext.fpregs->sw;
    }
#elif defined(TARGET_MAC) && defined(TARGET_IA32E)
    rip    = (unsigned long)ctxt->uc_mcontext->__ss.__rip;
    trapno = (long int)ctxt->uc_mcontext->__es.__trapno;
    mxcsr  = (long int)ctxt->uc_mcontext->__fs.__fpu_mxcsr;
#elif defined(TARGET_MAC) && defined(TARGET_IA32)
    rip    = (unsigned long)ctxt->uc_mcontext->__ss.__eip;
    trapno = (long int)ctxt->uc_mcontext->__es.__trapno;
    mxcsr  = (long int)ctxt->uc_mcontext->__fs.__fpu_mxcsr;
#endif

    ;
    printf("  Signal %d, pc=0x%lx, si_errno=%d, trap_no=%ld", sig, normalize_addr((uintptr_t)rip), (int)info->si_errno, trapno);

    if (!IsBadKernel) printf(", si_code=%d", (int)info->si_code);
    if (PrintSiAddr) printf(", si_addr=%lx", normalize_addr((uintptr_t)info->si_addr));
    if (PrintMxcsr) printf(", mxcsr=0x%lx", mxcsr);
    if (PrintX87Status) printf(", x87sw=0x%lx", x87sw);
    printf("\n");
}
