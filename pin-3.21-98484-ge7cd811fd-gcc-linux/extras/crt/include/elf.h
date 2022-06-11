/*
 * Copyright (C) 2017-2019 Intel Corporation.
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
#ifndef _ELF_H
#define _ELF_H

#include <stdint.h>
#include <sys/types.h>

#include "freebsd/3rd-party/include/elf.h"
#include "freebsd/3rd-party/sys/compat/linux/linux_misc.h"
#include "linux/auxvec.h"
#include "asm-x86/asm/auxvec.h"

#ifdef TARGET_IA32
typedef Elf32_Auxinfo Elf32_auxv_t;
#else
typedef Elf32_Auxinfo Elf32_auxv_t;
typedef Elf64_Auxinfo Elf64_auxv_t;
#endif // TARGET_IA32

/*
 * Copied from bionic
 */
#define ELF_ST_BIND(x) ((x) >> 4)
#define ELF_ST_TYPE(x) (((unsigned int) x) & 0xf)

#endif /* _ELF_H */
