/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#ifndef _MIX_FP_STATE_H_
#define _MIX_FP_STATE_H_

#define MIX_FP_ALIGN(x) reinterpret_cast< unsigned char* >((((reinterpret_cast< ADDRINT >(x) + 16) >> 4) << 4))

#if defined(_WIN32) && !defined(__GNUC__)
extern "C" void mix_fp_save(void* p);    /* assembly routine */
extern "C" void mix_fp_restore(void* p); /* assembly routine */
#define MIX_FP_SAVE(x) (mix_fp_save(x))
#define MIX_FP_RELOAD(x) (mix_fp_restore(x))

#else

#define MIX_FP_SAVE(x) asm volatile("fxsave %0 \n\t emms" : "=m"(*x))

// need the * because  x is an array and need to make it an lvalue
#define MIX_FP_RELOAD(x) asm volatile("fxrstor %0" : : "m"(*x))

#endif

#if defined(_WIN32) && !defined(__GNUC__)
typedef unsigned char __declspec(align(16)) mix_fp_save_buffer_t[512 + 16];
#else
typedef unsigned char mix_fp_save_buffer_t[512 + 16] __attribute__((aligned(16)));
#endif

#endif
