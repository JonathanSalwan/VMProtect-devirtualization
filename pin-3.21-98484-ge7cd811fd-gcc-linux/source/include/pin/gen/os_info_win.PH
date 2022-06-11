/*
 * Copyright (C) 2020-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#ifdef PIN_G_OS_INFO_WIN_PH
#error duplicate inclusion of os_info_win
#else
#define PIN_G_OS_INFO_WIN_PH
/*! @file
    Declares class representing Windows system-wide information in pin.
*/
/*! @ingroup WINDOWS_SYSCALL_API_UNDOC
 *  Value used as invalid system call number
 */
const SYSCALL_NUMBER_T SYSCALL_NUMBER_INVALID = ~0;

#endif // PIN_G_OS_INFO_WIN_PH
