/*
 * Copyright (C) 2015-2019 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#ifndef __TOOLCHAINS_H__
#define __TOOLCHAINS_H__

#ifdef _MSC_VER

#pragma section(".CRT$XIB",read)
#define INITIALIZER(f) \
   static void __cdecl f(void); \
   __declspec(allocate(".CRT$XIB")) void (__cdecl*f##_)(void) = f; \
   static void __cdecl f(void)

#elif defined(__APPLE__)

struct ProgramVars
{
	const void*		mh;
	int*			NXArgcPtr;
	const char***	NXArgvPtr;
	const char***	environPtr;
	const char**	__prognamePtr;
};

#define INITIALIZER(f) \
   static void f(int argc, char* argv[], char* envp[],const char* apple[], const ProgramVars* vars) __attribute__((constructor)); \
   static void f(int argc, char* argv[], char* envp[],const char* apple[], const ProgramVars* vars)

#elif defined(__GNUC__)

#define INITIALIZER(f) \
   static void f(void) __attribute__((constructor)); \
   static void f(void)

#endif

#if !defined(_LIBC) && defined(_MSC_VER)
#define CRT_DLLIMPORT __declspec(dllimport)
#else
#define CRT_DLLIMPORT
#endif


#endif // __TOOLCHAINS_H__
