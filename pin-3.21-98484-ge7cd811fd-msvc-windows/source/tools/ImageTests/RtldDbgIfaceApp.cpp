/*
 * Copyright (C) 2021-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <stdlib.h>

// This application is used by the RtldDbgIface* tests. We compile it twice, once with the R_DEBUG_INSTANCE flag and once without.
// The R_DEBUG_INSTANCE creates an instantiation of a symbol called _r_debug.
// the Glibc linker contains a variable called _r_debug, and the linker uses this variable to notify of image load\unload events.
// Pin tracks this variable to check for new image events.
// A problem rises when a _r_debug symbol exists also in the executable. In that case, at some point, the linker will copy its copy of
// _r_debug into the executable's _r_debug, and will use the _r_debug in the executable as the de-facto interface for notifying image events.
// Pin needs to recognize such cases, and know to start tracking the second _r_debug.

struct r_debug
{
    int32_t r_version;
    struct link_map* r_map;
    void* r_brk;
    enum
    {
        RT_CONSISTENT,
        RT_ADD,
        RT_DELETE
    } r_state;
    void* r_ldbase;
};

r_debug temp;

#ifdef R_DEBUG_INSTANCE
struct r_debug _r_debug;
#endif

int main()
{
#ifdef R_DEBUG_INSTANCE
    printf("\nmain r_brk %p", _r_debug.r_brk);
    printf("\nmain r_version 0x%d", _r_debug.r_version);
    printf("\nmain r_state 0x%d", _r_debug.r_state);
#else
    printf("\nmain no _r_debug symbol", temp);
#endif

    printf("\n");

    return 0;
}
