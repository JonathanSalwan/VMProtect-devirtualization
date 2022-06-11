/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * This test verifies that Pin builds the correct FP values in the
 * signal context frame if an exception occurs when there is stuff
 * on the x87 FP stack.  Previously, Pin got the "tags" wrong.
 *
 * NOTE: This test may also fail when run natively on some older
 * Linux kernels.  These kernels also have a bug that causes the
 * "tags" to be wrong in the signal handler.  Pin gives the correct
 * value even for buggy kernels.
 */

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/ucontext.h>

extern void DoMath();
static void Handle(int, siginfo_t*, void*);

int main()
{
    struct sigaction act;
    act.sa_sigaction = Handle;
    act.sa_flags     = SA_SIGINFO;
    sigemptyset(&act.sa_mask);
    sigaction(SIGSEGV, &act, 0);
    sigaction(SIGBUS, &act, 0);

    /*
     * Do some FP math, then raise a SEGV.  The SEGV handler below verifies
     * that the saved FP state has the expected values.
     */
    DoMath();
    /* does not return */

    return 0;
}

#if !defined(TARGET_MAC)
static void Handle(int sig, siginfo_t* info, void* vctxt)
{
    ucontext_t* ctxt = vctxt;

    /*
     * There should be two values on the FP stack, at physical FP registers R6 and R7.
     * The value at R7 should be 0.0 (zero tag) and the value at R6 should 1.0 (valid tag).
     */
    if ((ctxt->uc_mcontext.fpregs->tag & 0xffff) != 0x4fff)
    {
        printf("FP tags wrong, expected 0x4fff, but got 0x%lx\n", (unsigned long)(ctxt->uc_mcontext.fpregs->tag & 0xffff));
        exit(1);
    }

    /*
     * The top of the FP stack should be "1.0".
     */
    if ((ctxt->uc_mcontext.fpregs->_st[0].significand[0] != 0) || (ctxt->uc_mcontext.fpregs->_st[0].significand[1] != 0) ||
        (ctxt->uc_mcontext.fpregs->_st[0].significand[2] != 0) || (ctxt->uc_mcontext.fpregs->_st[0].significand[3] != 0x8000) ||
        (ctxt->uc_mcontext.fpregs->_st[0].exponent != 0x3fff))
    {
        printf("ST[0] wrong, expected 1.0, but got 0x%x 0x%x 0x%x 0x%x E 0x%x\n",
               (unsigned)ctxt->uc_mcontext.fpregs->_st[0].significand[0],
               (unsigned)ctxt->uc_mcontext.fpregs->_st[0].significand[1],
               (unsigned)ctxt->uc_mcontext.fpregs->_st[0].significand[2],
               (unsigned)ctxt->uc_mcontext.fpregs->_st[0].significand[3], (unsigned)ctxt->uc_mcontext.fpregs->_st[0].exponent);
        exit(1);
    }

    /*
     * The other value on the FP stack should be "0.0".
     */
    if ((ctxt->uc_mcontext.fpregs->_st[1].significand[0] != 0) || (ctxt->uc_mcontext.fpregs->_st[1].significand[1] != 0) ||
        (ctxt->uc_mcontext.fpregs->_st[1].significand[2] != 0) || (ctxt->uc_mcontext.fpregs->_st[1].significand[3] != 0) ||
        (ctxt->uc_mcontext.fpregs->_st[1].exponent != 0))
    {
        printf("ST[1] wrong, expected 0.0, but got 0x%x 0x%x 0x%x 0x%x E 0x%x\n",
               (unsigned)ctxt->uc_mcontext.fpregs->_st[1].significand[0],
               (unsigned)ctxt->uc_mcontext.fpregs->_st[1].significand[1],
               (unsigned)ctxt->uc_mcontext.fpregs->_st[1].significand[2],
               (unsigned)ctxt->uc_mcontext.fpregs->_st[1].significand[3], (unsigned)ctxt->uc_mcontext.fpregs->_st[1].exponent);
        exit(1);
    }

    exit(0);
}

#else
static void Handle(int sig, siginfo_t* info, void* vctxt)
{
    ucontext_t* ctxt = vctxt;
    char one[10]     = {0, 0, 0, 0, 0, 0, 0, 0x80, 0xff, 0x3f};
    char zero[10]    = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    /*
     * There should be two values on the FP stack, at physical FP registers R6 and R7.
     * The value at R7 should be 0.0 (zero tag) and the value at R6 should 1.0 (valid tag).
     */
    if (ctxt->uc_mcontext->__fs.__fpu_ftw != 0xc0)
    {
        printf("FP tags wrong, expected 0xc0, but got 0x%lx\n", (unsigned long)(ctxt->uc_mcontext->__fs.__fpu_ftw));
        exit(1);
    }

    /*
     * The top of the FP stack should be "1.0".
     */
    if (memcmp(ctxt->uc_mcontext->__fs.__fpu_stmm0.__mmst_reg, one, sizeof(one)) != 0)
    {
        printf("ST[0] wrong, expected 1.0, but got %g\n", *(double*)ctxt->uc_mcontext->__fs.__fpu_stmm0.__mmst_reg);
        exit(1);
    }

    /*
     * The other value on the FP stack should be "0.0".
     */
    if (memcmp(ctxt->uc_mcontext->__fs.__fpu_stmm1.__mmst_reg, zero, sizeof(zero)) != 0)
    {
        printf("ST[1] wrong, expected 0.0, but got %g\n", *(double*)ctxt->uc_mcontext->__fs.__fpu_stmm1.__mmst_reg);
        exit(1);
    }

    exit(0);
}
#endif
