/*
 * Copyright (C) 2011-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// Test that probe placed on function close (from libc.so) does not overwrite bytes of the function close_nocancel
#include "pin.H"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tool_macros.h"

#ifdef TARGET_MAC
#define LIBC_NAME "libsystem_kernel.dylib"
#ifdef TARGET_IA32
#define CLOSE_NOCANCEL C_MANGLE("close$NOCANCEL$UNIX2003")
#define CLOSE C_MANGLE("__close")
#else
#define CLOSE_NOCANCEL C_MANGLE("close_nocancel")
#define CLOSE C_MANGLE("close")
#endif
#else
#define LIBC_NAME "libc.so"
#define CLOSE_NOCANCEL C_MANGLE("close_nocancel")
#define CLOSE C_MANGLE("close")
#endif

typedef int (*func_ptr_t)(int);

static AFUNPTR close_orig;

static int close_replacement(int fd)
{
    printf("close_replacement called\n");

    return ((func_ptr_t)close_orig)(fd);
}

/*
 * Instrumentation routines
 */

VOID ImageLoad(IMG img, VOID* v)

{
    const char* name = IMG_Name(img).c_str();
    if (!strstr(name, LIBC_NAME))
    {
        return;
    }
    printf("image: %s\n", name);

    RTN close_routine = RTN_FindByName(img, CLOSE);
    if (!RTN_Valid(close_routine))
    {
        close_routine = RTN_FindByName(img, C_MANGLE("_close"));
    }
    RTN close_nocancel_routine = RTN_FindByName(img, CLOSE_NOCANCEL);
    if (!RTN_Valid(close_nocancel_routine))
    {
        close_nocancel_routine = RTN_FindByName(img, C_MANGLE("_close_nocancel"));
    }
    if (!RTN_Valid(close_nocancel_routine))
    {
        close_nocancel_routine = RTN_FindByName(img, C_MANGLE("__close_nocancel"));
    }

    if (RTN_Valid(close_routine) && RTN_Valid(close_nocancel_routine))
    {
        printf("  found function close          at %p\n", reinterpret_cast< void* >(RTN_Address(close_routine)));
        printf("  found function close_nocancel at %p\n", reinterpret_cast< void* >(RTN_Address(close_nocancel_routine)));
        printf("  placing probe on function close\n");
        unsigned int bytes4Before;
        unsigned int* ptr = reinterpret_cast< unsigned int* >(RTN_Address(close_nocancel_routine));
        bytes4Before      = *ptr;
        close_orig        = RTN_ReplaceProbed(close_routine, (AFUNPTR)(close_replacement));
        unsigned int bytes4After;
        bytes4After = *ptr;
        if (bytes4After != bytes4Before)
        {
            printf("***Error bytes in function close_noncancel have been overwritten by probe placed on function close\n");
            fflush(stdout);
            exit(1);
        }
    }
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

int main(int argc, char* argv[])
{
    // Initialize pin & symbol manager
    PIN_Init(argc, argv);
    PIN_InitSymbols();

    // Register ImageLoad
    IMG_AddInstrumentFunction(ImageLoad, 0);

    // Start the program, never returns
    PIN_StartProgramProbed();

    return 0;
}
/* ===================================================================== */
