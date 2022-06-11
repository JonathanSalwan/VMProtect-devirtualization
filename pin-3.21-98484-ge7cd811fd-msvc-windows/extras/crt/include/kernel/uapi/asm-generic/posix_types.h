/*
 * Copyright (C) 2015-2015 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/****************************************************************************
 ****************************************************************************
 ***
 ***   This header was automatically generated from a Linux kernel header
 ***   of the same name, to make information necessary for userspace to
 ***   call into the kernel available to libc.  It contains only constants,
 ***   structures, and macros generated from the original header, and thus,
 ***   contains no copyrightable information.
 ***
 ***   To edit the content of this header, modify the corresponding
 ***   source file (e.g. under external/kernel-headers/original/) then
 ***   run bionic/libc/kernel/tools/update_all.py
 ***
 ***   Any manual change here will be lost the next time this script will
 ***   be run. You've been warned!
 ***
 ****************************************************************************
 ****************************************************************************/
#ifndef __ASM_GENERIC_POSIX_TYPES_H
#define __ASM_GENERIC_POSIX_TYPES_H
#include <asm/bitsperlong.h>
#ifndef __kernel_long_t
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#if defined(TARGET_WINDOWS) && !defined(TARGET_IA32)
typedef long long __kernel_long_t;
typedef unsigned long long __kernel_ulong_t;
#else
typedef long __kernel_long_t;
typedef unsigned long __kernel_ulong_t;
#endif //defined(TARGET_WINDOWS) && !defined(TARGET_IA32)
#endif
#ifndef __kernel_ino_t
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
typedef __kernel_ulong_t __kernel_ino_t;
#endif
#ifndef __kernel_mode_t
#ifdef TARGET_MAC
typedef unsigned short __kernel_mode_t;
#else
typedef unsigned int __kernel_mode_t;
#endif
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#endif
#ifndef __kernel_pid_t
typedef int __kernel_pid_t;
#endif
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#ifndef __kernel_ipc_pid_t
typedef int __kernel_ipc_pid_t;
#endif
#ifndef __kernel_uid_t
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
typedef unsigned int __kernel_uid_t;
typedef unsigned int __kernel_gid_t;
#endif
#ifndef __kernel_suseconds_t
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
typedef __kernel_long_t __kernel_suseconds_t;
#endif
#ifndef __kernel_daddr_t
typedef int __kernel_daddr_t;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#endif
#ifndef __kernel_uid32_t
typedef unsigned int __kernel_uid32_t;
typedef unsigned int __kernel_gid32_t;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#endif
#ifndef __kernel_old_uid_t
typedef __kernel_uid_t __kernel_old_uid_t;
typedef __kernel_gid_t __kernel_old_gid_t;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#endif
#ifndef __kernel_old_dev_t
typedef unsigned int __kernel_old_dev_t;
#endif
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#ifndef __kernel_size_t
#if __BITS_PER_LONG != 64
typedef unsigned int __kernel_size_t;
typedef int __kernel_ssize_t;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
typedef int __kernel_ptrdiff_t;
#else
typedef __kernel_ulong_t __kernel_size_t;
typedef __kernel_long_t __kernel_ssize_t;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
typedef __kernel_long_t __kernel_ptrdiff_t;
#endif
#endif
#ifndef __kernel_fsid_t
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
typedef struct {
 int val[2];
} __kernel_fsid_t;
#endif
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
typedef __kernel_long_t __kernel_off_t;
typedef long long __kernel_loff_t;
typedef __kernel_long_t __kernel_time_t;
typedef __kernel_long_t __kernel_clock_t;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
typedef int __kernel_timer_t;
typedef int __kernel_clockid_t;
typedef char * __kernel_caddr_t;
typedef unsigned short __kernel_uid16_t;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
typedef unsigned short __kernel_gid16_t;
#endif
