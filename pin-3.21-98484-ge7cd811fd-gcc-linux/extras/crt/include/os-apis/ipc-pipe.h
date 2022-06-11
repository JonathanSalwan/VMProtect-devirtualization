/*
 * Copyright (C) 2015-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// <COMPONENT>: os-apis
// <FILE-TYPE>: component public header
/// @file ipc-pipe.h

/*!
 * @addtogroup OS_APIS_IPC IPC
 */

#ifndef OS_APIS_IPC_PIPE_H
#define OS_APIS_IPC_PIPE_H

#include "os-apis.h"

/*! @ingroup OS_APIS_FILE
 * Pipes creation flags
 */
typedef enum
{
    OS_PIPE_CREATE_FLAGS_NONE                   = 0,        // No flags
    OS_PIPE_CREATE_FLAGS_READ_SIDE_INHERITABLE  = (1 << 0), // Child processes inherits the pipe's read side
    OS_PIPE_CREATE_FLAGS_WRITE_SIDE_INHERITABLE = (1 << 1)  // Child processes inherits the pipe's write side
} OS_PIPE_CREATE_FLAGS;

/*! @ingroup OS_APIS_IPC
 * Creates an anonymous pipe, and returns handles to the read and write ends of the pipe.
 *
 * @param[in]  flags    Pipes creation flags
 * @param[out] readFd   File descriptor for the read side
 * @param[out] writeFd  File descriptor for the write side
 *
 * @retval     OS_RETURN_CODE_NO_ERROR           If the operation succeeded
 * @retval     OS_RETURN_CODE_INVALID_ARGS       One of the input arguments is invalid.
 * @retval     OS_RETURN_CODE_FILE_OPEN_FAILED   If the operation failed
 *
 * @par Availability:
 *   @b O/S:   All \n
 *   @b CPU:   All \n
 */
OS_RETURN_CODE OS_Pipe(OS_PIPE_CREATE_FLAGS flags, NATIVE_FD* readFd, NATIVE_FD* writeFd);

#endif // file guard
