/*
 * Copyright (C) 2017-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// <COMPONENT>: os-apis
// <FILE-TYPE>: component public header

/*
 * These defines are replacers for enum, Since this file is included in an assembly file
 * docall-ia32-mac-asm.spp we cannot use anything but defines
 */
#define OS_SYSCALL_TYPE_LINUX 0
#define OS_SYSCALL_TYPE_WIN 1
#define OS_SYSCALL_TYPE_SYSENTER 2
#define OS_SYSCALL_TYPE_WOW64 3
#define OS_SYSCALL_TYPE_INT80 4
#define OS_SYSCALL_TYPE_INT81 5
#define OS_SYSCALL_TYPE_INT82 6
#define OS_SYSCALL_TYPE_INT83 7
#define OS_SYSCALL_TYPE_UNIX OS_SYSCALL_TYPE_SYSENTER
