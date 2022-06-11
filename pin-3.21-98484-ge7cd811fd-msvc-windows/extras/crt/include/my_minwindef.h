/*
 * Copyright (C) 2015-2019 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#ifndef __MIN_WIN_DEF_H__
#define __MIN_WIN_DEF_H__

#include "os-apis.h"
#include "wchar.h"
#include "stdint.h"

#define WINAPI __stdcall
#define DLL_PROCESS_CREATE -1
/* WinNT compatible dll entry Op values */
#define DLL_PROCESS_ATTACH 1
#define DLL_THREAD_ATTACH  2
#define DLL_THREAD_DETACH  3
#define DLL_PROCESS_DETACH 0

#define EXCEPTION_MAXIMUM_PARAMETERS 15 // maximum number of exception parameters

#pragma section(".CRT$XCA",long,read)
#pragma section(".CRT$XCZ",long,read)
#pragma section(".CRT$XIA",long,read)
#pragma section(".CRT$XIZ",long,read)

#pragma section(".CRT$XPA",long,read)
#pragma section(".CRT$XPZ",long,read)
#pragma section(".CRT$XTA",long,read)
#pragma section(".CRT$XTZ",long,read)

#define _CRTALLOC(x) __declspec(allocate(x))

typedef void* HANDLE;
typedef uint32_t DWORD;
typedef void* LPVOID;
typedef void* PVOID;
typedef int BOOL;
#define WINAPI      __stdcall
#define APIENTRY    WINAPI

SPECIALCFUN BOOL APIENTRY DllMain(HANDLE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpreserved);

#endif // __MIN_WIN_DEF_H__
