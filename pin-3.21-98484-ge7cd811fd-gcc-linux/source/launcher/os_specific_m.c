/*
 * Copyright (C) 2012-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include "os_specific.h"
#include <libproc.h>

int check_set_evar(char const* evar, char const* pvar)
{
    /* set the pin env var to evar's value if evar has a non-null value */
    char* val = 0;
    val       = getenv(evar);
    if (val)
    {
        setenv(pvar, val, 1);
        unsetenv(evar);
        return 1;
    }
    return 0;
}

void update_environment(char* base_path)
{
    int r;
    const char* pin_runtime_dir = "runtime";
    const char* lib_ext_dir     = "lib-ext";
    const char* pincrt_lib_dir  = "pincrt";
    const char* extras_dir      = "extras";
    const char* lib_dir         = "lib";
    const int overwrite         = 1;
    char* pin_ld_library_path   = 0;
    char* new_ld_library_path   = 0;
    char* base_path32           = 0;
    char* base_path64           = 0;
    char* extras_path           = 0;
    char* xed32                 = 0;
    char* xed64                 = 0;
    char* xed_runtime_libs32    = 0;
    char* xed_runtime_libs64    = 0;
    char* pin_runtime_libs32    = 0;
    char* pin_runtime_libs64    = 0;
    char* pincrt_libs32         = 0;
    char* pincrt_libs64         = 0;
    char* ext_libs32            = 0;
    char* ext_libs64            = 0;
    char* ld_library_path       = 0;

    check_set_evar("DYLD_LIBRARY_PATH", "PIN_APP_DYLD_LIBRARY_PATH");

    check_set_evar("DYLD_INSERT_LIBRARIES", "PIN_APP_DYLD_INSERT_LIBRARIES");

    check_set_evar("DYLD_BIND_AT_LAUNCH", "PIN_APP_DYLD_BIND_AT_LAUNCH");

    setenv("PIN_DYLD_RESTORE_REQUIRED", "t", 1);

    base_path32 = appendPath(base_path, "/", "ia32");
    base_path64 = appendPath(base_path, "/", "intel64");

    extras_path        = appendPath(base_path, "/", extras_dir);
    xed32              = appendPath(extras_path, "/", "xed-ia32");
    xed64              = appendPath(extras_path, "/", "xed-intel64");
    xed_runtime_libs32 = appendPath(xed32, "/", lib_dir);
    xed_runtime_libs64 = appendPath(xed64, "/", lib_dir);

    /* make pin_libs - required for pin/vm */
    pin_runtime_libs32 = appendPath(base_path32, "/", pin_runtime_dir);
    pin_runtime_libs64 = appendPath(base_path64, "/", pin_runtime_dir);

    pincrt_libs32 = appendPath(pin_runtime_libs32, "/", pincrt_lib_dir);
    pincrt_libs64 = appendPath(pin_runtime_libs64, "/", pincrt_lib_dir);

    ext_libs32 = appendPath(base_path32, "/", lib_ext_dir);
    ext_libs64 = appendPath(base_path64, "/", lib_ext_dir);

    /* make pin_ld_library_path pre-pending pin_libs -- for the VM ultimately */
    ld_library_path     = getenv("PIN_VM_DYLD_LIBRARY_PATH");
    pin_ld_library_path = ld_library_path;

    /* Add the path which contains XED */
    pin_ld_library_path = appendPath(xed_runtime_libs32, ":", pin_ld_library_path);
    pin_ld_library_path = appendPath(xed_runtime_libs64, ":", pin_ld_library_path);
    new_ld_library_path = appendPath(xed_runtime_libs32, ":", new_ld_library_path);
    new_ld_library_path = appendPath(xed_runtime_libs64, ":", new_ld_library_path);

    /* Add the path which contains the Pin CRT runtime */
    pin_ld_library_path = appendPath(pincrt_libs32, ":", pin_ld_library_path);
    pin_ld_library_path = appendPath(pincrt_libs64, ":", pin_ld_library_path);
    new_ld_library_path = appendPath(pincrt_libs32, ":", new_ld_library_path);
    new_ld_library_path = appendPath(pincrt_libs64, ":", new_ld_library_path);

    /* Add pindwarf path to PIN tool's library search path */
    pin_ld_library_path = appendPath(ext_libs32, ":", pin_ld_library_path);
    pin_ld_library_path = appendPath(ext_libs64, ":", pin_ld_library_path);

    /* Set the pin vm library path. */
    r = setenv("PIN_VM_DYLD_LIBRARY_PATH", pin_ld_library_path, overwrite);
    check_retval(r, "setenv PIN_VM_DYLD_LIBRARY_PATH");

    /* Overwrite DYLD_LIBRARY_PATH with the libraries required for pin to run. */
    r = setenv("DYLD_LIBRARY_PATH", new_ld_library_path, overwrite);
    check_retval(r, "setenv DYLD_LIBRARY_PATH");
    (void)base_path; //pacify compiler
}

char* find_driver_name(char* argv0)
{
    char appPath[PATH_MAX];
    proc_pidpath(getpid(), appPath, sizeof(appPath));
    return strdup(appPath);
}

char** build_child_argv(char* base_path, int argc, char** argv, int user_argc, char** user_argv)
{
    char** child_argv = (char**)malloc(sizeof(char*) * (argc + user_argc + 4));
    if (!child_argv)
    {
        abort();
    }
    int var = 0, user_arg = 0, child_argv_ind = 0;

    /*
     * All macOS systems support 64 bit. Start with it and change to 32 bit only if needed.
     */
    child_argv[child_argv_ind++] = appendPath(base_path, "/", "intel64/bin/pinbin");
    child_argv[child_argv_ind++] = "-p32";
    child_argv[child_argv_ind++] = appendPath(base_path, "/", "ia32/bin/pinbin");

    /* Add the user arguments */
    for (user_arg = 0; user_arg < user_argc; ++user_arg)
    {
        child_argv[child_argv_ind++] = user_argv[user_arg];
    }

    /* Copy original command line parameters. */
    for (var = 1; var < argc; ++var)
    {
        child_argv[child_argv_ind++] = argv[var];
    }

    /* Null terminate the array. */
    child_argv[child_argv_ind++] = NULL;

    /* Clean the user arguments memory */
    if (user_argv)
    {
        free(user_argv);
    }

    return child_argv;
}
