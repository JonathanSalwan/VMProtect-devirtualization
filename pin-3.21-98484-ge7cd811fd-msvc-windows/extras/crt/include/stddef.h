/*
 * Copyright (C) 2015-2020 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*===---- stddef.h - Basic type definitions --------------------------------===
 *
 * Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
 * See https://llvm.org/LICENSE.txt for license information.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 *
 *===-----------------------------------------------------------------------===
 */

#if !defined(__STDDEF_H) || defined(__need_ptrdiff_t) ||                       \
    defined(__need_size_t) || defined(__need_wchar_t) ||                       \
    defined(__need_NULL) || defined(__need_wint_t)

#ifndef __has_feature
#define __has_feature(x) 0
#endif

#if !defined(__need_ptrdiff_t) && !defined(__need_size_t) &&                   \
    !defined(__need_wchar_t) && !defined(__need_NULL) &&                       \
    !defined(__need_wint_t)
/* Always define miscellaneous pieces when modules are available. */
#if !__has_feature(modules)
#define __STDDEF_H
#endif
#define __need_ptrdiff_t
#define __need_size_t
#define __need_wchar_t
#define __need_NULL
#define __need_STDDEF_H_misc
/* __need_wint_t is intentionally not defined here. */
#endif

# if __LP64__
#  define __PTRDIFF_TYPE__ __int64
# else
#  define __PTRDIFF_TYPE__ __int32
# endif

#if defined(__need_ptrdiff_t)
#if !defined(_PTRDIFF_T) || __has_feature(modules)
/* Always define ptrdiff_t when modules are available. */
#if !__has_feature(modules)
#define _PTRDIFF_T
#endif
typedef __PTRDIFF_TYPE__ ptrdiff_t;
#endif
#undef __need_ptrdiff_t
#endif /* defined(__need_ptrdiff_t) */

#ifndef __SIZE_TYPE__
#ifdef __i386__
# define __SIZE_TYPE__ unsigned int
#else
# define __SIZE_TYPE__ long unsigned int
#endif
#endif

#if defined(__need_size_t)
#if !defined(_SIZE_T) || __has_feature(modules)
/* Always define size_t when modules are available. */
#if !__has_feature(modules)
#define _SIZE_T
#endif
#ifdef __i386__
// MSVC 64 bit appear to have built-in size_t
typedef __SIZE_TYPE__ size_t;
#endif
#endif
#undef __need_size_t
#endif /*defined(__need_size_t) */

#if defined(__need_STDDEF_H_misc)
/* ISO9899:2011 7.20 (C11 Annex K): Define rsize_t if __STDC_WANT_LIB_EXT1__ is
 * enabled. */
#if (defined(__STDC_WANT_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__ >= 1 && \
     !defined(_RSIZE_T)) || __has_feature(modules)
/* Always define rsize_t when modules are available. */
#if !__has_feature(modules)
#define _RSIZE_T
#endif
typedef __SIZE_TYPE__ rsize_t;
#endif
#endif /* defined(__need_STDDEF_H_misc) */

#ifndef __WCHAR_TYPE__
#define __WCHAR_TYPE__ unsigned short
#endif

#if defined(__need_wchar_t)
#ifndef __cplusplus
/* Always define wchar_t when modules are available. */
#if !defined(_WCHAR_T) || __has_feature(modules)
#if !__has_feature(modules)
#define _WCHAR_T
#if defined(_MSC_EXTENSIONS)
#define _WCHAR_T_DEFINED
#endif
#endif
typedef __WCHAR_TYPE__ wchar_t;
#endif
#endif
#undef __need_wchar_t
#endif /* defined(__need_wchar_t) */

#if defined(__need_NULL)
#undef NULL
#ifdef __cplusplus
#  define NULL 0
#else
#  define NULL ((void*)0)
#endif
#ifdef __cplusplus
#if defined(_MSC_EXTENSIONS) && defined(_NATIVE_NULLPTR_SUPPORTED)
namespace std { typedef decltype(nullptr) nullptr_t; }
using ::std::nullptr_t;
#endif
#endif
#undef __need_NULL
#endif /* defined(__need_NULL) */

#if defined(__need_STDDEF_H_misc)
#if __STDC_VERSION__ >= 201112L || __cplusplus >= 201103L
/* IMPORTANT: the next commented include is dedicated to C++11 which pin
   does not support. We should never reach that include statement during
   build.
   this include causes trouble to klocwork: it does not find this file.
   so we comment it out for now and add some assert to make sure we do
   not get there.
*/
/* #include "__stddef_max_align_t.h" */
static_assert(0, "include stddef_max_align.h: should never reach this place during build");

#if !defined(__CLANG_MAX_ALIGN_T_DEFINED)
#ifndef _MSC_VER
typedef struct {
  long long __clang_max_align_nonce1
      __attribute__((__aligned__(__alignof__(long long))));
  long double __clang_max_align_nonce2
      __attribute__((__aligned__(__alignof__(long double))));
} max_align_t;
#else
typedef double max_align_t;
#endif
#define __CLANG_MAX_ALIGN_T_DEFINED
#endif
#endif

/* Define offsetof macro */
#ifdef __cplusplus

#ifdef  _WIN64
#define offsetof(s,m)   (size_t)( (ptrdiff_t)&reinterpret_cast<const volatile char&>((((s *)0)->m)) )
#else
#define offsetof(s,m)   (size_t)&reinterpret_cast<const volatile char&>((((s *)0)->m))
#endif

#else

#ifdef  _WIN64
#define offsetof(s,m)   (size_t)( (ptrdiff_t)&(((s *)0)->m) )
#else
#define offsetof(s,m)   (size_t)&(((s *)0)->m)
#endif

#endif /* __cplusplus */

#undef __need_STDDEF_H_misc
#endif  /* defined(__need_STDDEF_H_misc) */

/* Some C libraries expect to see a wint_t here. Others (notably MinGW) will use
__WINT_TYPE__ directly; accommodate both by requiring __need_wint_t */
#if defined(__need_wint_t)
/* Always define wint_t when modules are available. */
#if !defined(_WINT_T) || __has_feature(modules)
#if !__has_feature(modules)
#define _WINT_T
#endif
typedef __WINT_TYPE__ wint_t;
#endif
#undef __need_wint_t
#endif /* __need_wint_t */


#ifdef __cplusplus
extern "C" {
#endif
int __cdecl _resetstkoflw(void);
#ifdef __cplusplus
}
#endif

#endif
