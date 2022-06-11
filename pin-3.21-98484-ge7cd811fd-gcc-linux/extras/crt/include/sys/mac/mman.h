/*
 * Copyright (C) 2017-2019 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*-
 * Copyright (c) 1982, 1986, 1993
 *      The Regents of the University of California.  All rights reserved.
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
 *      @(#)mman.h      8.2 (Berkeley) 1/9/95
 * $FreeBSD$
 */

#ifndef _SYS_MMAN_MAC_H_
#define _SYS_MMAN_MAC_H_

#include <sys/cdefs.h>
#include <sys/types.h>


/*
 * Protections are chosen from these bits, or-ed together
 */
#define PROT_NONE   0x00    /* no permissions */
#define PROT_READ   0x01    /* pages can be read */
#define PROT_WRITE  0x02    /* pages can be written */
#define PROT_EXEC   0x04    /* pages can be executed */

/*
 * Flags contain sharing type and options.
 * Sharing types; choose one.
 */
#define MAP_SHARED  0x0001      /* share changes */
#define MAP_PRIVATE 0x0002      /* changes are private */
#define MAP_COPY    MAP_PRIVATE /* Obsolete */

/*
 * Other flags
 */
#define MAP_FIXED    0x0010 /* map addr must be exactly as requested */

#define MAP_RESERVED0020 0x0020 /* previously unimplemented MAP_RENAME */
#define MAP_RESERVED0040 0x0040 /* previously unimplemented MAP_NORESERVE */
#define MAP_RESERVED0080 0x0080 /* previously misimplemented MAP_INHERIT */
#define MAP_RESERVED0100 0x0100 /* previously unimplemented MAP_NOEXTEND */
#define MAP_HASSEMAPHORE 0x0200 /* region may contain semaphores */
#define MAP_STACK    0x0400 /* region grows down, like a stack */
#define MAP_NOSYNC   0x0800 /* page to but do not sync underlying file */

/*
 * Mapping type
 */
#define MAP_FILE        0x0000 /* map from file (default) */
#define MAP_ANON        0x1000 /* allocated from memory, swap space */
#define MAP_ANONYMOUS   MAP_ANON /* For compatibility. */

__BEGIN_DECLS

#define MAP_FAILED ((void *)-1)

#define MREMAP_MAYMOVE  1
#define MREMAP_FIXED    2

extern void* mmap(void*, size_t, int, int, int, off_t);
extern void* mmap64(void*, size_t, int, int, int, off64_t);
extern int munmap(void*, size_t);
extern int msync(const void*, size_t, int);
extern int mprotect(const void*, size_t, int);
extern void* mremap(void*, size_t, size_t, int);

extern int mlockall(int);
extern int munlockall(void);
extern int mlock(const void*, size_t);
extern int munlock(const void*, size_t);
extern int madvise(const void*, size_t, int);

extern int mlock(const void*, size_t);
extern int munlock(const void*, size_t);

extern int mincore(void*, size_t, unsigned char*);

__END_DECLS

#endif /* _SYS_MMAN_MAC_H_ */
