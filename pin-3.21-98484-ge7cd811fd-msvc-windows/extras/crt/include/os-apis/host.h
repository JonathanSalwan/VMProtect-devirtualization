/*
 * Copyright (C) 2015-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// <COMPONENT>: os-apis
// <FILE-TYPE>: component public header
/// @file host.h

/*!
 * @defgroup OS_APIS_HOST Host machine queries
 * @brief Contains API for host related queries
 */

#ifndef OS_APIS_HOST_H
#define OS_APIS_HOST_H

/*! @ingroup OS_APIS_HOST
 * CPU Architecture
 */
typedef enum
{
    OS_HOST_CPU_ARCH_TYPE_INVALID = 0,
    OS_HOST_CPU_ARCH_TYPE_IA32    = 1,
    OS_HOST_CPU_ARCH_TYPE_INTEL64 = 2
} OS_HOST_CPU_ARCH_TYPE;

/*! @ingroup OS_APIS_HOST
 * Retrieves the host name of the current host - as known by this host
 *
 * @param[out] buf    Buffer to receive the host name
 * @param[in]  buflen Size of buffer @b buf in bytes
 *
 * @retval     OS_RETURN_CODE_NO_ERROR           If the operation succeeded
 * @retval     OS_RETURN_CODE_BUFFER_TOO_SHORT   If the buffer @b buf is too short
 * @retval     OS_RETURN_CODE_QUERY_FAILED       If the operation failed
 *
 * @par Availability:
 *   @b O/S:   Windows, Linux & macOS* \n
 *   @b CPU:   All \n
 */
OS_RETURN_CODE OS_GetHostName(CHAR* buf, USIZE buflen);

/*! @ingroup OS_APIS_HOST
 * Query CPU architecture.
 *
 * @param[out] arch   The architecture of this system's CPU
 *
 * @retval     OS_RETURN_CODE_NO_ERROR           If the operation succeeded
 * @retval     OS_RETURN_CODE_QUERY_FAILED       If the operation failed
 *
 * @par Availability:
 *   @b O/S:   Windows, Linux & macOS* \n
 *   @b CPU:   All \n
 */
OS_RETURN_CODE OS_GetHostCPUArch(OS_HOST_CPU_ARCH_TYPE* arch);

/*! @ingroup OS_APIS_HOST
 * Get a string representing a certain CPU arch.
 *
 * @param[in] arch    The architecture to get a string for it
 *
 * @retval     read-only string representing the CPU arch
 *
 * @par Availability:
 *   @b O/S:   Windows, Linux & macOS* \n
 *   @b CPU:   All \n
 */
const char* OS_CPUArchToString(OS_HOST_CPU_ARCH_TYPE arch);

/*! @ingroup OS_APIS_HOST
 * Get kernel release description string.
 *
 * @param[out] buf     Buffer to receive the release string
 * @param[in]  bufsize Size of buffer @b buf in bytes
 *
 * @retval     OS_RETURN_CODE_NO_ERROR           If the operation succeeded
 * @retval     OS_RETURN_CODE_QUERY_FAILED       If the operation failed
 *
 * @par Availability:
 *   @b O/S:   Windows, Linux & macOS* \n
 *   @b CPU:   All \n
 */
OS_RETURN_CODE OS_GetKernelRelease(CHAR* buf, USIZE bufsize);

/*! @ingroup OS_APIS_HOST
 * Get the name of the operating system we're running on.
 *
 * @param[out] buf     Buffer to receive the OS name string
 * @param[in]  bufsize Size of buffer @b buf in bytes
 *
 * @retval     OS_RETURN_CODE_NO_ERROR           If the operation succeeded
 * @retval     OS_RETURN_CODE_QUERY_FAILED       If the operation failed
 *
 * @par Availability:
 *   @b O/S:   Windows, Linux & macOS* \n
 *   @b CPU:   All \n
 */
OS_RETURN_CODE OS_GetOSName(CHAR* buf, USIZE bufsize);

/*! @ingroup OS_APIS_HOST
 * Get the version of the operating system we're running on.
 *
 * @param[out] buf     Buffer to receive the OS version string
 * @param[in]  bufsize Size of buffer @b buf in bytes
 *
 * @retval     OS_RETURN_CODE_NO_ERROR           If the operation succeeded
 * @retval     OS_RETURN_CODE_QUERY_FAILED       If the operation failed
 *
 * @par Availability:
 *   @b O/S:   Windows, Linux & macOS* \n
 *   @b CPU:   All \n
 */
OS_RETURN_CODE OS_GetOSVersion(CHAR* buf, USIZE bufsize);

/*! @ingroup OS_APIS_HOST
 * Get CPU processor frequency in MHz.
 *
 * @param[out] freq    Frequency of the CPU
 *
 * @retval     OS_RETURN_CODE_NO_ERROR           If the operation succeeded
 * @retval     OS_RETURN_CODE_QUERY_FAILED       If the operation failed
 *
 * @par Availability:
 *   @b O/S:   macOS* \n
 *   @b CPU:   All \n
 */
OS_RETURN_CODE OS_ReadProcessorFrequency(UINT32* freq);

#endif // OS_APIS_HOST_H
