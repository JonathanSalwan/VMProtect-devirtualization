/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * See "swizzleapp.c" for a description of this test.
 */

#include <sys/mman.h>
#include <stdlib.h>
#include <unistd.h>

ADDRINT page_mask;
ADDRINT page_offset_mask;
ADDRINT swizzle_space = 0;
ADDRINT target_space  = 0;

static void make_swizzle_space(ADDRINT val)
{
    int page_size    = getpagesize();
    page_offset_mask = page_size - 1;
    page_mask        = ~page_offset_mask;
    // Get swizzle space by allocating memory and rounding up
    ADDRINT mem   = (ADDRINT)malloc(2 * page_size);
    swizzle_space = (mem + page_size - 1) & page_mask;
    if (0 != mprotect((void*)swizzle_space, page_size, 0))
    {
        perror("mprotect");
        exit(1);
    }
    target_space = val & page_mask;
#if defined(DEBUG_OUTPUT)
    fprintf(stderr, "page mask %p page_offset_mask %p swizzle_space %p target_space %p\n", (void*)page_mask,
            (void*)page_offset_mask, (void*)swizzle_space, (void*)target_space);
#endif
}

static VOID SwizzleRef(ADDRINT* val)
{
#if defined(DEBUG_OUTPUT)
    fprintf(stderr, "swizzle: in %p\n", (void*)*val);
#endif
    if (target_space == 0)
    {
        make_swizzle_space(*val);
    }

    if ((*val & page_mask) != target_space)
    {
        fprintf(stderr, "Multiple target space pages\n");
        exit(1);
    }
    *val = swizzle_space | (*val & page_offset_mask);
#if defined(DEBUG_OUTPUT)
    fprintf(stderr, "  out %p\n", (void*)*val);
#endif
}

static ADDRINT Unswizzle(ADDRINT val)
{
    if (page_mask == 0) return val;

#if defined(DEBUG_OUTPUT)
    fprintf(stderr, "unswizzle: in %p\n", (void*)val);
#endif
    if ((val & page_mask) == swizzle_space) val = target_space | (val & page_offset_mask);
#if defined(DEBUG_OUTPUT)
    fprintf(stderr, "  out %p\n", (void*)val);
#endif
    return val;
}

static void UnswizzleRef(ADDRINT* val) { *val = Unswizzle(*val); }
