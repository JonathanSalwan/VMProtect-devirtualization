/*
 * Copyright (C) 2015-2019 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#ifndef __MSVC_COMPAT_H__
#define __MSVC_COMPAT_H__

#define __signed__ signed
#define __inline__ __inline

#define __WINT_TYPE__ signed int
#define NO_ANSI_KEYWORDS
#define __STRICT_ANSI__
#define _ANSI_LIBRARY

#ifndef __cplusplus
# ifndef inline
#  define inline
# endif
#endif

#define __builtin_expect(a,b) (a == b)
#define __builtin_memset(d,c,l) memset(d,c,l)

#endif
