/*
 * Copyright (C) 2006-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// features.h does not exist on FreeBSD and macOS*
#ifdef TARGET_LINUX
// features initializes the system's state, including the state of __USE_GNU
#include <features.h>
#endif

// If __USE_GNU is defined, we don't need to do anything.
// If we defined it ourselves, we need to undefine it later.
#ifndef __USE_GNU
#define __USE_GNU
#define APP_UNDEF_USE_GNU
#endif

#ifdef TARGET_LINUX
#include <ucontext.h>
#else
#include <sys/ucontext.h>
#endif

// If we defined __USE_GNU ourselves, we need to undefine it here.
#ifdef APP_UNDEF_USE_GNU
#undef __USE_GNU
#undef APP_UNDEF_USE_GNU
#endif

#include <stdlib.h>
#include <signal.h>
#include <assert.h>
#include <setjmp.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_attr_t thread_attr;
int p_safe_exit;

void setup_signal_stack(void);
void print_signal_stack(void);
void install_signal_handler(void);
void signal_handler(int, siginfo_t*, void*);
void generate_segv(int val);
void lock();
void unlock();
void* thread_gen_segv(void*);

void printSignalMask()
{
    sigset_t maskSet;
    int maskBits;
#ifdef TARGET_MAC
    pthread_sigmask(0, NULL, &maskSet);
#else
    sigprocmask(0, NULL, &maskSet);
#endif
    int i;
    for (i = 32; i > 0; i--)
        maskBits = (maskBits << 1) | sigismember(&maskSet, i);
    printf("signal mask: 0x%08x\n", maskBits);
}

void setup_signal_stack()
{
    int ret_val;
    stack_t ss;
    stack_t oss;

    ss.ss_sp = malloc(SIGSTKSZ);
    assert(ss.ss_sp && "malloc failure");

    ss.ss_size  = SIGSTKSZ;
    ss.ss_flags = 0;

    printf("ESP of alternate stack: %p, top: %p, size: %ld\n", ss.ss_sp, (unsigned char*)ss.ss_sp + ss.ss_size, ss.ss_size);

    ret_val = sigaltstack(&ss, &oss);
    if (ret_val)
    {
        perror("ERROR, failed to set sigaltstack");
        exit(1);
    }

    printf("ESP of original stack: %p, top: %p, size: %ld, disabled = %s\n", oss.ss_sp, (unsigned char*)oss.ss_sp + oss.ss_size,
           oss.ss_size, (oss.ss_flags & SS_DISABLE) ? "true" : "false");
}

void print_signal_stack()
{
    int ret_val;
    stack_t oss;

    ret_val = sigaltstack(NULL, &oss);
    if (ret_val)
    {
        perror("ERROR, failed to read sigaltstack");
        exit(1);
    }

    printf("ESP of original stack: %p, top: %p, size: %ld, disabled = %s\n", oss.ss_sp, (unsigned char*)oss.ss_sp + oss.ss_size,
           oss.ss_size, (oss.ss_flags & SS_DISABLE) ? "true" : "false");
}

void install_signal_handler()
{
    int ret_val;
    struct sigaction s_sigaction;
    struct sigaction* p_sigaction = &s_sigaction;

    /* Register the signal hander using the siginfo interface*/
    p_sigaction->sa_sigaction = signal_handler;
    p_sigaction->sa_flags     = SA_SIGINFO | SA_ONSTACK;

    /* Don't mask any other signals */
    sigemptyset(&p_sigaction->sa_mask);

    ret_val = sigaction(SIGSEGV, p_sigaction, NULL);
    if (ret_val)
    {
        perror("ERROR, sigaction failed");
        exit(1);
    }
    ret_val = sigaction(SIGBUS, p_sigaction, NULL);
    if (ret_val)
    {
        perror("ERROR, sigaction failed");
        exit(1);
    }
}

void generate_segv(int val)
{
    int* p = 0;

    p_safe_exit = (int)&&safe_exit;

    printf("EIP of segfault: %p (only accurate with if compiled with -O)\n", &&segfault);

    /* Encourage the compiler to put val into a register so that the
     statement after the 'segfault' label is more likely to be the
     exact location of a mov to 0x0 */
    val++;

segfault:
    *p = val;

    printf("ERROR!\n");

safe_exit:
    printf("EIP of safe exit: 0x%x\n", p_safe_exit);
}

void lock()
{
    int ret_val;

    ret_val = pthread_mutex_lock(&mutex);
    if (ret_val)
    {
        perror("ERROR, pthread_mutex_lock failed");
    }

    fflush(stdout);
}

void unlock()
{
    int ret_val;

    fflush(stdout);

    ret_val = pthread_mutex_unlock(&mutex);
    if (ret_val)
    {
        perror("ERROR, pthread_mutex_unlock failed");
    }
}

void signal_handler(int signum, siginfo_t* siginfo, void* _uctxt)
{
    ucontext_t* uctxt = (ucontext_t*)_uctxt;
    ucontext_t signal_ctxt;

#if defined(TARGET_MAC)
    printf("signal %d (captured EIP: 0x%x)\n", signum, uctxt->uc_mcontext->__ss.__eip);
#else
    printf("signal %d (captured EIP: 0x%x)\n", signum, uctxt->uc_mcontext.gregs[REG_EIP]);
#endif
    printSignalMask();

#if defined(TARGET_MAC)
    /* On macOS* ucontext routines are deprecated */
    uctxt->uc_mcontext->__ss.__eip = p_safe_exit;
#else
    int ret_val = getcontext(&signal_ctxt);
    if (ret_val)
    {
        perror("ERROR, getcontext failed");
        exit(1);
    }
    printf("signal handler stack: 0x%0.8x\n", signal_ctxt.uc_mcontext.gregs[REG_ESP]);

    uctxt->uc_mcontext.gregs[REG_EIP] = p_safe_exit;
#endif
}

void* thread_start(void* arg)
{
    int ret_val;

    lock();

    printf("thread arg = %d\n", (unsigned int)arg);

    if (arg)
    {
        setup_signal_stack();
    }
    else
    {
        print_signal_stack();
    }

    generate_segv(1);

    unlock();

    if (arg == (void*)2)
    {
        pthread_t tid;
        void* thread_ret;

        ret_val = pthread_create(&tid, NULL, thread_start, NULL);
        if (ret_val)
        {
            perror("ERROR, pthread_create failed");
            exit(1);
        }

        lock();
        printf("created thread 0x%lx\n", (unsigned long)tid);
        unlock();

        ret_val = pthread_join(tid, &thread_ret);
        if (ret_val)
        {
            perror("ERROR, pthread_join failed");
            exit(1);
        }

        ret_val = pthread_create(&tid, NULL, thread_start, (void*)1);
        if (ret_val)
        {
            perror("ERROR, pthread_create failed");
            exit(1);
        }

        lock();
        printf("created thread 0x%lx\n", (unsigned long)tid);
        unlock();

        ret_val = pthread_join(tid, &thread_ret);
        if (ret_val)
        {
            perror("ERROR, pthread_join failed");
            exit(1);
        }
    }
    return 0;
}

int main(int argc, char** argv)
{
    pthread_t tid;
    int ret_val;
    void* thread_ret;

    setup_signal_stack();
    install_signal_handler();

    ret_val = pthread_create(&tid, NULL, thread_start, NULL);
    if (ret_val)
    {
        perror("ERROR, pthread_create failed");
        exit(1);
    }

    lock();
    printf("created thread 0x%lx\n", (unsigned long)tid);
    unlock();

    ret_val = pthread_join(tid, &thread_ret);
    if (ret_val)
    {
        perror("ERROR, pthread_join failed");
        exit(1);
    }

    ret_val = pthread_create(&tid, NULL, thread_start, (void*)1);
    if (ret_val)
    {
        perror("ERROR, pthread_create failed");
        exit(1);
    }

    lock();
    printf("created thread 0x%lx\n", (unsigned long)tid);
    unlock();

    ret_val = pthread_join(tid, &thread_ret);
    if (ret_val)
    {
        perror("ERROR, pthread_join failed");
        exit(1);
    }

    ret_val = pthread_create(&tid, NULL, thread_start, (void*)2);
    if (ret_val)
    {
        perror("ERROR, pthread_create failed");
        exit(1);
    }

    lock();
    printf("created thread 0x%lx\n", (unsigned long)tid);
    unlock();

    ret_val = pthread_join(tid, &thread_ret);
    if (ret_val)
    {
        perror("ERROR, pthread_join failed");
        exit(1);
    }
}
