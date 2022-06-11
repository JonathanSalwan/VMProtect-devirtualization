/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * This test verifies that Pin does not write beyond the end of the
 * sigset_t parameter to sigprocmask().  The Linux kernel really treats
 * these parameters as pointers to 64-bit words, which is smaller than
 * libc's sigset_t.  A noncompliant program might pass the address of a 64-bit
 * word as a parameter and it would still work OK.  This test verifies
 * that such noncompliant programs also work under Pin.
 *
 * FreeBSD note: the FreeBSD sigset size is 128 bit.
 */

#include <stdio.h>
#include <signal.h>
#include <stdint.h>

/*
 * This is an abbreviated signal set that matches the size really
 * accessed by the kernel.  It is SMALLER than libc's sigset_t.
 */
struct kernel_sigset
{
#ifdef TARGET_BSD
    uint32_t sig[4];
#else
    uint32_t sig[2];
#endif
};

int main()
{
    struct
    {
        kernel_sigset current;
        unsigned long dummy;
    } foo;

    /* This sentinal checks whether Pin overwrites beyond the end of 'current'. */
    foo.dummy = 0xdeadbeef;

    sigprocmask(SIG_SETMASK, 0, reinterpret_cast< sigset_t* >(&foo.current));

    if (foo.dummy != 0xdeadbeef)
    {
        printf("The stack dummy is CORRUPTED!\n");
        return 1;
    }
    return 0;
}
