/*
 * Copyright (C) 2006-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// features.h does not exist on FreeBSD
#ifndef TARGET_BSD
// features initializes the system's state, including the state of __USE_GNU
#include <features.h>
#endif

// If __USE_GNU is defined, we don't need to do anything.
// If we defined it ourselves, we need to undefine it later.
#ifndef __USE_GNU
#define __USE_GNU
#define APP_UNDEF_USE_GNU
#endif

#include <ucontext.h>

// If we defined __USE_GNU ourselves, we need to undefine it here.
#ifdef APP_UNDEF_USE_GNU
#undef __USE_GNU
#undef APP_UNDEF_USE_GNU
#endif

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <assert.h>
#include <setjmp.h>
#include <sys/types.h>
#include <linux/unistd.h>

ucontext_t* p_saved_ctxt;

void install_signal_handlers(void);
void segv_signal_handler(int, siginfo_t*, void*);
void usr2_signal_handler(int, siginfo_t*, void*);
void generate_segv(int val);
void print_fp_regs(struct _libc_fpstate*);
void print_fp_reg(struct _libc_fpreg*);

void install_signal_handlers()
{
    int ret_val;
    struct sigaction s_sigaction;
    struct sigaction* p_sigaction = &s_sigaction;

    /* Register the signal hander using the siginfo interface*/
    p_sigaction->sa_sigaction = segv_signal_handler;
    p_sigaction->sa_flags     = SA_SIGINFO;

    /* mask all other signals */
    sigfillset(&p_sigaction->sa_mask);

    ret_val = sigaction(SIGSEGV, p_sigaction, NULL);
    if (ret_val)
    {
        perror("ERROR, sigaction failed");
        exit(1);
    }

    p_sigaction->sa_sigaction = usr2_signal_handler;
    p_sigaction->sa_flags     = SA_SIGINFO;

    ret_val = sigaction(SIGUSR2, p_sigaction, NULL);
    if (ret_val)
    {
        perror("ERROR, sigaction failed");
        exit(1);
    }
}

void generate_segv(int val)
{
    int* p = 0;

    printf("EIP of segfault: 0x%x\n", &&segfault);

    __asm__ __volatile__("movl $0x600D1, %eax;\
            movl $0x600D2, %ebx;\
            movl $0x600D3, %ecx;\
            movl $0x600D4, %edx;\
            movl $0x600D5, %edi;\
            movl $0x600D6, %esi;\
            movl $0x600D7, %ebp");

segfault:
    __asm__ __volatile__("movl (0x0), %ecx");
}

void segv_signal_handler(int signum, siginfo_t* siginfo, void* _uctxt)
{
    int ret_val;
    ucontext_t* uctxt = (ucontext_t*)_uctxt;
    ucontext_t signal_ctxt;
    pid_t tid;

    printf("signal %d (captured EIP: 0x%x)\n", signum, uctxt->uc_mcontext.gregs[REG_EIP]);

    assert(uctxt->uc_mcontext.fpregs);
    printf("FCW: 0x%hx\n", uctxt->uc_mcontext.fpregs->cw);
    print_fp_regs(uctxt->uc_mcontext.fpregs);

    tid     = getpid();
    ret_val = kill(tid, SIGUSR2);
}

void print_fp_regs(struct _libc_fpstate* fpstate)
{
    int i;
    for (i = 0; i < 8; i++)
    {
        printf("reg %d: ", i);
        print_fp_reg(&fpstate->_st[i]);
        printf("\n");
    }
}

void print_fp_reg(struct _libc_fpreg* p_reg)
{
    printf("exponent = 0x%hx significand = 0x%hx%hx%hx%hx", p_reg->exponent, p_reg->significand[3], p_reg->significand[2],
           p_reg->significand[1], p_reg->significand[0]);
}

void usr2_signal_handler(int signum, siginfo_t* siginfo, void* _uctxt)
{
    int ret_val;
    ucontext_t* uctxt = (ucontext_t*)_uctxt;
    ucontext_t signal_ctxt;
    pid_t tid;

    printf("signal %d (captured EIP: 0x%x)\n", signum, uctxt->uc_mcontext.gregs[REG_EIP]);

    // Change contexts

    assert(uctxt->uc_mcontext.fpregs);
    printf("FCW: 0x%hx\n", uctxt->uc_mcontext.fpregs->cw);
    print_fp_regs(uctxt->uc_mcontext.fpregs);

    uctxt->uc_mcontext.gregs[REG_EDI] = p_saved_ctxt->uc_mcontext.gregs[REG_EDI];
    uctxt->uc_mcontext.gregs[REG_ESI] = p_saved_ctxt->uc_mcontext.gregs[REG_ESI];
    uctxt->uc_mcontext.gregs[REG_EBP] = p_saved_ctxt->uc_mcontext.gregs[REG_EBP];
    uctxt->uc_mcontext.gregs[REG_ESP] = p_saved_ctxt->uc_mcontext.gregs[REG_ESP];
    uctxt->uc_mcontext.gregs[REG_EBX] = p_saved_ctxt->uc_mcontext.gregs[REG_EBX];
    uctxt->uc_mcontext.gregs[REG_EDX] = p_saved_ctxt->uc_mcontext.gregs[REG_EDX];
    uctxt->uc_mcontext.gregs[REG_ECX] = p_saved_ctxt->uc_mcontext.gregs[REG_ECX];
    uctxt->uc_mcontext.gregs[REG_EAX] = p_saved_ctxt->uc_mcontext.gregs[REG_EAX];
    uctxt->uc_mcontext.gregs[REG_EIP] = p_saved_ctxt->uc_mcontext.gregs[REG_EIP];
    uctxt->uc_mcontext.gregs[REG_EFL] = p_saved_ctxt->uc_mcontext.gregs[REG_EFL];

    // getcontext() doesn't appear to properly set the segment registers,
    // so don't mess with them.
    //uctxt->uc_mcontext.gregs[REG_CS] = p_saved_ctxt->uc_mcontext.gregs[REG_CS];
    //uctxt->uc_mcontext.gregs[REG_SS] = p_saved_ctxt->uc_mcontext.gregs[REG_SS];
    //uctxt->uc_mcontext.gregs[REG_DS] = p_saved_ctxt->uc_mcontext.gregs[REG_DS];
    //uctxt->uc_mcontext.gregs[REG_ES] = p_saved_ctxt->uc_mcontext.gregs[REG_ES];
    //uctxt->uc_mcontext.gregs[REG_FS] = p_saved_ctxt->uc_mcontext.gregs[REG_FS];
    //uctxt->uc_mcontext.gregs[REG_GS] = p_saved_ctxt->uc_mcontext.gregs[REG_GS];
}

int main(int argc, char** argv)
{
    int ret_val;
    p_saved_ctxt = malloc(sizeof(ucontext_t));

    install_signal_handlers();

    volatile double test_d = 3.14;
    assert(test_d == 3.14);

    ret_val = getcontext(p_saved_ctxt);
    if (ret_val)
    {
        perror("ERROR, getcontext failed");
        exit(1);
    }

    print_fp_regs(p_saved_ctxt->uc_mcontext.fpregs);
    p_saved_ctxt->uc_mcontext.gregs[REG_EIP] = (int)&&safe_exit;

    assert(p_saved_ctxt->uc_mcontext.fpregs);
    printf("FCW: 0x%hx\n", p_saved_ctxt->uc_mcontext.fpregs->cw);

    generate_segv(1);

safe_exit:
    printf("safe exit\n");

    ret_val = getcontext(p_saved_ctxt);
    if (ret_val)
    {
        perror("ERROR, getcontext failed");
        exit(1);
    }

    assert(p_saved_ctxt->uc_mcontext.fpregs);
    printf("FCW: 0x%hx\n", p_saved_ctxt->uc_mcontext.fpregs->cw);

    free(p_saved_ctxt);
    p_saved_ctxt = 0;
    return 0;
}
