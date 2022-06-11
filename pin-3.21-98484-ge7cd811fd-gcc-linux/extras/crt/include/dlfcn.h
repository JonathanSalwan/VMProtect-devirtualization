/*
 * Copyright (C) 2015-2019 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * Copyright (C) 2008 The Android Open Source Project
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
#ifndef __DLFCN_H__
#define __DLFCN_H__

#include <sys/cdefs.h>

__BEGIN_DECLS

typedef struct {
    const char *dli_fname;  /* Pathname of shared object that
                               contains address */
    void       *dli_fbase;  /* Address at which shared object
                               is loaded */
    const char *dli_sname;  /* Name of nearest symbol with address
                               lower than addr */
    void       *dli_saddr;  /* Exact address of symbol named
                               in dli_sname */
} Dl_info;

extern void*        dlopen(const char*  filename, int flag);
extern int          dlclose(void*  handle);
extern const char*  dlerror(void);
extern void*        dlsym(void*  handle, const char*  symbol);
extern int          dladdr(const void* addr, Dl_info *info);

enum {
#if defined(__APPLE__)
  RTLD_LAZY   = 1,
  RTLD_NOW    = 2,
  RTLD_LOCAL  = 4,
  RTLD_GLOBAL = 8,
  RTLD_NOLOAD = 0x10,
  RTLD_NODELETE = 0x80,
  RTLD_FIRST = 0x100,   /* macOS 10.5 and later */
#else
# if defined(__LP64__)
  RTLD_NOW  = 2,
# else
  RTLD_NOW  = 0,
# endif
  RTLD_LAZY = 1,

  RTLD_LOCAL  = 0,
# if defined(__LP64__)
  RTLD_GLOBAL = 0x00100,
# else
  RTLD_GLOBAL = 2,
# endif
  RTLD_NOLOAD = 4,
#endif
};

#if defined(__APPLE__)
# define RTLD_NEXT       ((void *) -1)   /* Search subsequent objects. */
# define RTLD_DEFAULT    ((void *) -2)   /* Use default search algorithm. */
# define RTLD_SELF       ((void *) -3)   /* Search this and subsequent objects (macOS 10.5 and later) */
# define RTLD_MAIN_ONLY  ((void *) -5)   /* Search main executable only (macOS 10.5 and later) */
#else
# if defined (__LP64__)
#  define RTLD_DEFAULT  ((void*) 0)
#  define RTLD_NEXT     ((void*) -1L)
# else
#  define RTLD_DEFAULT  ((void*) 0xffffffff)
#  define RTLD_NEXT     ((void*) 0xfffffffe)
# endif
#endif

#if defined(__APPLE__) && defined(__NEED_DYLD_PRIVATE)
#include <mach-o/dyld.h>
#include <mach-o/dyld_images.h>

struct dyld_unwind_sections
{
	const struct mach_header*		mh;
	const void*						dwarf_section;
	uintptr_t						dwarf_section_length;
	const void*						compact_unwind_section;
	uintptr_t						compact_unwind_section_length;
};

extern bool _dyld_find_unwind_sections(void* addr, struct dyld_unwind_sections* info);
extern void _dyld_register_func_for_add_image(void (*func)(const struct mach_header* mh, intptr_t vmaddr_slide));
extern void _dyld_register_func_for_remove_image(void (*func)(const struct mach_header* mh, intptr_t vmaddr_slide));

#endif

__END_DECLS

#endif /* __DLFCN_H */


