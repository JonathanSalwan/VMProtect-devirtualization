/*
 * Copyright (C) 2015-2017 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*  $NetBSD: cdefs.h,v 1.58 2004/12/11 05:59:00 christos Exp $  */

/*
 * Copyright (c) 1991, 1993
 *  The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Berkeley Software Design, Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *  @(#)cdefs.h 8.8 (Berkeley) 1/9/95
 */

#ifndef _SYS_CDEFS_H_
#define _SYS_CDEFS_H_

/*
 * Testing against Clang-specific extensions.
 */

#ifndef __has_extension
#define __has_extension         __has_feature
#endif
#ifndef __has_feature
#define __has_feature(x)        0
#endif
#ifndef __has_include
#define __has_include(x)        0
#endif
#ifndef __has_builtin
#define __has_builtin(x)        0
#endif


/*
 * Macro to test if we're using a GNU C compiler of a specific vintage
 * or later, for e.g. features that appeared in a particular version
 * of GNU C.  Usage:
 *
 *  #if __GNUC_PREREQ(major, minor)
 *  ...cool feature...
 *  #else
 *  ...delete feature...
 *  #endif
 */
#ifdef __GNUC__
#define __GNUC_PREREQ(x, y)                     \
    ((__GNUC__ == (x) && __GNUC_MINOR__ >= (y)) ||          \
     (__GNUC__ > (x)))
#else
#define __GNUC_PREREQ(x, y) 0
#endif

#include <sys/cdefs_elf.h>

#if defined(__cplusplus)
#define __BEGIN_DECLS       extern "C" {
#define __END_DECLS     }
#define __static_cast(x,y)  static_cast<x>(y)
#else
#define __BEGIN_DECLS
#define __END_DECLS
#define __static_cast(x,y)  (x)y
#endif

/*
 * The __CONCAT macro is used to concatenate parts of symbol names, e.g.
 * with "#define OLD(foo) __CONCAT(old,foo)", OLD(foo) produces oldfoo.
 * The __CONCAT macro is a bit tricky -- make sure you don't put spaces
 * in between its arguments.  __CONCAT can also concatenate double-quoted
 * strings produced by the __STRING macro, but this only works with ANSI C.
 */

#define ___STRING(x)    __STRING(x)
#define ___CONCAT(x,y)  __CONCAT(x,y)

#if defined(__STDC__) || defined(__cplusplus)
#define __P(protos) protos      /* full-blown ANSI C */
#define __CONCAT(x,y)   x ## y
#define __STRING(x) #x

#define __const     const       /* define reserved names to standard */
#define __signed    signed
#ifndef __volatile
# define    __volatile  volatile
#endif
#if defined(__cplusplus)
#define __inline    inline      /* convert to C++ keyword */
#else
#if !defined(__GNUC__) && !defined(__lint__)
#define __inline            /* delete GCC keyword */
#endif /* !__GNUC__  && !__lint__ */
#endif /* !__cplusplus */

#else   /* !(__STDC__ || __cplusplus) */
#define __P(protos) ()      /* traditional C preprocessor */

#ifdef _MSC_VER
#define __CONCAT(x,y) x ## y
#define __STRING(x) #x

#else

#define __CONCAT(x,y)   x/**/y
#define __STRING(x) "x"

#endif

#if !defined(__GNUC__) && !defined(_MSC_VER)
#define __const             /* delete pseudo-ANSI C keywords */
#define __inline
#define __signed
#ifndef __volatile
# define    __volatile
#endif
#endif  /* !__GNUC__ */

/*
 * In non-ANSI C environments, new programs will want ANSI-only C keywords
 * deleted from the program and old programs will want them left alone.
 * Programs using the ANSI C keywords const, inline etc. as normal
 * identifiers should define -DNO_ANSI_KEYWORDS.
 */
#ifndef NO_ANSI_KEYWORDS
#define const       __const     /* convert ANSI C keywords */
#define inline      __inline
#define signed      __signed
#define volatile    __volatile
#endif /* !NO_ANSI_KEYWORDS */
#endif  /* !(__STDC__ || __cplusplus) */

/*
 * Used for internal auditing of the NetBSD source tree.
 */
#ifdef __AUDIT__
#define __aconst    __const
#else
#define __aconst
#endif

/*
 * The following macro is used to remove const cast-away warnings
 * from gcc -Wcast-qual; it should be used with caution because it
 * can hide valid errors; in particular most valid uses are in
 * situations where the API requires it, not to cast away string
 * constants. We don't use *intptr_t on purpose here and we are
 * explicit about unsigned long so that we don't have additional
 * dependencies.
 */
#define __UNCONST(a)    ((void *)(ADDRINT)(const void *)(a))

/*
 * GCC2 provides __extension__ to suppress warnings for various GNU C
 * language extensions under "-ansi -pedantic".
 */
#if !__GNUC_PREREQ(2, 0)
#define __extension__       /* delete __extension__ if non-gcc or gcc1 */
#endif

/*
 * GCC1 and some versions of GCC2 declare dead (non-returning) and
 * pure (no side effects) functions using "volatile" and "const";
 * unfortunately, these then cause warnings under "-ansi -pedantic".
 * GCC2 uses a new, peculiar __attribute__((attrs)) style.  All of
 * these work for GNU C++ (modulo a slight glitch in the C++ grammar
 * in the distribution version of 2.5.5).
 */
#if !__GNUC_PREREQ(2, 5)
#define __attribute__(x)    /* delete __attribute__ if non-gcc or gcc1 */
#if defined(__GNUC__) && !defined(__STRICT_ANSI__)
#define __dead      __volatile
#define __pure      __const
#endif
#endif

/* Delete pseudo-keywords wherever they are not available or needed. */
#ifndef __dead
#define __dead
#define __pure
#endif

#if __GNUC_PREREQ(2, 7)
#define __unused    __attribute__((__unused__))
#else
#define __unused    /* delete */
#endif

#define __pure2 __attribute__((__const__)) /* Android-added: used by FreeBSD libm */

#if __GNUC_PREREQ(3, 1)
#define __used      __attribute__((__used__))
#else
#define __used      /* delete */
#endif

#if __GNUC_PREREQ(2, 7)
#define __packed    __attribute__((__packed__))
#define __aligned(x)    __attribute__((__aligned__(x)))
#define __section(x)    __attribute__((__section__(x)))
#elif defined(__lint__)
#define __packed    /* delete */
#define __aligned(x)    /* delete */
#define __section(x)    /* delete */
#else
#define __packed    error: no __packed for this compiler
#define __aligned(x)    error: no __aligned for this compiler
#define __section(x)    error: no __section for this compiler
#endif

#if !__GNUC_PREREQ(2, 8)
#define __extension__
#endif

#if __GNUC_PREREQ(2, 8)
#define __statement(x)  __extension__(x)
#elif defined(lint)
#define __statement(x)  (0)
#else
#define __statement(x)  (x)
#endif

#ifdef __nonnull
# undef __nonnull
#endif // __nonnull

#define __nonnull(args) __attribute__((__nonnull__ args))

#define __printflike(x, y) __attribute__((__format__(printf, x, y))) __nonnull((x))
#define __scanflike(x, y) __attribute__((__format__(scanf, x, y))) __nonnull((x))

/*
 * C99 defines the restrict type qualifier keyword, which was made available
 * in GCC 2.92.
 */
#if defined(__STDC__VERSION__) && __STDC_VERSION__ >= 199901L
#define __restrict  restrict
#else
#if !__GNUC_PREREQ(2, 92)
#define __restrict  /* delete __restrict when not supported */
#endif
#endif

/*
 * C99 defines __func__ predefined identifier, which was made available
 * in GCC 2.95.
 */
#if !defined(__STDC_VERSION__) || !(__STDC_VERSION__ >= 199901L)
#if __GNUC_PREREQ(2, 6)
#define __func__    __PRETTY_FUNCTION__
#elif __GNUC_PREREQ(2, 4)
#define __func__    __FUNCTION__
#else
#define __func__    ""
#endif
#endif /* !(__STDC_VERSION__ >= 199901L) */

#if defined(_KERNEL)
#if defined(NO_KERNEL_RCSIDS)
#undef __KERNEL_RCSID
#define __KERNEL_RCSID(_n, _s)      /* nothing */
#endif /* NO_KERNEL_RCSIDS */
#endif /* _KERNEL */

/*
 * A barrier to stop the optimizer from moving code or assume live
 * register values. This is gcc specific, the version is more or less
 * arbitrary, might work with older compilers.
 */
#if __GNUC_PREREQ(2, 95)
#define __insn_barrier()    __asm __volatile("":::"memory")
#else
#define __insn_barrier()    /* */
#endif

/*
 * GNU C version 2.96 adds explicit branch prediction so that
 * the CPU back-end can hint the processor and also so that
 * code blocks can be reordered such that the predicted path
 * sees a more linear flow, thus improving cache behavior, etc.
 *
 * The following two macros provide us with a way to use this
 * compiler feature.  Use __predict_true() if you expect the expression
 * to evaluate to true, and __predict_false() if you expect the
 * expression to evaluate to false.
 *
 * A few notes about usage:
 *
 *  * Generally, __predict_false() error condition checks (unless
 *    you have some _strong_ reason to do otherwise, in which case
 *    document it), and/or __predict_true() `no-error' condition
 *    checks, assuming you want to optimize for the no-error case.
 *
 *  * Other than that, if you don't know the likelihood of a test
 *    succeeding from empirical or other `hard' evidence, don't
 *    make predictions.
 *
 *  * These are meant to be used in places that are run `a lot'.
 *    It is wasteful to make predictions in code that is run
 *    seldomly (e.g. at subsystem initialization time) as the
 *    basic block reordering that this affects can often generate
 *    larger code.
 */
#if __GNUC_PREREQ(2, 96)
#define __predict_true(exp) __builtin_expect((exp) != 0, 1)
#define __predict_false(exp)    __builtin_expect((exp) != 0, 0)
#else
#define __predict_true(exp) (exp)
#define __predict_false(exp)    (exp)
#endif

#if __GNUC_PREREQ(2, 96)
#define __noreturn    __attribute__((__noreturn__))
#define __mallocfunc  __attribute__((malloc))
#define __purefunc    __attribute__((pure))
#else
#define __noreturn
#define __mallocfunc
#define __purefunc
#endif

#if __GNUC_PREREQ(3, 1)
#define __always_inline __attribute__((__always_inline__))
#else
#define __always_inline
#endif

#if __GNUC_PREREQ(3, 4)
#define __wur __attribute__((__warn_unused_result__))
#else
#define __wur
#endif

#if __GNUC_PREREQ(4, 3) && defined(ENABLED_WARN_ATTR)
#define __errordecl(name, msg) extern void name(void) __attribute__((__error__(msg)))
#define __warnattr(msg) __attribute__((__warning__(msg)))
#else
#define __errordecl(name, msg) extern void name(void)
#define __warnattr(msg)
#endif

/*
 * Some BSD source needs these macros.
 * Originally they embedded the rcs versions of each source file
 * in the generated binary. We strip strings during build anyway,.
 */
#define __IDSTRING(_prefix,_s) /* nothing */
#define __COPYRIGHT(_s) /* nothing */
#define __FBSDID(_s) /* nothing */
#define __RCSID(_s) /* nothing */
#define __SCCSID(_s) /* nothing */

/*
 * _BSD_SOURCE and _GNU_SOURCE are expected to be defined by callers before
 * any standard header file is included. In those header files we test
 * against __USE_BSD and __USE_GNU. glibc does this in <features.h> but we
 * do it in <sys/cdefs.h> instead because that's where our existing
 * _POSIX_C_SOURCE tests were, and we're already confident that <sys/cdefs.h>
 * is included everywhere it should be.
 *
 * The _GNU_SOURCE test needs to come before any _BSD_SOURCE or _POSIX* tests
 * because _GNU_SOURCE implies everything else.
 */
#if defined(_GNU_SOURCE)
# define __USE_GNU 1
# undef _POSIX_SOURCE
# define _POSIX_SOURCE 1
# undef _POSIX_C_SOURCE
# define _POSIX_C_SOURCE 200809L
# undef _BSD_SOURCE
# define _BSD_SOURCE 1
#endif

#if defined(_BSD_SOURCE)
# define __USE_BSD 1
#endif

/*-
 * POSIX.1 requires that the macros we test be defined before any standard
 * header file is included.
 *
 * Here's a quick run-down of the versions:
 *  defined(_POSIX_SOURCE)      1003.1-1988
 *  _POSIX_C_SOURCE == 1        1003.1-1990
 *  _POSIX_C_SOURCE == 2        1003.2-1992 C Language Binding Option
 *  _POSIX_C_SOURCE == 199309       1003.1b-1993
 *  _POSIX_C_SOURCE == 199506       1003.1c-1995, 1003.1i-1995,
 *                  and the omnibus ISO/IEC 9945-1: 1996
 *  _POSIX_C_SOURCE == 200112       1003.1-2001
 *  _POSIX_C_SOURCE == 200809       1003.1-2008
 *
 * In addition, the X/Open Portability Guide, which is now the Single UNIX
 * Specification, defines a feature-test macro which indicates the version of
 * that specification, and which subsumes _POSIX_C_SOURCE.
 *
 * Our macros begin with two underscores to avoid namespace screwage.
 */

/* Deal with IEEE Std. 1003.1-1990, in which _POSIX_C_SOURCE == 1. */
#if defined(_POSIX_C_SOURCE) && _POSIX_C_SOURCE == 1
#undef _POSIX_C_SOURCE      /* Probably illegal, but beyond caring now. */
#define _POSIX_C_SOURCE     199009
#endif

/* Deal with IEEE Std. 1003.2-1992, in which _POSIX_C_SOURCE == 2. */
#if defined(_POSIX_C_SOURCE) && _POSIX_C_SOURCE == 2
#undef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE     199209
#endif

/* Deal with various X/Open Portability Guides and Single UNIX Spec. */
#ifdef _XOPEN_SOURCE
#if _XOPEN_SOURCE - 0 >= 700
#define __XSI_VISIBLE       700
#undef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE     200809
#elif _XOPEN_SOURCE - 0 >= 600
#define __XSI_VISIBLE       600
#undef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE     200112
#elif _XOPEN_SOURCE - 0 >= 500
#define __XSI_VISIBLE       500
#undef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE     199506
#endif
#endif

/*
 * Deal with all versions of POSIX.  The ordering relative to the tests above is
 * important.
 */
#if defined(_POSIX_SOURCE) && !defined(_POSIX_C_SOURCE)
#define _POSIX_C_SOURCE     198808
#endif
#ifdef _POSIX_C_SOURCE
#if _POSIX_C_SOURCE >= 200809
#define __POSIX_VISIBLE     200809
#define __ISO_C_VISIBLE     1999
#elif _POSIX_C_SOURCE >= 200112
#define __POSIX_VISIBLE     200112
#define __ISO_C_VISIBLE     1999
#elif _POSIX_C_SOURCE >= 199506
#define __POSIX_VISIBLE     199506
#define __ISO_C_VISIBLE     1990
#elif _POSIX_C_SOURCE >= 199309
#define __POSIX_VISIBLE     199309
#define __ISO_C_VISIBLE     1990
#elif _POSIX_C_SOURCE >= 199209
#define __POSIX_VISIBLE     199209
#define __ISO_C_VISIBLE     1990
#elif _POSIX_C_SOURCE >= 199009
#define __POSIX_VISIBLE     199009
#define __ISO_C_VISIBLE     1990
#else
#define __POSIX_VISIBLE     198808
#define __ISO_C_VISIBLE     0
#endif /* _POSIX_C_SOURCE */
#else
/*-
 * Deal with _ANSI_SOURCE:
 * If it is defined, and no other compilation environment is explicitly
 * requested, then define our internal feature-test macros to zero.  This
 * makes no difference to the preprocessor (undefined symbols in preprocessing
 * expressions are defined to have value zero), but makes it more convenient for
 * a test program to print out the values.
 *
 * If a program mistakenly defines _ANSI_SOURCE and some other macro such as
 * _POSIX_C_SOURCE, we will assume that it wants the broader compilation
 * environment (and in fact we will never get here).
 */
#if defined(_ANSI_SOURCE)   /* Hide almost everything. */
#define __POSIX_VISIBLE     0
#define __XSI_VISIBLE       0
#define __BSD_VISIBLE       0
#define __ISO_C_VISIBLE     1990
#elif defined(_C99_SOURCE)  /* Localism to specify strict C99 env. */
#define __POSIX_VISIBLE     0
#define __XSI_VISIBLE       0
#define __BSD_VISIBLE       0
#define __ISO_C_VISIBLE     1999
#else               /* Default environment: show everything. */
#define __POSIX_VISIBLE     200809
#define __XSI_VISIBLE       700
#define __BSD_VISIBLE       1
#define __ISO_C_VISIBLE     1999
#endif
#endif

/*
 * Default values.
 */
#ifndef __XPG_VISIBLE
# define __XPG_VISIBLE          700
#endif
#ifndef __POSIX_VISIBLE
# define __POSIX_VISIBLE        200809
#endif
#ifndef __ISO_C_VISIBLE
# define __ISO_C_VISIBLE        1999
#endif
#ifndef __BSD_VISIBLE
# define __BSD_VISIBLE          1
#endif

#define  __BIONIC__   1
#include <android/api-level.h>

/* glibc compatibility. */
#if __LP64__
#define __WORDSIZE 64
#else
#define __WORDSIZE 32
#endif

/*
 * When _FORTIFY_SOURCE is defined, automatic bounds checking is
 * added to commonly used libc functions. If a buffer overrun is
 * detected, the program is safely aborted.
 *
 * See
 * http://gcc.gnu.org/onlinedocs/gcc/Object-Size-Checking.html for details.
 * This feature is disabled for PIN CRT
 */
#if 0 && defined(_FORTIFY_SOURCE) && _FORTIFY_SOURCE > 0 && defined(__OPTIMIZE__) && __OPTIMIZE__ > 0
#define __BIONIC_FORTIFY 1
#if _FORTIFY_SOURCE == 2
#define __bos(s) __builtin_object_size((s), 1)
#else
#define __bos(s) __builtin_object_size((s), 0)
#endif
#define __bos0(s) __builtin_object_size((s), 0)

#define __BIONIC_FORTIFY_INLINE extern __inline__ __always_inline __attribute__((gnu_inline))
#endif
#define __BIONIC_FORTIFY_UNKNOWN_SIZE ((size_t) -1)

/* Used to tag non-static symbols that are private and never exposed by the shared library. */
#define __LIBC_HIDDEN__ __attribute__((visibility("hidden")))

/* Like __LIBC_HIDDEN__, but preserves binary compatibility for LP32. */
#ifdef __LP64__
#define __LIBC64_HIDDEN__ __LIBC_HIDDEN__
#else
#define __LIBC64_HIDDEN__ __LIBC_ABI_PUBLIC__
#endif

/* Used to tag non-static symbols that are public and exposed by the shared library. */
#define __LIBC_ABI_PUBLIC__ __attribute__((visibility ("default")))

/* Used to rename functions so that the compiler emits a call to 'x' rather than the function this was applied to. */
#define __RENAME(x) __asm__(#x)

#ifdef TARGET_MAC
# include_next <sys/cdefs.h>
#endif

#endif /* !_SYS_CDEFS_H_ */
