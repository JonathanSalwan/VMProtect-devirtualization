/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * This test verifies that the application can block SEGV without interfering with Pin's
 * ability to catch its own internally-generated SEGV's.  This application causes Pin to
 * speculatively fetch instructions from a page that is unreadable, which generates an
 * internal SEGV in Pin.  The test will only pass if Pin can handle that SEGV despite the
 * fact that the application has blocked it.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/mman.h>
#include <unistd.h>

#ifndef MAP_ANONYMOUS
#ifdef MAP_ANON
#define MAP_ANONYMOUS MAP_ANON
#endif
#endif

/*
 * We write this bit of machine code at the very end of a page, where the next page is unreadable.
 * We then call to "Entry" and expect it to return back.  This code snippet is constructed in
 * such a way that Pin will speculatively fetch beyond the final JNE and attempt to fetch from
 * the unreadable page.
 */
const unsigned char Code[] = {
    0xc3,                   /* L1:    ret           */
    0x66, 0x83, 0xfc, 0x00, /* Entry: cmp  $0x0,%sp */
    0x75, 0xf9              /*        jne  L1       */
};

const size_t EntryOffset = 1; /* Offset of 'Entry' from start of 'Code' */

static void BlockSegv();

int main(int argc, char** argv)
{
    size_t pageSize;
    char* twoPages;

    /*
     * Map a page of memory and ensure that the subsequent page is unreadable.
     */
    pageSize = getpagesize();
    twoPages = mmap(0, 2 * pageSize, (PROT_READ | PROT_WRITE | PROT_EXEC), (MAP_PRIVATE | MAP_ANONYMOUS), -1, 0);
    if (twoPages == MAP_FAILED)
    {
        printf("Unable to map pages\n");
        return 1;
    }

    printf("Mapped two pages at %p\n", twoPages);
    printf("Unprotecting page at %p\n", twoPages + pageSize);

    if (mprotect(twoPages + pageSize, pageSize, PROT_NONE) != 0)
    {
        printf("Unable to unprotect second page\n");
        return 1;
    }

    /*
     * Copy the "Code" to the end of the page.
     */
    memcpy(twoPages + pageSize - sizeof(Code), Code, sizeof(Code));

    /*
     * Block SEGV and then try to call the code snippet.  Pin will try to
     * fetch from the unreadable page, which raises SEGV.  We want to make
     * sure that the this doesn't cause a problem in Pin even though the
     * application has SEGV blocked.
     */
    BlockSegv();
    ((void (*)())(&twoPages[pageSize - sizeof(Code) + EntryOffset]))();

    printf("Got back OK\n");
    return 0;
}

static void BlockSegv()
{
    sigset_t ss;

    sigemptyset(&ss);
    sigaddset(&ss, SIGSEGV);
    if (sigprocmask(SIG_BLOCK, &ss, 0) != 0)
    {
        printf("Unable to block SEGV\n");
        exit(1);
    }
}
