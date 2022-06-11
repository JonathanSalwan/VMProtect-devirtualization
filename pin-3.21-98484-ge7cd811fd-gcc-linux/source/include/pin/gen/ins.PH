/*
 * Copyright (C) 2020-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#ifdef PIN_G_INS_PH
#error duplicate inclusion of ins
#else
#define PIN_G_INS_PH
/*! @file
   This file contains @ref INS primitives
 */
/*! @ingroup INS_INSPECTION
  system call number for function called from vsyscall
*/
const UINT32 VSYSCALL_NR = 0xABCDDCBA;

/*! @ingroup INS_INSPECTION
  Memory type of argument
*/
typedef enum
{
    MEMORY_TYPE_READ,
    MEMORY_TYPE_WRITE,
    MEMORY_TYPE_READ2
} MEMORY_TYPE;

/*! @ingroup INS_INSPECTION
 *  Calling standard of the system call instruction.
 */
typedef enum
{
    SYSCALL_STANDARD_INVALID,              ///< Invalid value; the instruction is not a system call
    SYSCALL_STANDARD_IA32_LINUX,           ///< Linux system call on IA-32 architecture
    SYSCALL_STANDARD_IA32_LINUX_SYSENTER,  ///< Linux system call via 'sysenter' on IA-32 architecture
    SYSCALL_STANDARD_IA32E_LINUX,          ///< Linux system call on Intel(R) 64 architecture
    SYSCALL_STANDARD_IA32E_LINUX_VSYSCALL, ///< Linux virtual system call on Intel(R) 64 architecture
    SYSCALL_STANDARD_IA32_MAC,             ///< macOS* system call in IA-32 architecture
    SYSCALL_STANDARD_IA32E_MAC,            ///< macOS* system call in Intel(R) 64 architecture
    SYSCALL_STANDARD_IA32_WINDOWS_FAST,    ///< "Fast" (SYSENTER) Windows system call in IA-32 architecture
    SYSCALL_STANDARD_IA32E_WINDOWS_FAST,   ///< "Fast" (SYSCALL) Windows system call in Intel(R) 64 architecture
    SYSCALL_STANDARD_IA32_WINDOWS_ALT,     ///< Alternative (INT2E) Windows system call in IA-32 architecture
    SYSCALL_STANDARD_WOW64,                ///< System call in WOW64 (32 bit process in 64-bit Windows)
    SYSCALL_STANDARD_WINDOWS_INT           ///< Software interruption (INT n) in Windows
} SYSCALL_STANDARD;

#endif // PIN_G_INS_PH
