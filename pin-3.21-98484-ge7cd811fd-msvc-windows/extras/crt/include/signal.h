/*
 * Copyright (C) 2015-2017 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * Copyright (C) 2008 The Android Open Source Project
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef _SIGNAL_H_
#define _SIGNAL_H_

#include <errno.h>
#include <sys/cdefs.h>
#include <limits.h>		/* For LONG_BIT */
#include <string.h>		/* For memset() */
#include <sys/types.h>

#if defined(TARGET_MAC)
# include <mac/signal.h>

#elif defined(__LP64__) || defined(__mips__)
#include <asm/sigcontext.h>
/* For 64-bit (and mips), the kernel's struct sigaction doesn't match the POSIX one,
 * so we need to expose our own and translate behind the scenes. */
#  define sigaction __kernel_sigaction
#  include <linux/signal.h>
#  undef sigaction
#else
#include <asm/sigcontext.h>
/* For 32-bit, we're stuck with the definitions we already shipped,
 * even though they contain a sigset_t that's too small. */
#  include <linux/signal.h>
#endif

#include <sys/ucontext.h>
#define __BIONIC_HAVE_UCONTEXT_T

__BEGIN_DECLS

typedef int sig_atomic_t;

/* The arm and x86 kernel header files don't define _NSIG. */
#ifndef _KERNEL__NSIG
# ifdef TARGET_MAC
#   define _KERNEL__NSIG 31
# else
#   define _KERNEL__NSIG 64
# endif
#endif

/* Userspace's NSIG is the kernel's _NSIG + 1. */
#define _NSIG (_KERNEL__NSIG + 1)

#ifndef TARGET_MAC
# define NSIG _NSIG
#endif

#ifdef TARGET_LINUX

/* We take a few real-time signals for ourselves. May as well use the same names as glibc. */
#define SIGRTMIN (__libc_current_sigrtmin())
#define SIGRTMAX (__libc_current_sigrtmax())
extern int __libc_current_sigrtmin(void);
extern int __libc_current_sigrtmax(void);

#endif


extern const char* const sys_siglist[];
extern const char* const sys_signame[]; /* BSD compatibility. */

#ifdef TARGET_MAC
typedef sig_t __sighandler_t;
#else
typedef __sighandler_t sig_t; /* BSD compatibility. */
#endif // TARGET_MAC

typedef __sighandler_t sighandler_t; /* glibc compatibility. */

#define si_timerid si_tid /* glibc compatibility. */

#ifdef TARGET_MAC

# ifndef sa_handler
#  define sa_handler _u._sa_handler
#  define sa_sigaction _u._sa_sigaction
# endif // sa_handler

#elif defined(__LP64__)

struct sigaction {
  union __sigaction_u {
    sighandler_t _sa_handler;
    void (*_sa_sigaction)(int, struct siginfo*, void*);
  } _u;
  sigset_t sa_mask;
  int sa_flags;
  void (*sa_restorer)(void);
};

# ifndef sa_handler
#  define sa_handler _u._sa_handler
#  define sa_sigaction _u._sa_sigaction
# endif

#endif // __LP64__

extern int sigaction(int, const struct sigaction*, struct sigaction*);

extern sighandler_t signal(int, sighandler_t);

extern int siginterrupt(int, int);

extern int sigaddset(sigset_t*, int);
extern int sigdelset(sigset_t*, int);
extern int sigemptyset(sigset_t*);
extern int sigfillset(sigset_t*);
extern int sigismember(const sigset_t*, int);

extern int sigpending(sigset_t*) __nonnull((1));
extern int sigprocmask(int, const sigset_t*, sigset_t*);
extern int sigsuspend(const sigset_t*) __nonnull((1));
extern int sigwait(const sigset_t*, int*) __nonnull((1, 2));

extern int raise(int);
extern int kill(pid_t, int);
extern int killpg(int, int);

extern int sigaltstack(const stack_t*, stack_t*);

#ifdef TARGET_LINUX
extern void psiginfo(const siginfo_t*, const char*);
extern void psignal(int, const char*);
extern int sigtimedwait(const sigset_t *, siginfo_t *, const struct timespec *) __nonnull ((1));
#endif

__END_DECLS

#endif /* _SIGNAL_H_ */
