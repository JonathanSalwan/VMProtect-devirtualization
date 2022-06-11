/*
 * Copyright (C) 2017-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// <COMPONENT>: os-apis
// <FILE-TYPE>: component public header

#ifndef _WIN_SYSCALLS_H_
#define _WIN_SYSCALLS_H_

/*! @ingroup SYSCALL
 * Integral type that represents system call number.
 */
typedef unsigned __int32 SYSCALL_NUMBER_T;

/*! @ingroup SYSCALL
 * Standard (mandatory) list of system calls to be inspected and monitored by Pin SL
 */
#define STANDARD_SL_SYSCALL_LIST                   \
    SYSCALL_DEF(NtContinue, 2)                     \
    SYSCALL_DEF(NtContinueEx, 2)                   \
    SYSCALL_DEF(NtCallbackReturn, 3)               \
    SYSCALL_DEF(NtGetContextThread, 2)             \
    SYSCALL_DEF(NtSetContextThread, 2)             \
    SYSCALL_DEF(NtTerminateThread, 2)              \
    SYSCALL_DEF(NtTerminateProcess, 2)             \
    SYSCALL_DEF(NtRaiseException, 3)               \
    SYSCALL_DEF(NtAllocateVirtualMemory, 6)        \
    SYSCALL_DEF(NtProtectVirtualMemory, 5)         \
    SYSCALL_DEF(NtCreateSection, 7)                \
    SYSCALL_DEF(NtMapViewOfSection, 10)            \
    SYSCALL_DEF(NtUnmapViewOfSection, 2)           \
    SYSCALL_DEF(NtFreeVirtualMemory, 4)            \
    SYSCALL_DEF(NtFlushInstructionCache, 3)        \
    SYSCALL_DEF(NtSuspendThread, 2)                \
    SYSCALL_DEF(NtCreateProcess, 8)                \
    SYSCALL_DEF(NtCreateProcessEx, 9)              \
    SYSCALL_DEF(NtCreateThread, 8)                 \
    SYSCALL_DEF(NtResumeThread, 2)                 \
    SYSCALL_DEF(NtCreateUserProcess, 11)           \
    SYSCALL_DEF(NtCreateThreadEx, 11)              \
    SYSCALL_DEF(NtDelayExecution, 2)               \
    SYSCALL_DEF(NtYieldExecution, 0)               \
    SYSCALL_DEF(NtQueryInformationProcess, 5)      \
    SYSCALL_DEF(NtSetInformationProcess, 4)        \
    SYSCALL_DEF(NtQueryVirtualMemory, 6)           \
    SYSCALL_DEF(NtAlertResumeThread, 2)            \
    SYSCALL_DEF(NtClose, 1)                        \
    SYSCALL_DEF(NtCreateEvent, 5)                  \
    SYSCALL_DEF(NtCreateFile, 11)                  \
    SYSCALL_DEF(NtCreateMutant, 4)                 \
    SYSCALL_DEF(NtCreateNamedPipeFile, 14)         \
    SYSCALL_DEF(NtCreateSemaphore, 5)              \
    SYSCALL_DEF(NtDeleteFile, 1)                   \
    SYSCALL_DEF(NtDuplicateObject, 7)              \
    SYSCALL_DEF(NtFlushBuffersFile, 2)             \
    SYSCALL_DEF(NtOpenFile, 6)                     \
    SYSCALL_DEF(NtOpenKey, 3)                      \
    SYSCALL_DEF(NtOpenProcess, 4)                  \
    SYSCALL_DEF(NtOpenThread, 4)                   \
    SYSCALL_DEF(NtQueryAttributesFile, 2)          \
    SYSCALL_DEF(NtQueryInformationFile, 5)         \
    SYSCALL_DEF(NtQueryInformationThread, 5)       \
    SYSCALL_DEF(NtQueryObject, 5)                  \
    SYSCALL_DEF(NtQueryPerformanceCounter, 2)      \
    SYSCALL_DEF(NtQuerySecurityObject, 5)          \
    SYSCALL_DEF(NtQuerySystemInformation, 4)       \
    SYSCALL_DEF(NtQueryValueKey, 6)                \
    SYSCALL_DEF(NtReadFile, 9)                     \
    SYSCALL_DEF(NtReadVirtualMemory, 5)            \
    SYSCALL_DEF(NtReleaseMutant, 2)                \
    SYSCALL_DEF(NtReleaseSemaphore, 3)             \
    SYSCALL_DEF(NtResetEvent, 2)                   \
    SYSCALL_DEF(NtSetEvent, 2)                     \
    SYSCALL_DEF(NtSetInformationFile, 5)           \
    SYSCALL_DEF(NtWaitForMultipleObjects, 5)       \
    SYSCALL_DEF(NtWaitForSingleObject, 3)          \
    SYSCALL_DEF(NtWriteFile, 9)                    \
    SYSCALL_DEF(NtWriteVirtualMemory, 5)           \
    SYSCALL_DEF(NtSignalAndWaitForSingleObject, 4) \
    SYSCALL_DEF(NtQueryVolumeInformationFile, 5)   \
    SYSCALL_DEF(NtCreateMailslotFile, 8)

/*! @ingroup SYSCALL
 * Additional (project-specific) list of Windows NT syscalls to be inspected and monitored by Pin SL
 */
#if !defined(EXTRA_SL_SYSCALL_LIST)
#define EXTRA_SL_SYSCALL_LIST
#endif

/*! @ingroup SYSCALL
 * Full (standard + extra) list of "known to Pin" system calls
 */
#define SL_SYSCALL_LIST      \
    STANDARD_SL_SYSCALL_LIST \
    EXTRA_SL_SYSCALL_LIST

/*! @ingroup SYSCALL
 * Maximum number of "known to Pin" system calls
 */
#define MAX_KNOWN_SYSCALLS 128

/*! @ingroup SYSCALL
 * Maximum number of system call arguments
 */
#define MAX_SYSCALL_ARGS 16

/*! @ingroup SYSCALL
 * Enumeration of keys that identify "known to Pin" system calls
 */
typedef enum SYSCALL_KEY
{
#define SYSCALL_DEF(name, num_args) SYSCALL_KEY_##name,
    SL_SYSCALL_LIST
#undef SYSCALL_DEF

        SYSCALL_KEY_END,
    SYSCALL_KEY_UNKNOWN = SYSCALL_KEY_END
} SYSCALL_KEY;

#define SYSCALL_KEY_FIRST ((SYSCALL_KEY)0)

typedef char __TOO_MANY_SYSCALLS__[((unsigned)SYSCALL_KEY_END <= MAX_KNOWN_SYSCALLS) ? 1 : -1];

#endif // _WIN_SYSCALLS_H_
