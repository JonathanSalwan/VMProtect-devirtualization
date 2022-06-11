/*
 * Copyright (C) 2016-2016 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*	$OpenBSD: setjmp.h,v 1.1 2004/01/28 01:39:39 mickey Exp $	*/
/*	$NetBSD: setjmp.h,v 1.1 2003/04/26 18:39:47 fvdl Exp $	*/

/*
 * machine/setjmp.h: machine dependent setjmp-related information.
 * These are only the callee-saved registers, code calling setjmp
 * will expect the rest to be clobbered anyway.
 */

#define _JB_RBX		0
#define _JB_RBP		1
#define _JB_R12		2
#define _JB_R13		3
#define _JB_R14		4
#define _JB_R15		5
#define _JB_RSP		6
#define _JB_PC		7

#ifdef TARGET_WINDOWS

# define _JB_RSI	8
# define _JB_RDI	9
# define _JB_XMM6	10
# define _JB_XMM7	12
# define _JB_XMM8	14
# define _JB_XMM9	16
# define _JB_XMM10	18
# define _JB_XMM11	20
# define _JB_XMM12	22
# define _JB_XMM13	24
# define _JB_XMM14	26
# define _JB_XMM15	28

# define	_JBLEN	32		/* size, in void*s, of a jmp_buf */

#else

# define _JB_SIGFLAG	8
# define _JB_SIGMASK	9

# define	_JBLEN	11		/* size, in void*s, of a jmp_buf */

#endif
