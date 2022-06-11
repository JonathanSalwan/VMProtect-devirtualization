/*
 * Copyright (C) 2016-2016 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*  $OpenBSD: setjmp.h,v 1.2 2000/08/05 22:07:32 niklas Exp $   */
/*  $NetBSD: setjmp.h,v 1.1 1994/12/20 10:36:43 cgd Exp $   */

/*
 * machine/setjmp.h: machine dependent setjmp-related information.
 */

#ifdef TARGET_WINDOWS

#define _JBLEN  6      /* size, in void*s, of a jmp_buf */

#else

#define _JBLEN  7      /* size, in void*s, of a jmp_buf */

#endif
