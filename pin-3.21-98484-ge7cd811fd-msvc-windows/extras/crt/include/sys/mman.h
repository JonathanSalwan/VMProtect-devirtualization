/*
 * Copyright (C) 2015-2019 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#ifndef _SYS_MMAN_H_
#define _SYS_MMAN_H_

#ifdef TARGET_MAC
#include <sys/mac/mman.h>
#else
#include <sys/nonmac/mman.h>
#endif


#endif /* _SYS_MMAN_H_ */
