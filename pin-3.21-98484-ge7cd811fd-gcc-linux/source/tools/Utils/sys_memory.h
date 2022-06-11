/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 *  Generic memory management API. 
 */
#ifndef SYS_MEMORY_H
#define SYS_MEMORY_H

#include "stddef.h"

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

    /*!
 *  Memory protection attributes. 
 */
    typedef enum
    {
        MEM_INACESSIBLE,
        MEM_READ_EXEC,
        MEM_READ_WRITE_EXEC
    } MEM_PROTECTION;

    /*!
 *  Return page size in bytes. 
 */
    size_t GetPageSize();

    /*!
 *  Allocate memory pages with the specified protection attributes. 
 */
    void* MemAlloc(size_t size, MEM_PROTECTION protect);

    /*!
 *  Free memory pages at the specified address. 
 */
    void MemFree(void* addr, size_t size);

    /*!
 *  Set specified protection for memory pages at the specified address. 
 */
    int MemProtect(void* addr, size_t size, MEM_PROTECTION protect);

#ifdef __cplusplus
}
#endif

#endif //SYS_MEMORY_H
/* ===================================================================== */
/* eof */
/* ===================================================================== */
