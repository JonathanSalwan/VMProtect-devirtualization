/*
 * Copyright (C) 2020-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#ifdef PIN_G_OS_INFO_CLIENT_WINDOWS_PH
#error duplicate inclusion of os_info_client_windows
#else
#define PIN_G_OS_INFO_CLIENT_WINDOWS_PH
/*! @file
 *
 * Client side interface to pin vm for specifically Windows only functions.
 * At present none of these is documented, they are provided for the use of PinPlay.
 */
/*! @ingroup WINDOWS_SYSCALL_API_UNDOC
 * Get the system call number which corresponds to the equivalent system call key.
 * This mapping depends on the precise version of the operating system on which
 * Pin is running.
 *
 * @param[in]       key     system call key
 * @return          system call number
 *
 * @par Availability:
 * \b Mode:  JIT\n
 * \b O/S:   Windows\n
 * \b CPU:   IA-32 and Intel(R) 64 architectures\n
 */
extern UINT32 PIN_GetWindowsSyscallFromKey(SYSCALL_KEY key);

/*! @ingroup WINDOWS_SYSCALL_API_UNDOC
 * Get the system call key which corresponds to the equivalent system call number.
 * Pin only knows about a limited set of system calls, if the system call number is
 * unknown, this function will return SYSCALL_KEY_UNKNOWN.
 *
 * @param[in]       systcallNo     system call number
 * @return          system call key, or SYSCALL_KEY_UNKNOWN
 *
 * @par Availability:
 * \b Mode:  JIT\n
 * \b O/S:   Windows\n
 * \b CPU:   IA-32 and Intel(R) 64 architectures\n
 */
extern SYSCALL_KEY PIN_GetKeyFromWindowsSyscall(UINT32 syscallNo);

#endif // PIN_G_OS_INFO_CLIENT_WINDOWS_PH
