/*
 * Copyright (C) 2013-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// <COMPONENT>: os-apis
// <FILE-TYPE>: component public header
/// @file os_return_codes.h

/*!
 * @addtogroup OS_APIS_DEF Generic error codes
 */

#ifndef OS_APIS_OS_RETURN_CODES_H
#define OS_APIS_OS_RETURN_CODES_H

/*! @ingroup OS_APIS_DEF
 * Describe an OS-API cross OS, generic error
 */
typedef enum
{
    OS_RETURN_CODE_NO_ERROR = 0,
    OS_RETURN_CODE_INVALID_ARGS,
    OS_RETURN_CODE_MEMORY_MAP_FAILED,
    OS_RETURN_CODE_MEMORY_FREE_FAILED,
    OS_RETURN_CODE_QUERY_FAILED,
    OS_RETURN_CODE_FLUSH_ICACHE_FAILED,
    OS_RETURN_CODE_MEMORY_PROTECT_FAILED,
    OS_RETURN_CODE_MEMORY_INFO_FAILED,
    OS_RETURN_CODE_FILE_OPERATION_FAILED,
    OS_RETURN_CODE_FILE_OPEN_FAILED,
    OS_RETURN_CODE_FILE_WRITE_FAILED,
    OS_RETURN_CODE_FILE_READ_FAILED,
    OS_RETURN_CODE_FILE_SEEK_FAILED,
    OS_RETURN_CODE_FILE_CLOSE_FAILED,
    OS_RETURN_CODE_FILE_DELETE_FAILED,
    OS_RETURN_CODE_FILE_FLUSH_FAILED,
    OS_RETURN_CODE_FILE_QUERY_FAILED,
    OS_RETURN_CODE_THREAD_CREATE_FAILED,
    OS_RETURN_CODE_THREAD_EXIT_FAILED,
    OS_RETURN_CODE_THREAD_SUSPEND_FAILED,
    OS_RETURN_CODE_THREAD_RESUME_FAILED,
    OS_RETURN_CODE_THREAD_SLEEP_FAILED,
    OS_RETURN_CODE_THREAD_QUERY_FAILED,
    OS_RETURN_CODE_THREAD_EXCEPTION_FAILED,
    OS_RETURN_CODE_PROCESS_QUERY_FAILED,
    OS_RETURN_CODE_PROCESS_UPDATE_FAILED,
    OS_RETURN_CODE_TIME_QUERY_FAILED,
    OS_RETURN_CODE_BUFFER_TOO_SHORT,
    OS_RETURN_CODE_INTERRUPTED,
    OS_RETURN_CODE_FILE_EXIST,
    OS_RETURN_CODE_NOT_IMPLEMENTED_YET,
    OS_RETURN_CODE_EXCEED_LIMIT_OF_OPEN_OBJECTS,
    OS_RETURN_CODE_NO_MEMORY,
    OS_RETURN_CODE_PROCESS_CREATION_FAILED,
    OS_RETURN_CODE_PROCESS_WAIT_FAILED,
    OS_RETURN_CODE_NOT_IMPLEMENTED,
    OS_RETURN_CODE_RPC_FAILED,
    OS_RETURN_CODE_BAD_FILE_DESCRIPTOR
} OS_RETURN_CODE_GENERIC;

/*! @ingroup OS_APIS_DEF
 * Full error code returned from OS-APIs functions
 */
typedef struct _OS_RETURN_CODE
{
    OS_RETURN_CODE_GENERIC generic_err; //!< Generic OS-APIs error
    int os_specific_err;                //!< OS specific error code as returned from the system call
} OS_RETURN_CODE;

typedef struct _OS_RETURN_CODE_POSIX
{
    ADDRINT function_return_value;
    ADDRINT return_errno;
} OS_RETURN_CODE_POSIX;

#ifdef __cplusplus

extern "C++"
{
    inline bool operator==(const OS_RETURN_CODE_GENERIC& generic, const OS_RETURN_CODE& code)
    {
        return code.generic_err == generic;
    }

    inline bool operator!=(const OS_RETURN_CODE_GENERIC& generic, const OS_RETURN_CODE& code)
    {
        return code.generic_err != generic;
    }
}
#endif

#define OS_RETURN_CODE_IS_SUCCESS(r) (OS_RETURN_CODE_NO_ERROR == r.generic_err)

#define OS_RETURN_CODE_POSIX_IS_SUCCESS(r) (-1 != r.function_return_value)

#define OS_RETURN_CODE_DECLARE(lvalue) OS_RETURN_CODE lvalue = {OS_RETURN_CODE_NO_ERROR, 0}

#define OS_RETURN_CODE_ASSIGN_ERROR(lvalue, r, errno) \
    {                                                 \
        lvalue.generic_err     = r;                   \
        lvalue.os_specific_err = (int)errno;          \
    }

#define OS_RETURN_CODE_RETURN_ERROR(r, errno) \
    {                                         \
        OS_RETURN_CODE tmpRet;                \
        tmpRet.generic_err     = r;           \
        tmpRet.os_specific_err = (int)errno;  \
        return tmpRet;                        \
    }

#define OS_RETURN_CODE_RETURN_SUCCESS OS_RETURN_CODE_RETURN_ERROR(OS_RETURN_CODE_NO_ERROR, 0)

#define OS_RETURN_CODE_RETURN_ERROR_UNIX(r, ret) OS_RETURN_CODE_RETURN_ERROR(r, OS_SyscallReturnValue(ret))
#define OS_RETURN_CODE_RETURN_ERROR_WINDOWS(r, ret) OS_RETURN_CODE_RETURN_ERROR(r, ret)

#define OS_RETURN_CODE_RETURN_ERROR_GENERIC_ONLY(r) OS_RETURN_CODE_RETURN_ERROR(r, 0)

#endif
