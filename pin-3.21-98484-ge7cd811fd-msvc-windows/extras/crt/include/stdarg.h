/*
 * Copyright (C) 2015-2019 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#ifndef _STDARAG_H_
#define _STDARAG_H_

#include <stdint.h>

#ifndef _VA_LIST_DEFINED
typedef char*  va_list;
# define _VA_LIST_DEFINED
#endif
#ifndef __GNUC__
typedef va_list __va_list;
#endif

#ifdef  __cplusplus
extern "C" {
#endif
/* Builtin intrinsic function */
extern void __cdecl __va_start(va_list *, ...);
#ifdef  __cplusplus
}
#endif

#define _MACHINE_WORD ( sizeof(void*) )
#define _ALIGN_ADDR_TO_MACHINE_WORD(ptr,v) ( ( (uintptr_t)ptr + sizeof(v) + _MACHINE_WORD - 1 ) & ~( _MACHINE_WORD - 1 ) )
#define _ADVANCE_PTR_AND_RETURN_PREVIOUS(ptr,sz) ( ( ptr += sz ) - sz )

#define va_arg0(ap,t,sz)    ( *(t *) _ADVANCE_PTR_AND_RETURN_PREVIOUS(ap,(sz)))

#if defined(TARGET_IA32)
# define va_start(ap,v)  ( ap = (va_list)_ALIGN_ADDR_TO_MACHINE_WORD(&(v),v) )
# define va_arg(ap,t)    va_arg0(ap,t,_ALIGN_ADDR_TO_MACHINE_WORD(0,t))
#elif defined(TARGET_IA32E)
# define va_start(ap, x) __va_start(&ap, x)
# define va_arg(ap,t)    ( ( sizeof(t) > _MACHINE_WORD || ( sizeof(t) & (sizeof(t) - 1) ) != 0 ) ? *va_arg0(ap,t*,_MACHINE_WORD) : va_arg0(ap,t,_MACHINE_WORD) )
#else
# error Unsupported architecture
#endif

#define va_end(ap)      ( ap = (va_list)0 )
#define va_copy(dest, src) (dest = src)

#endif // _STDARAG_H_


