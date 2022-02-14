/*
 * Copyright (C) 2012-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include "os_specific.h"
#include "os-apis/host.h"

static int IsHostArch64Bit()
{
    OS_RETURN_CODE os_ret;
    OS_HOST_CPU_ARCH_TYPE host_arch;
    static int host_is_64bit = 1;
    static int checked       = 0;

    if (1 == checked) return host_is_64bit;
    checked = 1;

    os_ret = OS_GetHostCPUArch(&host_arch);
    if (os_ret.generic_err == OS_RETURN_CODE_NO_ERROR)
    {
        if (OS_HOST_CPU_ARCH_TYPE_IA32 == host_arch)
        {
            host_is_64bit = 0;
        }
        if (host_arch == OS_HOST_CPU_ARCH_TYPE_INVALID)
        {
            fprintf(stderr, "OS_GetHostCPUArch returned OS_HOST_CPU_ARCH_TYPE_INVALID. Assuming 64 bit.\n");
        }
    }
    else
    {
        fprintf(stderr, "OS_GetHostCPUArch returned with error. Assuming 64 bit.\ngeneric_err=%d, os_specific_err=%d \n",
                (int)os_ret.generic_err, os_ret.os_specific_err);
    }
    return host_is_64bit;
}

void update_environment(char* base_path)
{
    /*
     Set the following environment variables:
     1) PIN_VM32_LD_LIBRARY_PATH and PIN_VM64_LD_LIBRARY_PATH
     2) PIN_LD_RESTORE_REQUIRED (set it to "t")
     3) PIN_APP_LD_LIBRARY_PATH (copy of LD_LIBRARY_PATH (if set))
     4) PIN_APP_LD_ASSUME_KERNEL (copy of LD_ASSUME_KERNEL (if set))
     4a) PIN_APP_LD_BIND_NOW
     4b) PIN_APP_LD_PRELOAD
     5) unset LD_ASSUME_KERNEL
     6) PIN_INJECTOR32_LD_LIBRARY_PATH and PIN_INJECTOR64_LD_LIBRARY_PATH
     7) Set LD_LIBRARY_PATH to the proper PIN_INJECTOR*_LD_LIBRARY_PATH based on the host architecture.

     On Unix systems, we run pinbin instead of pin.
     */

    int r;
    const int overwrite                      = 1;
    char* pin_32_ld_library_path             = 0;
    char* pin_32_ld_library_path_getenv      = 0;
    char* pin_32_ld_library_path_xed         = 0;
    char* pin_32_ld_library_path_extlibs     = 0;
    char* pin_64_ld_library_path             = 0;
    char* pin_64_ld_library_path_getenv      = 0;
    char* pin_64_ld_library_path_xed         = 0;
    char* pin_64_ld_library_path_extlibs     = 0;
    char* injector_32_ld_library_path        = 0;
    char* injector_32_ld_library_path_getenv = 0;
    char* injector_32_ld_library_path_xed    = 0;
    char* injector_64_ld_library_path        = 0;
    char* injector_64_ld_library_path_getenv = 0;
    char* injector_64_ld_library_path_xed    = 0;
    const char* pin_runtime_dir              = "runtime";
    const char* lib_ext_dir                  = "lib-ext";
    const char* pincrt_lib_dir               = "pincrt";
    const char* extras_dir                   = "extras";
    const char* lib_dir                      = "lib";
    char* ld_library_path                    = 0;
    const char* ld_assume_kernel;
    char* base_path32        = 0;
    char* base_path64        = 0;
    char* extras_path        = 0;
    char* xed32              = 0;
    char* xed64              = 0;
    char* xed_runtime_libs32 = 0;
    char* xed_runtime_libs64 = 0;
    char* pin_runtime_libs32 = 0;
    char* pin_runtime_libs64 = 0;
    char* pincrt_libs32      = 0;
    char* pincrt_libs64      = 0;
    char* ext_libs32         = 0;
    char* ext_libs64         = 0;
    const char* incoming_ld_preload;
    const char* incoming_ld_bind_now;

    base_path32 = appendPath(base_path, "/", "ia32");
    base_path64 = appendPath(base_path, "/", "intel64");

    extras_path = appendPath(base_path, "/", extras_dir);
    xed32       = appendPath(extras_path, "/", "xed-ia32");
    xed64       = appendPath(extras_path, "/", "xed-intel64");
    if (extras_path) free(extras_path);
    xed_runtime_libs32 = appendPath(xed32, "/", lib_dir);
    xed_runtime_libs64 = appendPath(xed64, "/", lib_dir);
    if (xed32) free(xed32);
    if (xed64) free(xed64);

    /* make pin_libs - required for pin/vm */
    pin_runtime_libs32 = appendPath(base_path32, "/", pin_runtime_dir);
    pin_runtime_libs64 = appendPath(base_path64, "/", pin_runtime_dir);

    ext_libs32 = appendPath(base_path32, "/", lib_ext_dir);
    ext_libs64 = appendPath(base_path64, "/", lib_ext_dir);
    if (base_path32) free(base_path32);
    if (base_path64) free(base_path64);

    pincrt_libs32 = appendPath(pin_runtime_libs32, "/", pincrt_lib_dir);
    pincrt_libs64 = appendPath(pin_runtime_libs64, "/", pincrt_lib_dir);
    if (pin_runtime_libs32) free(pin_runtime_libs32);
    if (pin_runtime_libs64) free(pin_runtime_libs64);

    /* Set Pin Vm library paths */
    pin_32_ld_library_path_getenv = getenv("PIN_VM32_LD_LIBRARY_PATH");
    pin_64_ld_library_path_getenv = getenv("PIN_VM64_LD_LIBRARY_PATH");

    pin_32_ld_library_path_xed     = appendPath(xed_runtime_libs32, ":", pin_32_ld_library_path_getenv);
    pin_64_ld_library_path_xed     = appendPath(xed_runtime_libs64, ":", pin_64_ld_library_path_getenv);
    pin_32_ld_library_path_extlibs = appendPath(ext_libs32, ":", pin_32_ld_library_path_xed);
    if (ext_libs32) free(ext_libs32);
    if (pin_32_ld_library_path_xed) free(pin_32_ld_library_path_xed);
    pin_64_ld_library_path_extlibs = appendPath(ext_libs64, ":", pin_64_ld_library_path_xed);
    if (ext_libs64) free(ext_libs64);
    if (pin_64_ld_library_path_xed) free(pin_64_ld_library_path_xed);
    pin_32_ld_library_path = appendPath(pincrt_libs32, ":", pin_32_ld_library_path_extlibs);
    if (pin_32_ld_library_path_extlibs) free(pin_32_ld_library_path_extlibs);
    pin_64_ld_library_path = appendPath(pincrt_libs64, ":", pin_64_ld_library_path_extlibs);
    if (pin_64_ld_library_path_extlibs) free(pin_64_ld_library_path_extlibs);

    r = setenv("PIN_VM32_LD_LIBRARY_PATH", pin_32_ld_library_path, overwrite);
    check_retval(r, "setenv PIN_VM32_LD_LIBRARY_PATH");
    r = setenv("PIN_VM64_LD_LIBRARY_PATH", pin_64_ld_library_path, overwrite);
    check_retval(r, "setenv PIN_VM64_LD_LIBRARY_PATH");
    if (pin_32_ld_library_path) free(pin_32_ld_library_path);
    if (pin_64_ld_library_path) free(pin_64_ld_library_path);

    /* Set Pin injector library paths */
    injector_32_ld_library_path_getenv = getenv("PIN_INJECTOR32_LD_LIBRARY_PATH");
    injector_64_ld_library_path_getenv = getenv("PIN_INJECTOR64_LD_LIBRARY_PATH");

    injector_32_ld_library_path_xed = appendPath(xed_runtime_libs32, ":", injector_32_ld_library_path_getenv);
    injector_64_ld_library_path_xed = appendPath(xed_runtime_libs64, ":", injector_64_ld_library_path_getenv);
    if (xed_runtime_libs32) free(xed_runtime_libs32);
    if (xed_runtime_libs64) free(xed_runtime_libs64);
    injector_32_ld_library_path = appendPath(pincrt_libs32, ":", injector_32_ld_library_path_xed);
    injector_64_ld_library_path = appendPath(pincrt_libs64, ":", injector_64_ld_library_path_xed);
    if (pincrt_libs32) free(pincrt_libs32);
    if (pincrt_libs64) free(pincrt_libs64);

    r = setenv("PIN_INJECTOR32_LD_LIBRARY_PATH", injector_32_ld_library_path, overwrite);
    check_retval(r, "setenv PIN_INJECTOR32_LD_LIBRARY_PATH");
    r = setenv("PIN_INJECTOR64_LD_LIBRARY_PATH", injector_64_ld_library_path, overwrite);
    check_retval(r, "setenv PIN_INJECTOR64_LD_LIBRARY_PATH_64");

    /* This variable tells the injector to restore environment variables after pin is injected. */
    r = setenv("PIN_LD_RESTORE_REQUIRED", "t", overwrite);
    check_retval(r, "setenv PIN_LD_RESTORE_REQUIRED");

    /*
     * Backup the LD_LIBRARY_PATH, since pin uses a different one while launching. It will be restored
     * when the app is loaded to memory.
     */
    ld_library_path = getenv("LD_LIBRARY_PATH");
    if (ld_library_path)
    {
        r = setenv("PIN_APP_LD_LIBRARY_PATH", ld_library_path, overwrite);
        check_retval(r, "setenv PIN_APP_LD_LIBRARY_PATH");
    }

    /* Overwrite LD_LIBRARY_PATH with the libraries required for pin to run. */
    r = setenv("LD_LIBRARY_PATH", (IsHostArch64Bit()) ? injector_64_ld_library_path : injector_32_ld_library_path, overwrite);
    check_retval(r, "setenv LD_LIBRARY_PATH");
    if (injector_32_ld_library_path) free(injector_32_ld_library_path);
    if (injector_32_ld_library_path_xed) free(injector_32_ld_library_path_xed);
    if (injector_64_ld_library_path) free(injector_64_ld_library_path);
    if (injector_64_ld_library_path_xed) free(injector_64_ld_library_path_xed);

    /*
     * If the LD_BIND_NOW, LD_ASSUME_KERNEL and LD_PRELOAD variables were defined they should pass as
     * is to the app. Since pin's injector doesn't need it, we save them now and restore it after
     * pin is injected into the process.
     */
    ld_assume_kernel = getenv("LD_ASSUME_KERNEL");
    if (ld_assume_kernel)
    {
        r = setenv("PIN_APP_LD_ASSUME_KERNEL", ld_assume_kernel, overwrite);
        check_retval(r, "setenv PIN_APP_LD_ASSUME_KERNEL");
        unsetenv("LD_ASSUME_KERNEL");
    }

    incoming_ld_bind_now = getenv("LD_BIND_NOW");
    if (incoming_ld_bind_now)
    {
        r = setenv("PIN_APP_LD_BIND_NOW", incoming_ld_bind_now, overwrite);
        check_retval(r, "setenv PIN_APP_LD_BIND_NOW");
        unsetenv("LD_BIND_NOW");
    }

    incoming_ld_preload = getenv("LD_PRELOAD");
    if (incoming_ld_preload)
    {
        r = setenv("PIN_APP_LD_PRELOAD", incoming_ld_preload, overwrite);
        check_retval(r, "setenv PIN_APP_LD_PRELOAD");
        unsetenv("LD_PRELOAD");
    }
}

char* find_driver_name(char* argv0)
{
    int chars;
    const char* proc_link = 0;
    char base_path[PATH_MAX];

    /* The path of the current running executable (self) under the procfs. */
    proc_link = "/proc/self/exe";

    if (access(proc_link, F_OK) != 0)
    {
        /* no /proc... */
        size_t len = strnlen_s(argv0, PATH_MAX);
        assert((len > 0) && (len < PATH_MAX));
        strcpy(base_path, argv0);
    }
    else
    {
        chars = readlink(proc_link, base_path, PATH_MAX);
        if (chars == -1)
        {
            perror("readlink:");
            exit(1);
        }
        base_path[chars] = 0; /* null terminate the string */
    }

    return strdup(base_path);
}

char** build_child_argv(char* base_path, int argc, char** argv, int user_argc, char** user_argv)
{
    int var = 0, user_arg = 0, child_argv_ind = 0;
    char *s1 = 0, *s2 = 0;
    char** child_argv = (char**)malloc(sizeof(char*) * (argc + user_argc + 4));
    if (child_argv == NULL) abort();

    /*
        This is just to make sure both binaries actually exist. The program will exit if any of these calls fail.
      */
    s1 = appendPath(base_path, "/", "ia32/bin/pinbin");
    s2 = find_driver_name(s1);
    if (s1) free(s1);
    if (s2) free(s2);
    s1 = appendPath(base_path, "/", "intel64/bin/pinbin");
    s2 = find_driver_name(s1);
    if (s1) free(s1);
    if (s2) free(s2);

    /*
       Set the default pinbin to that of the architecture of the host.
       The reason is that a 64bit machine may not have 32bit glibc installed,
       in which case pin will fail to run if the 32bit pinbin
       version is used.
       If host is 64bit then start with 64bit pinbin. pinbin may later switch to the 32bit pinbin using the -p32 parameter.
       If host is 32bit then start with 32bit pinbin. pinbin may later switch to the 64bit pinbin using the -p64 parameter.
    */
    if (IsHostArch64Bit())
    {
        child_argv[child_argv_ind++] = appendPath(base_path, "/", "intel64/bin/pinbin");
        child_argv[child_argv_ind++] = "-p32";
        child_argv[child_argv_ind++] = appendPath(base_path, "/", "ia32/bin/pinbin");
    }
    else
    {
        child_argv[child_argv_ind++] = appendPath(base_path, "/", "ia32/bin/pinbin");
        child_argv[child_argv_ind++] = "-p64";
        child_argv[child_argv_ind++] = appendPath(base_path, "/", "intel64/bin/pinbin");
    }

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
