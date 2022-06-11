/*
 * Copyright (C) 2015-2019 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#ifndef __PIN_WINDOWS_H__
#define __PIN_WINDOWS_H__

#define _INCLUDING_WINDOWS_H_

#include <wchar.h>

#undef __volatile
#define WIN32_LEAN_AND_MEAN

int _wcsicmp(const wchar_t *string1, const wchar_t *string2);
#define swprintf_s swprintf

#if !defined(_AMD64_) && !defined(_X86_)
# if defined(TARGET_IA32)
#  define _X86_
# elif defined(TARGET_IA32E)
#  define _AMD64_
# endif
#endif

struct _CONTEXT;

#ifdef _MSC_EXTENSIONS
# undef _MSC_EXTENSIONS
#endif

#define DUMMYSTRUCTNAME
#define DUMMYUNIONNAME
#ifndef _WINDOWS_H_PATH_
# error Must define _WINDOWS_H_PATH_
#endif
#define WINDOWS_H_PATH <_WINDOWS_H_PATH_/Windows.h>
#include WINDOWS_H_PATH
#include <winioctl.h>

#undef _INCLUDING_WINDOWS_H_

#define GetExceptionCode            _exception_code
#define exception_code              _exception_code
#define GetExceptionInformation     (struct _EXCEPTION_POINTERS *)_exception_info
#define exception_info              (struct _EXCEPTION_POINTERS *)_exception_info
#define AbnormalTermination         _abnormal_termination
#define abnormal_termination        _abnormal_termination

#ifdef __cplusplus
extern "C" {
#endif
unsigned long __cdecl _exception_code(void);
void *        __cdecl _exception_info(void);
int           __cdecl _abnormal_termination(void);

#pragma intrinsic(_exception_code)
#pragma intrinsic(_exception_info)
#pragma intrinsic(_abnormal_termination)

#ifdef __cplusplus
}
#endif

#endif // __PIN_WINDOWS_H__
