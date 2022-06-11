/*
 * Copyright (C) 2015-2019 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#ifndef _INC_EXCPT
#define _INC_EXCPT

/*
 * Currently, all MS C compilers for Win32 platforms default to 8 byte
 * alignment.
 */
#pragma pack(push,8)

#ifdef  __cplusplus
extern "C" {
#endif

/*
 * Exception disposition return values.
 */
typedef enum _EXCEPTION_DISPOSITION {
    ExceptionContinueExecution,
    ExceptionContinueSearch,
    ExceptionNestedException,
    ExceptionCollidedUnwind
} EXCEPTION_DISPOSITION;

/*
 * Legal values for expression in except().
 */

#define EXCEPTION_EXECUTE_HANDLER       1
#define EXCEPTION_CONTINUE_SEARCH       0
#define EXCEPTION_CONTINUE_EXECUTION    -1

#ifdef  __cplusplus
}
#endif

#pragma pack(pop)

#endif // _INC_EXCPT
