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
#include <string.h>

ucontext_t* p_saved_ctxt;
unsigned altBaseSP;

void install_signal_handler(void);
void signal_handler(int, siginfo_t*, void*);
void generate_segv(int val);

void printSignalMask()
{
    sigset_t maskSet;
    int maskBits;
    sigprocmask(0, NULL, &maskSet);
    int i;
    for (i = 32; i > 0; i--)
        maskBits = (maskBits << 1) | sigismember(&maskSet, i);
    printf("signal mask: 0x%0.8x\n", maskBits);
}

void setup_signal_stack()
{
    int ret_val;
    stack_t ss;

    ss.ss_sp = malloc(SIGSTKSZ);
    assert(ss.ss_sp && "malloc failure");

    ss.ss_size  = SIGSTKSZ;
    ss.ss_flags = 0;

    //printf("ESP of alternate stack: 0x%x, top: 0x%x, size: %d\n", ss.ss_sp, ss.ss_sp + ss.ss_size, ss.ss_size);
    printf("NormalizedSP: 0, alternate stack size: 0x%x\n", ss.ss_size);
    altBaseSP = (int)ss.ss_sp;

    ret_val = sigaltstack(&ss, NULL);
    if (ret_val)
    {
        perror("ERROR, sigaltstack failed");
        exit(1);
    }
}

void install_signal_handler()
{
    int ret_val;
    struct sigaction s_sigaction;
    struct sigaction* p_sigaction = &s_sigaction;

    /* Register the signal hander using the siginfo interface*/
    p_sigaction->sa_sigaction = signal_handler;
    p_sigaction->sa_flags     = SA_SIGINFO | SA_ONSTACK;
    //p_sigaction->sa_flags = SA_SIGINFO;

    /* Don't mask any other signals */
    sigemptyset(&p_sigaction->sa_mask);

    ret_val = sigaction(SIGSEGV, p_sigaction, NULL);
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

void signal_handler(int signum, siginfo_t* siginfo, void* _uctxt)
{
    ucontext_t* uctxt = (ucontext_t*)_uctxt;
    ucontext_t signal_ctxt;

    printf("signal %d (captured EIP: 0x%x)\n", signum, uctxt->uc_mcontext.gregs[REG_EIP]);
    printSignalMask();

    int ret_val = getcontext(&signal_ctxt);
    if (ret_val)
    {
        perror("ERROR, getcontext failed");
        exit(1);
    }
    //printf("signal handler stack: 0x%0.8x\n", signal_ctxt.uc_mcontext.gregs[REG_ESP]);
    printf("signal handler normalized stack: 0x%0.8x\n", signal_ctxt.uc_mcontext.gregs[REG_ESP] - altBaseSP);

    printf("EDI: 0x%0.8x\n", uctxt->uc_mcontext.gregs[REG_EDI]);
    printf("ESI: 0x%0.8x\n", uctxt->uc_mcontext.gregs[REG_ESI]);
    printf("EBP: 0x%0.8x\n", uctxt->uc_mcontext.gregs[REG_EBP]);
    //printf("ESP: 0x%0.8x\n", uctxt->uc_mcontext.gregs[REG_ESP]);
    printf("EBX: 0x%0.8x\n", uctxt->uc_mcontext.gregs[REG_EBX]);
    printf("EDX: 0x%0.8x\n", uctxt->uc_mcontext.gregs[REG_EDX]);
    printf("ECX: 0x%0.8x\n", uctxt->uc_mcontext.gregs[REG_ECX]);
    printf("EAX: 0x%0.8x\n", uctxt->uc_mcontext.gregs[REG_EAX]);
    printf("EIP: 0x%0.8x\n", uctxt->uc_mcontext.gregs[REG_EIP]);
    printf("EFL: 0x%0.8x\n", uctxt->uc_mcontext.gregs[REG_EFL]);

    // Change contexts

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

    free(p_saved_ctxt);
    p_saved_ctxt = 0;
}

int main(int argc, char** argv)
{
    int ret_val;
    p_saved_ctxt = malloc(sizeof(ucontext_t));

    setup_signal_stack();
    install_signal_handler();

    ret_val = getcontext(p_saved_ctxt);
    if (ret_val)
    {
        perror("ERROR, getcontext failed");
        exit(1);
    }

    p_saved_ctxt->uc_mcontext.gregs[REG_EIP] = (int)&&safe_exit;

    generate_segv(1);

safe_exit:
    printf("safe exit\n");
    printSignalMask();

    return 0;
}
