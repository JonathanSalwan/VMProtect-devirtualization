/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * This test verifies that Pin correctly returns EFAULT when emulating a system call
 * that has an invalid pointer parameter.  For example, Pin must not crash when
 * reading through an invalid pointer that is passed in from the application.
 *
 * Note, this test must invoke the system calls directly with syscall() instead of
 * using the libc wrappers.  The libc wrappers don't check for invalid pointers, so
 * passing an invalid pointer can cause a crash in libc!
 */

#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <sys/syscall.h>

int main()
{
    int ret;

#ifdef SYS_rt_sigsuspend
    ret = syscall(SYS_rt_sigsuspend, -1, 8);
    if (ret != -1 || errno != EFAULT)
    {
        printf("SYS_rt_sigsuspend did not return EFAULT: ret=%d, errno=%d\n", ret, errno);
        return 1;
    }
#endif

#ifdef SYS_sigsuspend
    ret = syscall(SYS_sigsuspend, -1);
    if (ret != -1 || errno != EFAULT)
    {
        printf("SYS_sigsuspend did not return EFAULT: ret=%d, errno=%d\n", ret, errno);
        return 1;
    }
#endif

#ifdef SYS_rt_sigprocmask
    ret = syscall(SYS_rt_sigprocmask, SIG_SETMASK, -1, -1, 8);
    if (ret != -1 || errno != EFAULT)
    {
        printf("SYS_rt_sigprocmask did not return EFAULT: ret=%d, errno=%d\n", ret, errno);
        return 1;
    }
#endif

#ifdef SYS_sigprocmask
    ret = syscall(SYS_sigprocmask, SIG_SETMASK, -1, -1);
    if (ret != -1 || errno != EFAULT)
    {
        printf("SYS_sigprocmask did not return EFAULT: ret=%d, errno=%d\n", ret, errno);
        return 1;
    }
#endif

#ifdef SYS_rt_sigaction
    ret = syscall(SYS_rt_sigaction, SIGUSR1, -1, -1, 8);
    if (ret != -1 || errno != EFAULT)
    {
        printf("SYS_rt_sigaction did not return EFAULT: ret=%d, errno=%d\n", ret, errno);
        return 1;
    }
#endif

#ifdef SYS_sigaction
    ret = syscall(SYS_sigaction, SIGUSR1, -1, -1);
    if (ret != -1 || errno != EFAULT)
    {
        printf("SYS_sigaction did not return EFAULT: ret=%d, errno=%d\n", ret, errno);
        return 1;
    }
#endif

    return 0;
}
