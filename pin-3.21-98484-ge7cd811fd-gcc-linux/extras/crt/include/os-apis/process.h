/*
 * Copyright (C) 2015-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// <COMPONENT>: os-apis
// <FILE-TYPE>: component public header

/*!
 * @defgroup OS_APIS_PROCESS Process
 * @brief Contains process-related os apis
 */

#ifndef OS_APIS_PROCESS_H
#define OS_APIS_PROCESS_H

#include "process-core.h"

/*! @ingroup OS_APIS_FILE
 * Opaque object that represents a process that we can wait for its termination
 */
typedef UINT64 OS_PROCESS_WAITABLE_PROCESS;

/*! @ingroup OS_APIS_PROCESS
 * Retrieve the command line for a given process.
 *
 * @param[in]  pid          Process descriptor
 * @param[out] argc         Command line's argc
 * @param[out] argv         Command line's argv array, allocated by this function (with OS_AllocateMemory())
 * @param[out] bufsize      Size in bytes of the buffer pointer by argv
 *
 * @return      Operation status code.
 * @retval      OS_RETURN_CODE_NO_ERROR             If the operation succeeded
 * @retval      OS_RETURN_CODE_PROCESS_QUERY_FAILED If the operation Failed
 *
 * @par Availability:
 *   @b O/S:   Windows, Linux & macOS*\n
 *   @b CPU:   All\n
 */
OS_RETURN_CODE OS_GetCommandLine(NATIVE_PID pid, USIZE* argc, CHAR*** argv, USIZE* bufsize);

/*! @ingroup OS_APIS_PROCESS
 * Retrieve pointer to process environment variables block encoded in UTF8.
 *
 * @param[in]  pid          Process descriptor.
 * @param[out] block        Return pointer to point to the environment block.
 * @param[out] bufSize      Number of bytes allocated for the envrionement block.
 *
 * @return      Operation status code.
 * @retval      OS_RETURN_CODE_NO_ERROR                 If the operation succeeded
 * @retval      OS_RETURN_CODE_PROCESS_UPDATE_FAILED    If the operation Failed
 *
 * @note On Linux and macOS* this function will retrieve the initial environment
 *       block that was valid when the process started.
 *       This function doesn't catch up with changes done to the environment variables
 *       while the process is running.
 * @par Availability:
 *   @b O/S:   Windows, Linux & macOS*\n
 *   @b CPU:   All\n
 */
OS_RETURN_CODE OS_GetEnvironmentBlock(NATIVE_PID pid, CHAR*** block, USIZE* bufSize);

/*! @ingroup OS_APIS_PROCESS
 * Exits the current process.
 *
 * @param[in]  code         Process descriptor
 *
 * @par Availability:
 *   @b O/S:   Windows, Linux & macOS*\n
 *   @b CPU:   All\n
 */
NORETURN void OS_ExitProcess(INT code);

/*! @ingroup OS_APIS_PROCESS
 * Queries whether the current process is being debugged.
 *
 * @param[out] pid          Process descriptor
 * @param[out] isPresent    True if the process 'pid' is being debugged.
 *
 * @return      Operation status code.
 * @retval      OS_RETURN_CODE_NO_ERROR             If the operation succeeded
 * @retval      OS_RETURN_CODE_PROCESS_QUERY_FAILED If the operation Failed
 *
 * @par Availability:
 *   @b O/S:   Windows, Linux & macOS*\n
 *   @b CPU:   All\n
 */
OS_RETURN_CODE OS_IsDebuggerPresent(NATIVE_PID pid, BOOL_T* isPresent);

/*! @ingroup OS_APIS_PROCESS
 * Queries whether the current process is being debugged.
 *
 * @param[out] uid          Process descriptor
 *
 * @return      Operation status code.
 * @retval      OS_RETURN_CODE_NO_ERROR             If the operation succeeded
 * @retval      OS_RETURN_CODE_PROCESS_QUERY_FAILED If the operation Failed
 *
 * @par Availability:
 *   @b O/S:   Linux & macOS*\n
 *   @b CPU:   All\n
 */
OS_RETURN_CODE OS_GetUid(NATIVE_UID* uid);

/*! @ingroup OS_APIS_PROCESS
 * Queries whether the current process is being debugged.
 *
 * @param[out] files        Points to an array of NATIVE_FD objects with at least
 *                          3 elements. These elements will be filled with the file
 *                          descriptors of STDIN, STDOUT, STDERR respectively.
 *
 * @return      Operation status code.
 * @retval      OS_RETURN_CODE_NO_ERROR             If the operation succeeded
 * @retval      OS_RETURN_CODE_PROCESS_QUERY_FAILED If the operation Failed
 *
 * @par Availability:
 *   @b O/S:   Windows\n
 *   @b CPU:   All\n
 */
OS_RETURN_CODE OS_FindStdFiles(NATIVE_FD* files);

/*! @ingroup OS_APIS_PROCESS
 * Create a process, possibly redirect its standard input/output/error.
 * The process is created using the system command line interpreter or shell.
 *
 * @param[in]  args         Full command line for the process.
 * @param[in]  stdFiles     Points to an array of NATIVE_FD objects with at least
 *                          3 elements. These elements will be used to determine the
 *                          file descriptors: STDIN, STDOUT, STDERR respectively of
 *                          the newly created process.
 * @param[in]  closeFiles   (Optional) Points to an array of NATIVE_FD objects that needs to be
 *                          closed in the context of the created proces.
 *                          The last entry of this array should be INVALID_NATIVE_FD.
 * @param[in]  environ      Pointer to the environment block to use in the new process.
 * @param[out] process      An opaque object representing the created process.
 *                          One can wait for the process termination by calling
 *                          @ref OS_WaitForProcessTermination
 *
 * @return      Operation status code.
 * @retval      OS_RETURN_CODE_NO_ERROR             If the operation succeeded
 * @retval      OS_RETURN_CODE_PROCESS_QUERY_FAILED If the operation Failed
 *
 * @par Availability:
 *   @b O/S:   Windows, Linux & macOS*\n
 *   @b CPU:   All\n
 */
OS_RETURN_CODE OS_CreateProcess(const CHAR* args, NATIVE_FD* stdFiles, NATIVE_FD* closeFiles, char** environ,
                                OS_PROCESS_WAITABLE_PROCESS* process);

/*! @ingroup OS_APIS_PROCESS
 * Type definition of a pointer to an OS_CreateProcess function.
 */
typedef OS_RETURN_CODE (*OS_FnPtrCreateProcess)(const CHAR* args, NATIVE_FD* stdFiles, NATIVE_FD* closeFiles, char** environ,
                                                OS_PROCESS_WAITABLE_PROCESS* process);

/*! @ingroup OS_APIS_PROCESS
 * Wait for process termination.
 *
 * @param[in]  process      An opaque object representing the process to wait for.
 * @param[out] exitStatus   The exit status of the terminated process.
 *
 * @return      Operation status code.
 * @retval      OS_RETURN_CODE_NO_ERROR             If the operation succeeded
 * @retval      OS_RETURN_CODE_PROCESS_QUERY_FAILED If the operation Failed
 *
 * @par Availability:
 *   @b O/S:   Windows, Linux & macOS*\n
 *   @b CPU:   All\n
 */
OS_RETURN_CODE OS_WaitForProcessTermination(OS_PROCESS_WAITABLE_PROCESS process, UINT32* exitStatus);

/*! @ingroup OS_APIS_PROCESS
 * Trigger a software breakpoint.
 * When the current process is attached to a debugger, this function will stop the process and
 * notify the debugger about a breakpoint that was triggered.
 *
 * @par Availability:
 *   @b O/S:   Windows, Linux & macOS*\n
 *   @b CPU:   All\n
 */
VOID OS_TriggerSoftwareBreakpoint();

#ifndef TARGET_WINDOWS
/*! @ingroup OS_APIS_PROCESS
 * Notify the locking mechanism about a forked child.
 *
 * @note    Must be called earliest as possible by the child process after it was created for functional correctness.
 *
 * @par Availability:
 *   - @b O/S:   Linux & macOS*
 *   - @b CPU:   All
 */
void OS_NotifyFork();

/*! @ingroup OS_APIS_PROCESS
 * Get the current generation of forked processes.
 * This value is increased by one every time OS_NotifyFork() is called.
 *
 * @par Availability:
 *   - @b O/S:   Linux & macOS*
 *   - @b CPU:   All
 */
UINT32 OS_GetForksCount();

#endif // TARGET_WINDOWS

#endif // file guard
