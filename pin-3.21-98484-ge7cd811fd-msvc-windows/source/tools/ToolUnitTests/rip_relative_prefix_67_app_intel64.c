/*
 * Copyright (C) 2017-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 * This test checks that Pin calculates the correct address for RIP-relative addressing instruction with 67H prefix
 * (Address-size override prefix).
 *
 * func_is_5(int) uses global data (located in the same page the function is located), using RIP-relative instruction
 * (with 67H prefix).
 * What this test is doing, is to copy this function (only function without the global data) to a page with high
 * address (larger than a 32 bit address) while making sure that the 32 low bits of the new function address are the
 * same a the original address.
 * After that it calls the copy func_is_5(5) which is located in the high page.
 * Although we didn't copy the global data into the new page the RIP-relative instruction should work since 67H
 * prefix is used. Meaning the address of the memory operand that is using RIP-relative should only use the
 * low 32 bits of instruction. Which means that the IP that will be used is EIP which is equal to the EIP of the
 * original instruction, therefore the memory address that will be calculated will be that of the address of the
 * global data in the original page.
 *
 * If Pin doens't calculate the correct address (for RIP-relative addressing instruction with 67H prefix) the test
 * will fail.
 *
 */
#include <string.h>
#include <stdio.h>
#include <sys/mman.h>

// Size of func_is_5 function
extern int func_is_5_size;

/**
 * Return true if the specified val equals 5, False otherwise.
 */
extern int func_is_5(int val);

typedef union
{
    int (*fptr)(int);
    void* ptr;
} fptr_t;

// Get address of func_is_5()
void* get_func_is_5()
{
    fptr_t fp;
    fp.fptr = func_is_5;
    return fp.ptr;
}

int main()
{
    fptr_t nfp;
    void* addr = get_func_is_5();

    // Map a page in high address larger then a 32 bit address (above 0x600000000)
    unsigned long long aaddr = ((unsigned long long)addr) & ~(4095ULL); // aligned to page
    // new address must have the same low 32 bits as the original function in order to make the test work
    unsigned long long naddr = aaddr | 0x600000000;                      // move to high addr
    unsigned long long faddr = ((unsigned long long)addr) | 0x600000000; // new address of func_is_5
    void* area = mmap((void*)naddr, 4096, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);

    if (area == MAP_FAILED)
    {
        perror("mmap");
        return 1;
    }

    // Sanity check
    if (func_is_5(5) != 1)
    {
        fprintf(stderr, "func_is_5 failed\n");
        return 1;
    }

    printf("original address of func_is_5: %p\n", addr);
    printf("area address: %p\n", area);
    printf("new address of func_is_5: %p\n", faddr);
    printf("func_is_5 size: %d\n", func_is_5_size);

    // Copy func_is_5() function to the new mapped page
    // Note that we only copy the function, not the .data that is located after the function
    memcpy((void*)faddr, addr, func_is_5_size);
    nfp.ptr = (void*)faddr;

    // Call the func_is_5() copy that is located in the new mapped page
    // This is expected to work since although func_is_5() use global data using a rip-relative addressing instruction,
    // it has 67H prefix (Address-size override prefix) which use only the low 32 bit of the RIP address.
    // The low 32 bit of the RIP address equal to that of the original func_is_5(). So the rip-relative calculation
    // will return the address of the data which is located in the original page of the original func_is_5()
    // function. Which means the right data will be used and the test will work. (Unless Pin doesn't calculate
    // the correct address for rip-relative addressing instruction with 67H prefix)
    if ((*nfp.fptr)(5) != 1)
    {
        fprintf(stderr, "new func_is_5 failed\n");
        return 1;
    }

    return 0;
}
