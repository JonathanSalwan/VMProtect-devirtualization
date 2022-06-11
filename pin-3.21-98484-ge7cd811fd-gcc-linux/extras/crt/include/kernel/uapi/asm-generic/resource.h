/*
 * Copyright (C) 2016-2016 Intel Corporation.
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
#ifndef _UAPI_ASM_GENERIC_RESOURCE_H
#define _UAPI_ASM_GENERIC_RESOURCE_H
#define RLIMIT_CPU 0
#define RLIMIT_FSIZE 1
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define RLIMIT_DATA 2
#define RLIMIT_STACK 3
#define RLIMIT_CORE 4
#ifndef RLIMIT_RSS
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define RLIMIT_RSS 5
#endif

#ifdef TARGET_MAC
#ifndef RLIMIT_NPROC
#define RLIMIT_NPROC 7
#endif
#ifndef RLIMIT_NOFILE
#define RLIMIT_NOFILE 8
#endif
#ifndef RLIMIT_MEMLOCK
#define RLIMIT_MEMLOCK 9
#endif
#ifndef RLIMIT_AS
#define RLIMIT_AS 5
#endif
#define RLIM_NLIMITS 9
#ifndef RLIM_INFINITY
#define RLIM_INFINITY   (((__uint64_t)1 << 63) - 1)     /* no limit */
#define RLIM_SAVED_MAX  RLIM_INFINITY   /* Unrepresentable hard limit */
#define RLIM_SAVED_CUR  RLIM_INFINITY   /* Unrepresentable soft limit */
#endif
#else
#ifndef RLIMIT_NPROC
#define RLIMIT_NPROC 6
#endif
#ifndef RLIMIT_NOFILE
#define RLIMIT_NOFILE 7
#endif
#ifndef RLIMIT_MEMLOCK
#define RLIMIT_MEMLOCK 8
#endif
#ifndef RLIMIT_AS
#define RLIMIT_AS 9
#endif
#define RLIM_NLIMITS 16
#ifndef RLIM_INFINITY
#define RLIM_INFINITY (~0UL)
#endif
#endif

#define RLIMIT_LOCKS 10
#define RLIMIT_SIGPENDING 11
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define RLIMIT_MSGQUEUE 12
#define RLIMIT_NICE 13
#define RLIMIT_RTPRIO 14
#define RLIMIT_RTTIME 15
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#ifndef _STK_LIM_MAX
#define _STK_LIM_MAX RLIM_INFINITY
#endif
#endif
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
