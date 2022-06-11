/*
 * Copyright (C) 2015-2019 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#ifndef _CRT_DEFS
#define _CRT_DEFS

#include <stdint.h>
#include <stddef.h>
#include <sal.h>

#ifndef _ERRNO_T_DEFINED
#define _ERRNO_T_DEFINED
typedef int errno_t;
#endif

#ifndef _CONST_RETURN
#ifdef  __cplusplus
#define _CONST_RETURN  const
#define _CRT_CONST_CORRECT_OVERLOADS
#else
#define _CONST_RETURN
#endif
#endif

#if defined(_M_X64)
#define _UNALIGNED __unaligned
#else
#define _UNALIGNED
#endif

#if !defined(_CRT_ALIGN)
#if defined(__midl)
#define _CRT_ALIGN(x)
#else
#define _CRT_ALIGN(x) __declspec(align(x))
#endif
#endif

#ifndef _CRTIMP
# define _CRTIMP
#endif // _CRTIMP

#endif // _CRT_DEFS
