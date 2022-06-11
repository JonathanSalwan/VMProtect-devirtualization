/*
 * Copyright (C) 2020-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#ifdef PIN_G_INS_SYSCALL_IA32_WINDOWS_PH
#error duplicate inclusion of ins_syscall_ia32_windows
#else
#define PIN_G_INS_SYSCALL_IA32_WINDOWS_PH
/*! @file
 * Syscall information for Windows/x86 systems
 */
/*! @ingroup INS_INSPECTION
 * Check to see if the given instruction is a system call and return corresponding 
 * calling standard.
 * @return calling standard of the system call instruction or SYSCALL_STANDARD_INVALID
 *         if the instruction is not a system call.
 */
extern SYSCALL_STANDARD INS_SyscallStd(INS ins);

/*! @ingroup INS_INSPECTION
 * @return true if the instruction is a system call 
 */
inline BOOL INS_IsSyscall(INS ins) { return (INS_SyscallStd(ins) != SYSCALL_STANDARD_INVALID); }

#endif // PIN_G_INS_SYSCALL_IA32_WINDOWS_PH
