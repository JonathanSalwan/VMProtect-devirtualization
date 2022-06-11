/*
 * Copyright (C) 2014-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <elf.h>

void* findVdso(void* auxv_start)
{
#ifdef TARGET_IA32
    Elf32_auxv_t* auxv = (Elf32_auxv_t*)auxv_start;
    void* vdso         = (void*)0xffffe000;
#else
    Elf64_auxv_t* auxv = (Elf64_auxv_t*)auxv_start;
    void* vdso         = (void*)0xffffffffff600000;
#endif

    for (; auxv->a_type != AT_NULL; auxv++)
    {
        if (auxv->a_type == AT_SYSINFO_EHDR)
        {
            return (void*)auxv->a_un.a_val;
        }
    }
    return vdso;
}

int main(int argc, char** argv, char** envp)
{
    while (*envp++ != NULL)
        ;

    void* vdso = findVdso(envp);

    printf("%p:%p\n", vdso, vdso + getpagesize());

    return 0;
}
