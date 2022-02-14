/*
 * Copyright (C) 2016-2016 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#ifndef _PINCRT_USTAT_
#define _PINCRT_USTAT_

#include <sys/types.h>

struct ustat
{
    daddr_t f_tfree;      /* Total free blocks */
    ino_t   f_tinode;     /* Number of free inodes */
    char    f_fname[6];   /* Filsys name */
    char    f_fpack[6];   /* Filsys pack name */
};

#endif // _PINCRT_USTAT_
