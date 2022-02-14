/*
 * Copyright (C) 2006-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 *
 * this headerfile contains defines the types that are the foundation of
 * other code
 *
 */

#ifndef TYPES_FOUNDATION_PH
#define TYPES_FOUNDATION_PH

#ifdef PIN_CRT
#include <types.h>
#endif

#ifdef __cplusplus
typedef bool BOOL;
#else
typedef unsigned char BOOL;
#endif

#ifdef PIN_CRT
using ::ADDRINT;
using ::UINT32;
using ::UINT64;

#else // !PIN_CRT
#if !defined(VOID)
typedef void VOID;
#endif

typedef char CHAR;
typedef unsigned int UINT;
typedef int INT;
typedef double FLT64;
typedef float FLT32;
typedef unsigned int USIZE;

/*
 * Generic type for three-state logic.
 */
enum TRI
{
    TRI_YES,
    TRI_NO,
    TRI_MAYBE
};

typedef uint8_t UINT8;
typedef uint16_t UINT16;
typedef uint32_t UINT32;
typedef uint64_t UINT64;
typedef int8_t INT8;
typedef int16_t INT16;
typedef int32_t INT32;
typedef int64_t INT64;

/*
 * Unsigned integer of the same size as a pointer on the TARGET architecture.
 * This quantity can be converted to and from an OADDR/IADDR
 *
 * There is an option of overflow and underflow when using ADDRDELTA.
 * For example, let's look at a 32 bit address.
 * 32 bit address is 32 bit unsigned int number.
 * This means that the maximum delta between two addresses is (2^32 - 1), which means that the ADDRDELTA which is signed may overflow.
 */
#if defined(TARGET_IA32)
typedef UINT32 ADDRINT;
typedef INT32 ADDRDELTA;

#elif defined(TARGET_IA32E)
typedef UINT64 ADDRINT;
typedef INT64 ADDRDELTA;

#else
#error "Unsupported target architecture"
#endif

#endif // PIN_CRT

/*
 * Generic macro definitions
 */

#ifndef MAX
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

#endif
