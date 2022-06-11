/*
 * Copyright (C) 2016-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// <COMPONENT>: os-apis
// <FILE-TYPE>: component public header

/*!
 * @defgroup OS_APIS_PROCESS Process
 * @brief Contains process-related os apis
 */

#ifndef OS_APIS_PROCESS_CORE_H
#define OS_APIS_PROCESS_CORE_H

/*! @ingroup OS_APIS_PROCESS
 * Retrieves the process ID of the current process.
 *
 * @param[out] pid          Process descriptor
 *
 * @return      Operation status code.
 * @retval      OS_RETURN_CODE_NO_ERROR             If the operation succeeded
 * @retval      OS_RETURN_CODE_PROCESS_QUERY_FAILED If the operation Failed
 *
 * @par Availability:
 *   @b O/S:   Windows, Linux & macOS*\n
 *   @b CPU:   All\n
 */
OS_RETURN_CODE OS_GetPid(NATIVE_PID* pid);

#endif // file guard
