/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * This test verifies that Pin knows the correct size of the signal
 * mask passed to the old-style signal system calls.  These system calls
 * take a 4-byte signal mask, which is shorter than the mask accepted
 * by the modern system calls.
 *
 * Note, we must use syscall() here because libc does not use these
 * old-style system calls any more.
 */

#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/syscall.h>

int main()
{
    char mask1[8];
    char mask2[8];

    memset(mask1, 0, sizeof(mask1));
    memset(mask2, 0, sizeof(mask2));
#ifdef SYS_sigprocmask
    syscall(SYS_sigprocmask, SIG_SETMASK, 0, mask1);
#endif
    if (memcmp(mask1 + 4, mask2 + 4, sizeof(mask1) - 4) != 0)
    {
        printf("SYS_sigprocmask wrote beyond end of signal mask\n");
        return 1;
    }

    memset(mask1, 0xff, sizeof(mask1));
    memset(mask2, 0xff, sizeof(mask2));
#ifdef SYS_sigprocmask
    syscall(SYS_sigprocmask, SIG_SETMASK, 0, mask1);
#endif
    if (memcmp(mask1 + 4, mask2 + 4, sizeof(mask1) - 4) != 0)
    {
        printf("SYS_sigprocmask wrote beyond end of signal mask\n");
        return 1;
    }

    return 0;
}
