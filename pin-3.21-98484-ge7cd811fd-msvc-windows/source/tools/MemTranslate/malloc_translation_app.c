/*
 * Copyright (C) 2015-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#define _GNU_SOURCE
#include <dlfcn.h>
#include <link.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// The highest bit value in a pointer
#define HIGHEST_BIT ((uintptr_t)1 << (8 * sizeof(void*) - 1))
// Turn on the highest bit in a pointer
#define MK_PTR(ptr) ((void*)(((uintptr_t)ptr) | HIGHEST_BIT))
// Turn off the highest bit in a pointer
#define STRIP_PTR(ptr) ((void*)((uintptr_t)ptr & ~HIGHEST_BIT))

// Initial allocator buffer (see below for explnaition).
static char initial_buf[1024 * 1024];
static size_t initial_buf_idx = 0;

static void* (*libc_malloc)(size_t)             = NULL;
static void* (*libc_calloc)(size_t, size_t)     = NULL;
static void* (*libc_realloc)(void* ptr, size_t) = NULL;
static void (*libc_free)(void*)                 = NULL;

/*
 * Initialize all malloc related symbols from libc
 */
void __attribute__((constructor)) init()
{
    libc_malloc  = dlsym(RTLD_NEXT, "malloc");
    libc_calloc  = dlsym(RTLD_NEXT, "calloc");
    libc_realloc = dlsym(RTLD_NEXT, "realloc");
    libc_free    = dlsym(RTLD_NEXT, "free");
}

/*********************************************************
 * Below we implement an initial allocator.
 * The first allocated bytes cannot be allocate with libc's
 * memory allocation function.
 * This is merely because calling dlsym() in the loader, to
 * get the address of libc's memory allocation function,
 * causes the loader to call calloc() (the overriden version
 * of calloc()) and if we call dlsym() in calloc() it will
 * eventually cause an infinite recursion.
 * To overcome this, we implement a simple allocator here
 * that allocates the first bytes of the program from
 * the static buffer initial_buf[].
 *********************************************************/
void* initial_malloc(size_t size)
{
    void* ret;
    if (sizeof(initial_buf) < initial_buf_idx + size + sizeof(size_t))
    {
        return NULL;
    }
    *((size_t*)&initial_buf[initial_buf_idx]) = size;
    ret                                       = (void*)&initial_buf[initial_buf_idx + sizeof(size_t)];
    initial_buf_idx += ((size + 2 * sizeof(size_t) - 1) / sizeof(size_t)) * sizeof(size_t);
    return ret;
}

size_t initial_free(void* ptr)
{
    if ((ptr >= (void*)initial_buf) && (ptr < (void*)&initial_buf[initial_buf_idx]))
    {
        return *((size_t*)ptr - 1);
    }
    return 0;
}

/*********************************************************
 * Wrapper functions for libc memory allocation functions.
 * Here we translate the memory addresses in and out of
 * memory allocation function so the highest bit in the
 * memory address will be turn on.
 *********************************************************/
void* malloc(size_t size)
{
    void* ret;
    if (NULL != (ret = initial_malloc(size)))
    {
        return MK_PTR(ret);
    }
    return MK_PTR(libc_malloc(size));
}

void* calloc(size_t nmemb, size_t size)
{
    void* ret;
    if (NULL != (ret = initial_malloc(nmemb * size)))
    {
        return MK_PTR(ret);
    }
    return MK_PTR(libc_calloc(nmemb, size));
}

void* realloc(void* ptr, size_t size)
{
    size_t old_size;
    if (0 != (old_size = initial_free(STRIP_PTR(ptr))))
    {
        void* new_ptr = malloc(size);
        memcpy(STRIP_PTR(new_ptr), STRIP_PTR(ptr), size < old_size ? size : old_size);
        return new_ptr;
    }
    return MK_PTR(libc_realloc(STRIP_PTR(ptr), size));
}

void free(void* ptr)
{
    if (0 < initial_free(STRIP_PTR(ptr)))
    {
        return;
    }
    return libc_free(STRIP_PTR(ptr));
}

/*
 * This simple program just loads the library which its filename was
 * provided ain the command line arguments
 */
int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <path to dynamic library>\n", argv[0]);
        exit(1);
    }
    const char* file = argv[1];
    printf("Loading shared object %s\n", file);
    fflush(stdout);
    void* handle = dlopen(file, RTLD_NOW | RTLD_LOCAL);
    if (NULL == handle)
    {
        fprintf(stderr, "Failed to load %s - %s\n", file, dlerror());
        exit(1);
    }
    printf("Unloading shared object %s\n", file);
    fflush(stdout);
    dlclose(handle);

    printf("Application finished successfully!\n");
    return 0;
}
